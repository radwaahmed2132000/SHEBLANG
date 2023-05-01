%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unordered_map>
#include <string>
#include "cl.h"

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
void set_break_parent(nodeType* case_list, nodeType* parent_switch);

nodeType* fn(nodeTypeTag* name, nodeTypeTag* return_type, nodeType* statements);
nodeType* fn_call(nodeTypeTag* name);

void freeNode(nodeType *p);
std::string ex(nodeType *p);
int yylex(void);

void yyerror(char *s);
float sym[26];                    /* symbol table */
std::unordered_map<std::string, std::pair<conTypeEnum,std::string>> sym2;
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

%type <nPtr> stmt expr stmt_list case case_list function_call function_return_type function_defn var_defn var_decl
%%

program:
        stmt_list { ex($1); freeNode($1); exit(0); }
        | /* NULL */
        ;

var_decl:
        // TODO: Use $1 for semantic analysis.
        IDENTIFIER IDENTIFIER       { $$ = $2; }
        ;

var_defn:
        var_decl '=' expr ';'        { $$ = opr('=', 2, $1, $3); }
        ;

stmt:
          ';'                                     { $$ = opr(';', 0); }
        | FOR '(' var_defn expr ';' expr ')' stmt { $$ = opr(FOR, 4, $3, $4, $6, $8); }
        | IF '(' expr ')' stmt %prec IFX          { $$ = opr(IF, 2, $3, $5); }
        | IF '(' expr ')' stmt ELSE stmt          { $$ = opr(IF, 3, $3, $5, $7); }
        | SWITCH '(' IDENTIFIER ')' case          { $$ = sw($3, $5); set_break_parent($5, $$); }
        | expr ';'                                { $$ = $1; }
        | BREAK ';'                               { $$ = br(); }
        | PRINT expr ';'                          { $$ = opr(PRINT, 1, $2); }
        | WHILE '(' expr ')' stmt                 { $$ = opr(WHILE, 2, $3, $5); }
        | DO stmt WHILE '(' expr ')' ';'          { $$ = opr(DO, 2, $5, $2); }
        | '{' stmt_list '}'                       { $$ = $2; }
        | var_decl ';'               { printf("Variable declaration parsed successfully\n"); }
        | var_defn                                { $$ = $1; }
        | CONST var_defn                          { $$ = $2; }
        | enum_decl                               
        | function_defn
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
        | IDENTIFIER IORA expr          { $$ = opr('=', 2, $1, opr('"', 2, $1, $3)); }
        | PP expr                       { $$ = opr(PP, 1, $2); }
        | MM expr                       { $$ = opr(MM, 1, $2); }
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


%%

nodeType* fn(nodeTypeTag* name, nodeTypeTag* return_type, nodeType* statements) {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    p->type = typeFunction;
    p->un = std::variant<NODE_TYPES>(functionNodeType{return_type, name, statements});

    return p;
}

nodeType* fn_call(nodeTypeTag* name) {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    p->type = typeFunction;
    p->un = std::variant<NODE_TYPES>(functionNodeType{nullptr, name});

    return p;
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

nodeType *br() {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    p->type = typeBreak;
    p->un = std::variant<NODE_TYPES>(breakNodeType{NULL});

    return p;
}

nodeType *cs(nodeType* self, nodeType* prev) {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    p->type = typeCase;
    p->un = std::variant<NODE_TYPES>(caseNodeType{});

    auto& cs = std::get<caseNodeType>(p->un);

    if(self->type == typeOpr) {
        cs.self = self;
        cs.prev = NULL;
    } else if (self->type == typeCase) {
        cs.prev = self;
        cs.self = prev;
    }

    return p;
}

nodeType *sw(nodeType* var, nodeType* case_list_head) {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeSwitch;

    p->un = std::variant<NODE_TYPES>(switchNodeType{
            0, 0, var, case_list_head
    });

    return p;
}

// con.valueName = valueName
// Assumes that the field in the union has the same name as the argument.
// We have to access specific union members since constructing like this:
//      conNodeType{fValue}
// Seems to interpret the given value in context of the first union member.
// In our case, this is the int member.
// This is an issue with floats because a float with 0b00...0001 != 1.0f,
// but a very small value (~0).
#define CON_INIT(ptr_name, valueName, value, conInnerType)      \
        nodeType* ptr_name;                                     \
        if ((ptr_name = new nodeType()) == NULL)                \
                yyerror("out of memory");                       \
        (ptr_name)->type = typeCon;                             \
        auto con = conNodeType{};                               \
        con.valueName = value;                                  \
        con.conType = conInnerType;                             \
        (ptr_name)->un = std::variant<NODE_TYPES>(con);         \
        return ptr_name

nodeType *con(int iValue)   { CON_INIT(p, iValue, iValue, intType);   return p; }
nodeType *con(float fValue) { CON_INIT(p, fValue, fValue, floatType); return p; }
nodeType *con(bool bValue)  { CON_INIT(p, bValue, bValue, boolType);  return p; }
nodeType *con(char cValue)  { CON_INIT(p, cValue, cValue, charType);  return p; }
nodeType *con(char* sValue) { CON_INIT(p, sValue, sValue, stringType);  return p; }

// Create an identifier node.
nodeType *id(const char* id) {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeId;
    p->un = std::variant<NODE_TYPES>(idNodeType{std::string(id)});

    return p;
}

// Create an operator node with a variable number of inputs (usually 2)
// Note that `oper` is an enum value (or #ifdef), you can find the definintions in `y.tab.h`.
nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    nodeType *p;
    int i;

    /* allocate node, extending op array */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeOpr;
    p->un = std::variant<NODE_TYPES>(oprNodeType{});
    auto& opr = std::get<oprNodeType>(p->un);
    opr.oper = oper;
    va_start(ap, nops);
    for (i = 0; i < nops; i++)
        opr.op.push_back(va_arg(ap, nodeType*));
    va_end(ap);
    return p;
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
