%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unordered_map>
#include <string>
#include "cl.h"

extern int yylineno;            /* from lexer */

/* prototypes */
nodeType *opr(int oper, int nops, ...);
nodeType *id(const char* id);

nodeType *con(int iValue);
nodeType *con(float fValue);
nodeType *con(bool bValue);
nodeType *con(char cValue);
nodeType *con(char* sValue);
nodeType *sw(nodeType* var, nodeType* case_list_head);
nodeType *cs(nodeType* self, nodeType* next);
nodeType *br();

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body);
nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body);
nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body);

void set_break_parent(nodeType* case_list, nodeType* parent_switch);

nodeType* fn(nodeType* name, nodeType* return_type, nodeType* statements);
nodeType* fn_call(nodeType* name);

void freeNode(nodeType *p);
Value ex(nodeType *p);
int yylex(void);

void yyerror(char *s);
float sym[26];                     /* symbol table */
std::unordered_map<std::string, SymbolTableEntry> sym2;

nodeType* varDecl(nodeType* type, nodeType* name) {
    auto nameStr = std::get<idNodeType>(name->un).id;
    auto typeStr = std::get<idNodeType>(type->un).id;

    return new nodeType(VarDecl(type, name));
}

nodeType* varDefn(nodeType* decl, nodeType* initExpr, bool isConstant) {
    auto* declPtr = std::get_if<VarDecl>(&decl->un);
    return new nodeType(VarDefn(declPtr, initExpr, isConstant));
}

%}

%union {
    int iValue;                 /* integer value */
    float fValue;               /* double value */
    bool bValue;                /* boolean value */
    char cValue;                /* char value */
    char* sValue;               /* string value */
    nodeType *nPtr;             /* node pointer */
};

%token <nPtr> IDENTIFIER
%token <iValue> INTEGER
%token <fValue> REAL
%token <bValue> BOOLEAN
%token <cValue> CHARACTER
%token <sValue> STR
%token WHILE IF PRINT DO FOR SWITCH CASE DEFAULT CASE_LIST BREAK ENUM FN RETURN
%token CONST INT FLOAT BOOL CHAR STRING
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

%type <nPtr> stmt expr stmt_list case case_list function_call function_return_type function_defn function_decl var_defn var_decl
%%

