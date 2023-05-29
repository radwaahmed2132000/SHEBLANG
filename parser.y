%locations

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
void printSymbolTables();
void appendSymbolTable(int i);

int yylex(void);
void yyerror(char *s);
extern int yylineno;            /* from lexer */

%}


%union { nodeType* node; }

%token WHILE IF PRINT DO FOR SWITCH CASE DEFAULT BREAK ENUM FN RETURN
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

%token <node> IDENTIFIER INTEGER REAL BOOLEAN CHARACTER STR

%type <node> stmt stmt_list function_defn enum_defn return_statement
%type <node> expr literal enum_use
%type <node> case case_list
%type <node> function_call
%type <node> function_return_type 
%type <node> var_decl
%type <node> var_defn
%type <node> function_parameter_list identifier_list
%type <node> expr_list

%%

program:
        stmt_list { 
                /* First, create all the Symbol Tables. */
                setup_scopes($1);

                auto result = semantic_analysis($1);

                if (warningsOutput.sizeError > 0) {
                    warningsOutput.print();
                }

                if (errorsOutput.sizeError > 0) {
                    errorsOutput.print();
                    exit(1);
                }

                /* Excute the code using the interpreter / compiler. */
                ex($1);

                /* Print the final version of the symbol tables. */
                printSymbolTables();

                /* Free Memory used. */
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
        | IF '(' ')' stmt                         { $$ = $4; printf("%s",std::string("Syntax Error at line "+std::to_string(yylineno)+". If's condition can\'t be empty.\n").c_str()); }
        | IF '(' expr ')' stmt %prec IFX          { $$ = opr(IF, 2, $3, $5); }
        | IF '(' expr ')' stmt ELSE stmt          { $$ = opr(IF, 3, $3, $5, $7); }
        | SWITCH '(' expr ')' case                { $$ = sw($3, $5); }
        | expr ';'                                { $$ = $1; }
        | BREAK ';'                               { $$ = br(); }
        | PRINT expr ';'                          { $$ = UnOp::node(UnOper::Print, $2); }
        | WHILE '(' expr ')' stmt                 { 
                $$ = while_loop($3, $5); 
                set_break_parent($5, $$);
        }
        | DO stmt WHILE '(' expr ')' ';'          { 
                $$ = do_while_loop($5, $2); 
                set_break_parent($2, $$);
        }
        | '{' stmt_list '}'                       { $2->addNewScope = true; $$ = $2; }
        | var_decl ';'                            { $$ = $1; }
        | var_defn                                { $$ = $1; }
        | CONST var_decl '=' expr ';'             { $$ = varDefn($2, $4, true); }
        | enum_defn                              
        | function_defn
        | return_statement                        
        ;

return_statement:
                RETURN expr ';' { $$ = UnOp::node(UnOper::Return, $2); }
                ;

case: 
     CASE expr ':' stmt         { $$ = cs($2, $4); }
     | DEFAULT ':' stmt         { $$ = cs(nullptr, $3); }
     | '{' case_list '}'        { $2->addNewScope = true; $$ = $2; }
     ;

case_list:
         case                     { $$ = linkedListStump<caseNodeType>($1); } 
         | case_list case         { $$ = appendToLinkedList<caseNodeType>($1, $2); }
         ;

literal:
       INTEGER | REAL | BOOLEAN | CHARACTER | STR | IDENTIFIER 

stmt_list:
          stmt                  { $$ = statementList($1); }
        | stmt_list stmt        { $$ = $1; ($1)->asPtr<StatementList>()->addStatement($2); } 

expr : 
        literal                         { $$ = $1; }
        | expr PP                       { $$ = UnOp::node(UnOper::Increment, $1); }
        | expr MM                       { $$ = UnOp::node(UnOper::Decrement, $1); }
        | '+' expr %prec UPLUS          { $$ = UnOp::node(UnOper::Plus, $2); }
        | '-' expr %prec UMINUS         { $$ = UnOp::node(UnOper::Minus, $2); }
        | '!' expr                      { $$ = UnOp::node(UnOper::BoolNeg, $2); }
        | '~' expr                      { $$ = UnOp::node(UnOper::BitToggle, $2); }
        | IDENTIFIER '=' expr           { $$ = BinOp::assign($1, $3); }
        | IDENTIFIER PA expr            { $$ = BinOp::opAssign(BinOper::Add, $1, $3); }
        | IDENTIFIER SA expr            { $$ = BinOp::opAssign(BinOper::Sub, $1, $3); }
        | IDENTIFIER MA expr            { $$ = BinOp::opAssign(BinOper::Mul, $1, $3); }
        | IDENTIFIER DA expr            { $$ = BinOp::opAssign(BinOper::Div, $1, $3); }
        | IDENTIFIER RA expr            { $$ = BinOp::opAssign(BinOper::Mod, $1, $3); }
        | IDENTIFIER LSA expr           { $$ = BinOp::opAssign(BinOper::LShift,  $1, $3); }
        | IDENTIFIER RSA expr           { $$ = BinOp::opAssign(BinOper::RShift,  $1, $3); }
        | IDENTIFIER ANDA expr          { $$ = BinOp::opAssign(BinOper::BitAnd, $1, $3); }
        | IDENTIFIER EORA expr          { $$ = BinOp::opAssign(BinOper::BitXor, $1, $3); }
        | IDENTIFIER IORA expr          { $$ = BinOp::opAssign(BinOper::BitOr, $1, $3); }
        | expr '&' expr                 { $$ = BinOp::node(BinOper::BitAnd, $1, $3); }
        | expr '|' expr                 { $$ = BinOp::node(BinOper::BitOr, $1, $3); }
        | expr '^' expr                 { $$ = BinOp::node(BinOper::BitXor, $1, $3); }
        | expr LS expr                  { $$ = BinOp::node(BinOper::LShift, $1, $3); }
        | expr RS expr                  { $$ = BinOp::node(BinOper::RShift, $1, $3); }
        | expr '+' expr                 { $$ = BinOp::node(BinOper::Add, $1, $3); }
        | expr '-' expr                 { $$ = BinOp::node(BinOper::Sub, $1, $3); }
        | expr '*' expr                 { $$ = BinOp::node(BinOper::Mul, $1, $3); }
        | expr '/' expr                 { $$ = BinOp::node(BinOper::Div, $1, $3); }
        | expr '%' expr                 { $$ = BinOp::node(BinOper::Mod, $1, $3); }
        | expr '<' expr                 { $$ = BinOp::node(BinOper::LessThan, $1, $3); }
        | expr '>' expr                 { $$ = BinOp::node(BinOper::GreaterThan, $1, $3); }
        | expr AND expr                 { $$ = BinOp::node(BinOper::And, $1, $3); }
        | expr OR expr                  { $$ = BinOp::node(BinOper::Or, $1, $3); }
        | expr GE expr                  { $$ = BinOp::node(BinOper::GreaterEqual, $1, $3); }
        | expr LE expr                  { $$ = BinOp::node(BinOper::LessEqual, $1, $3); }
        | expr NE expr                  { $$ = BinOp::node(BinOper::NotEqual, $1, $3); }
        | expr EQ expr                  { $$ = BinOp::node(BinOper::Equal, $1, $3); }
        | '(' expr ')'                  { $$ = $2; }
        | function_call                 { $$ = $1; }
        | enum_use
        ;

enum_use:
        IDENTIFIER SCOPE_RES IDENTIFIER { $$ = enum_use($1, $3); }

function_call:
             IDENTIFIER '(' expr_list ')' { 
                     $$ = functionCall($1, $3);
             }

expr_list:
         expr_list ',' expr { $$ = appendToLinkedList<ExprListNode>($1, exprListNode($3)); }
       | expr               { $$ = linkedListStump<ExprListNode>(exprListNode($1)); }
       |                    { $$ = linkedListStump<ExprListNode>(nullptr); }
       ;

enum_defn:
         ENUM IDENTIFIER '{' identifier_list '}' ';' { 
                auto idListEnd =  ($4)->as<IdentifierListNode>();
                auto enumMembers = idListEnd.toVec();
                $$ = enum_defn($2, enumMembers);
         };

identifier_list:
               identifier_list ',' IDENTIFIER   { $$ = appendToLinkedList<IdentifierListNode>($1, identifierListNode($3, false)); }
               | IDENTIFIER                     { $$ = linkedListStump<IdentifierListNode>(identifierListNode($1, false)); }
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
                     auto* head = ($4)->asPtr<VarDecl>();
                     $$ = statementList(fn($2, head, $6, $8));
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
