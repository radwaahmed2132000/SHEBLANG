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
nodeType *con(int value);
nodeType *sw(nodeType* var, nodeType* case_list_head);
nodeType *cs(nodeType* self, nodeType* next);
nodeType *br();
void set_break_parent(nodeType* case_list, nodeType* parent_switch);

void freeNode(nodeType *p);
int ex(nodeType *p);
int yylex(void);

void yyerror(char *s);
int sym[26];                    /* symbol table */
std::unordered_map<std::string, int> sym2;
%}

%union {
    int iValue;                 /* integer value */
    nodeType *nPtr;             /* node pointer */
};

%token <iValue> INTEGER
%token <nPtr> IDENTIFIER
%token WHILE IF PRINT DO FOR SWITCH CASE DEFAULT CASE_LIST BREAK
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

%type <nPtr> stmt expr stmt_list case case_list
%%

program:
        function                { exit(0); }
        ;

function:
          function stmt         { ex($2); freeNode($2); }
        | /* NULL */
        ;

stmt:
          ';'                                     { $$ = opr(';', 0, NULL, NULL); }
        | expr ';'                                { $$ = $1; }
        | BREAK ';'                               { $$ = br(); }
        | PRINT expr ';'                          { $$ = opr(PRINT, 1, $2); }
        | WHILE '(' expr ')' stmt                 { $$ = opr(WHILE, 2, $3, $5); }
        | DO stmt WHILE '(' expr ')' ';'          { $$ = opr(DO, 2, $5, $2); }
        | IF '(' expr ')' stmt %prec IFX          { $$ = opr(IF, 2, $3, $5); }
        | IF '(' expr ')' stmt ELSE stmt          { $$ = opr(IF, 3, $3, $5, $7); }
        | FOR '(' expr ';' expr ';' expr ')' stmt { $$ = opr(FOR, 4, $3, $5, $7, $9); }
        | SWITCH '(' IDENTIFIER ')' case            { 
                $$ = sw($3, $5); 
                set_break_parent($5, $$);
        }
        | '{' stmt_list '}'                       { $$ = $2; }
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
        | IDENTIFIER                      { $$ = $1; }
        | IDENTIFIER '=' expr          { $$ = opr('=', 2, $1, $3); }
        | IDENTIFIER PA expr           { $$ = opr(PA, 2, $1, $3); }
        | IDENTIFIER SA expr           { $$ = opr(SA, 2, $1, $3); }
        | IDENTIFIER MA expr           { $$ = opr(MA, 2, $1, $3); }
        | IDENTIFIER DA expr           { $$ = opr(DA, 2, $1, $3); }
        | IDENTIFIER RA expr           { $$ = opr(RA, 2, $1, $3); }
        | IDENTIFIER LSA expr          { $$ = opr(LSA, 2, $1, $3); }
        | IDENTIFIER RSA expr          { $$ = opr(RSA, 2, $1, $3); }
        | IDENTIFIER ANDA expr         { $$ = opr(ANDA, 2, $1, $3); }
        | IDENTIFIER EORA expr         { $$ = opr(EORA, 2, $1, $3); }
        | IDENTIFIER IORA expr         { $$ = opr(IORA, 2, $1, $3); }
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
        ;

%%

void set_break_parent(nodeType* node, nodeType* parent_switch) {
        if(node == NULL) return;
        
        if(node->type == typeBreak) {
               auto b = std::get<breakNodeType>(node->un);
               b.parent_switch = parent_switch;
        } else if(node->type == typeOpr) {
               auto opr = std::get<oprNodeType>(node->un);
                for(int i = 0; i < opr.op.size(); i++) {
                        set_break_parent(opr.op[i], parent_switch);
                }
        } else if(node->type == typeCase) {
               auto c = std::get<caseNodeType>(node->un);
               set_break_parent(c.self, parent_switch);
               set_break_parent(c.prev, parent_switch);
        }
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


// Create constant value / literal node.
nodeType *con(int value) {
    nodeType *p;

    /* allocate node */
    if ((p = new nodeType()) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeCon;
    p->un = std::variant<NODE_TYPES>(conNodeType{value});

    return p;
}

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
//    int i;
//
//    if (!p) return;
//    if (p->type == typeOpr) {
//        for (i = 0; i < p->opr.nops; i++)
//            freeNode(p->opr.op[i]);
//    }
//    free (p);
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
