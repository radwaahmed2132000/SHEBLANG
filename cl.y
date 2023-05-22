%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unordered_map>
#include <string>
#include "cl.h"
#include "node_constructors.h"
#include <variant>


/* prototypes */
void freeNode(nodeType *p);
Value ex(nodeType *p);

int yylex(void);
void yyerror(char *s);
extern int yylineno;            /* from lexer */
%}

%union {
    int iValue;                 /* integer value */
    float fValue;               /* double value */
    bool bValue;                /* boolean value */
    char cValue;                /* char value */
    char* sValue;               /* string value */
    nodeType *nPtr;             /* node pointer */
    int lineNo;                 /* Line Number of Code Line */
};

%token <nPtr> IDENTIFIER
%token <iValue> INTEGER
%token <fValue> REAL
%token <bValue> BOOLEAN
%token <cValue> CHARACTER
%token <sValue> STR
%token WHILE IF PRINT DO FOR SWITCH CASE DEFAULT CASE_LIST BREAK ENUM FN RETURN
%token CONST INT FLOAT BOOL CHAR STRING SCOPE_RES
%nonassoc IFX
%nonassoc ELSE

%right '=' PA SA MA DA RA LSA RSA ANDA EORA IORA
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQ NE
%left GE LE '>' '<'
%left LS RS
%left '+' '-'
%left '*' '/' '%'
%right UPLUS UMINUS '!' '~' PP MM
/* left a++   a--	Suffix/postfix increment and decrement */

%type <nPtr> stmt expr stmt_list case case_list function_call function_return_type 
 function_defn var_defn function_parameter_list var_decl return_statement 
 enum_defn identifier_list enum_use
%%

program:
        stmt_list { 
                auto result = semantic_analysis($1);
                if (result.isSuccess()) {
                    // printf("Semantic analysis successful\n");
                } else {
                    // printf("Semantic analysis failed\n");
                    for (auto& error : std::get<ErrorType>(result)) {
                        printf("%s\n", error.c_str());
                    }
                    exit(1);
                }
                ex($1);
                freeNode($1);
                exit(0);
            }
        | /* NULL */
        ;

var_decl:
        IDENTIFIER IDENTIFIER       { $$ = varDecl($1, $2); }
        ;

var_defn:
        var_decl '=' expr ';'        { 
            $$ = varDefn($1, $3, false);
        };

stmt:
        ';'                                     { $$ = opr(';', 0); }
        | FOR '(' var_defn expr ';' expr ')' stmt { 
                $$ = for_loop($3, $4, $6, $8); 
                set_break_parent($8, $$);
        }
        | IF '(' expr ')' stmt %prec IFX          { $$ = opr(IF, 2, $3, $5); }
        | IF '(' expr ')' stmt ELSE stmt          { $$ = opr(IF, 3, $3, $5, $7); }
        | SWITCH '(' expr ')' case                { $$ = sw($3, $5); set_break_parent($5, $$); }
        | expr ';'                                { $$ = $1; }
        | BREAK ';'                               { $$ = br(); }
        | PRINT expr ';'                          { $$ = opr(PRINT, 1, $2); }
        | WHILE '(' expr ')' stmt                 { 
                $$ = while_loop($3, $5); 
                set_break_parent($5, $$);
        }
        | DO stmt WHILE '(' expr ')' ';'          { 
                $$ = do_while_loop($5, $2); 
                set_break_parent($2, $$);
        }
        | '{' stmt_list '}'                       { $$ = $2; }
        | var_decl ';'                            
        | var_defn                                { $$ = $1; }

        | CONST var_decl '=' expr ';' { 
            $$ = varDefn($2, $4, true);;
        }
        
        | enum_defn                              
        | function_defn
        | return_statement                        
        ;

return_statement:
                RETURN expr ';' { $$ = opr(RETURN, 1, $2); }
                ;

case: 
     CASE expr ':' stmt         { $$ = cs($2, $4); }
     | DEFAULT ':' stmt         { $$ = cs(nullptr, $3); }
     | '{' case_list '}'        { $$ = $2; }
     ;

case_list:
         case                     { $$ = linkedListStump<caseNodeType>($1); } 
         | case_list case         { $$ = appendToLinkedList<caseNodeType>($1, $2); }
         ;

stmt_list:
          stmt                  { $$ = linkedListStump<StatementList>(statementList($1)); }
        | stmt_list stmt        { 
            if(!std::holds_alternative<StatementList>($2->un)) {
                $$ = appendToLinkedList<StatementList>($1, statementList($2)); 
            } else {
                $$ = appendToLinkedList<StatementList>($1, $2); 
            }
        };