program:
        stmt_list { 
                auto result = semantic_analysis($1);
                if (result.isSuccess()) {
                    // printf("Semantic analysis successful\n");
                } else {
                    printf("Semantic analysis failed\n");
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
        | SWITCH '(' IDENTIFIER ')' case          { $$ = sw($3, $5); set_break_parent($5, $$); }
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
            $$ = varDefn($2, $4, true);
        }
        
        | enum_decl                               
        | function_defn
        | function_decl ';'
        | return_statement                        { printf("Return statement\n"); }
        ;

return_statement:
                RETURN expr ';'
                ;

case: 
     CASE INTEGER ':' stmt      { $$ = opr(CASE, 2, con($2), $4); }
     | DEFAULT ':' stmt         { $$ = opr(DEFAULT, 1, $3); }
     | '{' case_list '}'        { $$ = $2; }
     ;

case_list:
         case                     { $$ = cs($1, NULL); }
         | case_list case         { $$ = cs($1, $2); }
         ;

stmt_list:
          stmt                  { $$ = $1; }
        | stmt_list stmt        { $$ = opr(';', 2, $1, $2); }
        ;

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
        ;

function_call:
             IDENTIFIER '(' expr_list ')' { 
                     auto fn_name = std::get<idNodeType>($1->un);
                     printf("Function call (%s) parsed successfully\n", fn_name.id.c_str()); 
                     $$ = fn_call($1);
             }

expr_list:
         expr_list ',' expr
       | expr
       |
       ;

enum_decl:
         ENUM IDENTIFIER '{' identifier_list '}' ';' { 
                 auto enum_name = std::get<idNodeType>($2->un);
                 printf("Enum (%s) parsed successfully\n", enum_name.id.c_str()); 
         }
         ;

identifier_list:
               identifier_list ',' IDENTIFIER
               | IDENTIFIER
               |
               ;

function_return_type:
                    IDENTIFIER          {  $$ = $1; }
                    | /* EMPTY */       {  $$ = id("void"); }
                    ;

function_parameter_list:
                       function_parameter_list ',' var_decl
                       | var_decl
                       | /* NULL */
                       ;
function_defn:
             FN IDENTIFIER '(' function_parameter_list ')' function_return_type '{' stmt_list '}' { 
                     $$ = fn($2, $6, $8);
                     auto fn_name = std::get<idNodeType>($2->un);
                     printf("Function (%s) parsed successfully\n", fn_name.id.c_str()); 
             } 

function_decl:
             FN IDENTIFIER '(' function_parameter_list ')' function_return_type { 
                     $$ = fn_call($2); // TODO: Create another function for the declaration.
                     auto fn_name = std::get<idNodeType>($2->un);
                     printf("Function Declaration (%s) parsed successfully\n", fn_name.id.c_str()); 
             } 

%%

nodeType* fn(nodeType* name, nodeType* return_type, nodeType* statements) {
    return new nodeType(functionNodeType{return_type, name, statements});
}

nodeType* fn_call(nodeType* name) {
    return new nodeType(functionNodeType{nullptr, name});
}

struct set_break_parent_visitor {
        nodeType* parent_switch;

        void operator()(breakNodeType& b) {
               b.parent_switch = parent_switch;
        }
        void operator()(oprNodeType& opr) {
                for(int i = 0; i < opr.op.size(); i++) {
                        set_break_parent(opr.op[i], parent_switch);
                }
        }
        void operator()(caseNodeType& c) {
               set_break_parent(c.self, parent_switch);
               set_break_parent(c.prev, parent_switch);
        }

       // the default case:
       template<typename T> void operator()(T const &) const { } 
};

void set_break_parent(nodeType* node, nodeType* parent_switch) {
        if(node == NULL) return;
        
        std::visit(set_break_parent_visitor{parent_switch}, node->un);
}
nodeType *do_while_loop(nodeType* loop_condition, nodeType* loop_body) {
    return new nodeType(doWhileNodeType{false, loop_condition, loop_body});
}

nodeType *while_loop(nodeType* loop_condition, nodeType* loop_body) {
    return new nodeType(whileNodeType{false, loop_condition, loop_body});
}

nodeType *for_loop(nodeType* init_statement, nodeType* loop_condition, nodeType* post_loop_statement, nodeType* loop_body) {
    return new nodeType(forNodeType{false, init_statement, loop_condition, post_loop_statement, loop_body});
}

nodeType *br() {
    return new nodeType(breakNodeType{NULL});
}

nodeType *cs(nodeType* self, nodeType* prev) {
    auto cs = caseNodeType{};

    std::visit(
        Visitor {
                [&cs, self](oprNodeType&)  { cs.self = self; cs.prev = NULL; },
                [&cs, self, prev](caseNodeType&) { cs.prev = self; cs.self = prev; },
                [](auto) {}
        },
        self->un
    );

    return new nodeType(cs);
}

nodeType *sw(nodeType* var, nodeType* case_list_head) {
    return new nodeType(switchNodeType{ 0, 0, var, case_list_head });
}

#define CON_INIT(ptr_name, value) new nodeType(std::variant<NODE_TYPES>(conNodeType{value}))

nodeType *con(int iValue)   { return CON_INIT(p, iValue);    }
nodeType *con(float fValue) { return CON_INIT(p, fValue);  }
nodeType *con(bool bValue)  { return CON_INIT(p, bValue);   }
nodeType *con(char cValue)  { return CON_INIT(p, cValue);   }
nodeType *con(char* sValue) { return CON_INIT(p, std::string(sValue)); }

// Create an identifier node.
nodeType *id(const char* id) {
    return new nodeType(idNodeType{std::string(id)});
}

// Create an operator node with a variable number of inputs (usually 2)
// Note that `oper` is an enum value (or #ifdef), you can find the definintions in `y.tab.h`.
nodeType *opr(int oper, int nops, ...) {
    va_list ap;

    auto opr = oprNodeType{};
    opr.oper = oper;

    va_start(ap, nops);

    for(int i = 0; i < nops; i++)
        opr.op.push_back(va_arg(ap, nodeType*));

    va_end(ap);

    return new nodeType(opr);
}

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