expr:
          INTEGER                       { $$ = con($1); }
        | REAL                          { $$ = con($1); }
        | BOOLEAN                       { $$ = con($1); }
        | CHARACTER                     { $$ = con($1); }
        | STR                           { $$ = con($1);  }
        | IDENTIFIER                    { $$ = $1; }
        | IDENTIFIER '=' expr           { $$ = opr('=', 2, $1, $3); }
        | IDENTIFIER PA expr            { $$ = opr('=', 2, $1, opr('+', 2, $1, $3)); }
        | IDENTIFIER SA expr            { $$ = opr('=', 2, $1, opr('-', 2, $1, $3)); }
        | IDENTIFIER MA expr            { $$ = opr('=', 2, $1, opr('*', 2, $1, $3)); }
        | IDENTIFIER DA expr            { $$ = opr('=', 2, $1, opr('/', 2, $1, $3)); }
        | IDENTIFIER RA expr            { $$ = opr('=', 2, $1, opr('%', 2, $1, $3)); }
        | IDENTIFIER LSA expr           { $$ = opr('=', 2, $1, opr(LS,  2, $1, $3)); }
        | IDENTIFIER RSA expr           { $$ = opr('=', 2, $1, opr(RS,  2, $1, $3)); }
        | IDENTIFIER ANDA expr          { $$ = opr('=', 2, $1, opr('&', 2, $1, $3)); }
        | IDENTIFIER EORA expr          { $$ = opr('=', 2, $1, opr('^', 2, $1, $3)); }
        | IDENTIFIER IORA expr          { $$ = opr('=', 2, $1, opr('|', 2, $1, $3)); }
        | expr PP                       { $$ = opr(PP, 1, $1); }
        | expr MM                       { $$ = opr(MM, 1, $1); }
        | '+' expr %prec UPLUS          { $$ = opr(UPLUS, 1, $2); }
        | '-' expr %prec UMINUS         { $$ = opr(UMINUS, 1, $2); }
        | '!' expr                      { $$ = opr('!', 1, $2); }
        | '~' expr                      { $$ = opr('~', 1, $2); }
        | expr '&' expr                 { $$ = opr('&', 2, $1, $3); }
        | expr '|' expr                 { $$ = opr('|', 2, $1, $3); }
        | expr '^' expr                 { $$ = opr('^', 2, $1, $3); }
        | expr LS expr                  { $$ = opr(LS, 2, $1, $3); }
        | expr RS expr                  { $$ = opr(RS, 2, $1, $3); }
        | expr '+' expr                 { $$ = opr('+', 2, $1, $3); }
        | expr '-' expr                 { $$ = opr('-', 2, $1, $3); }
        | expr '*' expr                 { $$ = opr('*', 2, $1, $3); }
        | expr '/' expr                 { $$ = opr('/', 2, $1, $3); }
        | expr '%' expr                 { $$ = opr('%', 2, $1, $3); }
        | expr '<' expr                 { $$ = opr('<', 2, $1, $3); }
        | expr '>' expr                 { $$ = opr('>', 2, $1, $3); }
        | expr AND expr                 { $$ = opr(AND, 2, $1, $3); }
        | expr OR expr                  { $$ = opr(OR, 2, $1, $3); }
        | expr GE expr                  { $$ = opr(GE, 2, $1, $3); }
        | expr LE expr                  { $$ = opr(LE, 2, $1, $3); }
        | expr NE expr                  { $$ = opr(NE, 2, $1, $3); }
        | expr EQ expr                  { $$ = opr(EQ, 2, $1, $3); }
        | '(' expr ')'                  { $$ = $2; }
        | function_call                 { $$ = $1; }
        | enum_use
        ;

enum_use:
        IDENTIFIER SCOPE_RES IDENTIFIER { $$ = enum_use($1, $3); }

function_call:
             IDENTIFIER '(' expr_list ')' { 
                     auto fn_name = std::get<idNodeType>($1->un);
                     $$ = fn_call($1);
             }

expr_list:
         expr_list ',' expr
       | expr
       |
       ;

enum_defn:
         ENUM IDENTIFIER '{' identifier_list '}' ';' { 
                auto idListEnd = std::get<IdentifierListNode>($4->un);
                auto enumMembers = idListEnd.toVec();
                $$ = enum_defn($2, enumMembers);

                // auto e = std::get<enumNode>($$->un);
                // for(const auto& [memberName, memberIndex]: e.enumMembers) {
                //     std::cout << memberName << ", " << memberIndex << '\n';
                // }
         };

identifier_list:
               identifier_list ',' IDENTIFIER   { $$ = appendToLinkedList<IdentifierListNode>($1, identifierListNode($3)); }
               | IDENTIFIER                     { $$ = linkedListStump<IdentifierListNode>(identifierListNode($1)); }
               | /* EMPTY */                    { $$ = linkedListStump<IdentifierListNode>(nullptr); }
               ;

function_return_type:
                    IDENTIFIER          {  $$ = $1; }
                    | /* EMPTY */       {  $$ = id("void"); }
                    ;

function_parameter_list:
                       function_parameter_list ',' var_decl  { $$ = appendToLinkedList<VarDecl>($1, $3); }
                       | var_decl                            { $$ = linkedListStump<VarDecl>($1); }
                       | /* NULL */                          { $$ = linkedListStump<VarDecl>(nullptr); }
                       ;
function_defn:
             FN IDENTIFIER '(' function_parameter_list ')' function_return_type '{' stmt_list '}' { 
                     auto head = std::get<VarDecl>($4->un);
                     auto paramsList = head.toVec();

                     $$ = fn($2, paramsList, $6, $8);
             } 

%%

// De-allocates a node and all of its children (if any).
void freeNode(nodeType *p) {
    int i;

    if (!p) return;
    free (p);
}

void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}

int main(void) {
#if defined(YYDEBUG) && (YYDEBUG != 0)
        yydebug = 1;
#endif
    yyparse();
    return 0;
}
