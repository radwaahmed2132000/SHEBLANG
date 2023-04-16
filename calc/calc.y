// These includes/declarations are copy-pasted into the parser code as is.
%{
			#include <stdio.h> // Must include this since we use `fprintf` below.
			#include <stdlib.h>
			#include <stdarg.h>
			#include "calc_helpers.h"

			ParserNode *constant(int value);
			ParserNode *identifier(int i);
			ParserNode *operator(int oper, int num_operands, ...);
			void free_node(ParserNode *p);

			// Implemented in other files for an interpreter, a compiler, and a graph builder.
			int evaluate(ParserNode* p); // `ex()` in the manual.

			// A warning is emitted if these aren't declared
			int yylex(void); 			
			void yyerror(char *);
			int sym[26];
%}

%union {
 /* 
	 This union represents one of three things: An integer constant 
							A variable name (1 character) 
							An internal node pointing to an operator. 
	*/
	int iValue;
	char sIndex;
	ParserNode *nPtr;
};

/* 
	Integers map to the `iValue` member of the union.
	 While expressions map to the nPtr member.
	 The following production:
			expr: INTEGER { $$ = con($1); }
	 maps to the following C code in the generated parser:
			yylval.nPtr = con(yyvsp[0].iValue); 

	Where `yyvsp[0]` refers to the top of the value stack (parser implementation detail, no worries
	if you don't understand it).
*/
%token <iValue> INTEGER
%token <sIndex> VARIABLE
%token WHILE IF PRINT

/*
	Define operator associativity.
	Note that operators defined later (i.e. multiplication, division, unary minus, etc..) 
	have higher precedence. This helps since our grammar currently is ambiguous with respect to operator precedence.
*/
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'

%nonassoc UMINUS

%type <nPtr> stmt expr stmt_list

%%

/* 
	Recursively expand, evaluate, then print terms. 
	Expands to the left.
*/
program:
			 function			 { exit(0); }
			;

function:
				function stmt 		{ evaluate($2); free_node($2); }
				| /* NULL */ 
				;

stmt:
				';'													{ $$ = operator(';', 2, NULL, NULL); }
				 | expr ';'										{ $$ = $1; }
				 | PRINT expr ';' 					{ $$ = operator(PRINT, 1, $2); } 
				 // Assign to the value in the symbol table.
				 // $1			$2	$3
				 //	v				v		v
				 | VARIABLE '=' expr 	';'		      { $$ = operator('=', 2, identifier($1), $3); }				
				 | WHILE '(' expr ')' stmt	      { $$ = operator(WHILE, 2, $3, $5); }
				 | IF '(' expr ')' stmt %prec IFX { $$ = operator(IF, 2, $3, $5); }
				 | IF '(' expr ')' stmt ELSE stmt { $$ = operator(IF, 3, $3, $5, $7); }
				 | '{' stmt_list '}' 							{ $$ = $2; }
				 ;

stmt_list:
				 stmt								{ $$ = $1; }
				 | stmt_list stmt		{ $$ = operator(';', 2, $1, $2); }
				 ;
			
// Expands expressions to either integer literals, or add/subtract expressions.
expr:
		INTEGER					        { $$ = constant($1); }
		| VARIABLE			        { $$ = identifier($1); } // Return the value corresponding to the variable from the symbol table.
		| '-' expr %prec UMINUS { $$ = operator(UMINUS, 1, $2); }
		| expr '+' expr         { $$ = operator('+', 2, $1, $3); }
		| expr '-' expr         { $$ = operator('-', 2, $1, $3); }
		| expr '*' expr         { $$ = operator('*', 2, $1, $3); }
		| expr '/' expr         { $$ = operator('/', 2, $1, $3); }
		| expr '<' expr         { $$ = operator('<', 2, $1, $3); }
		| expr '>' expr         { $$ = operator('>', 2, $1, $3); }
		| expr GE expr          { $$ = operator(GE, 2, $1, $3); }
		| expr LE expr          { $$ = operator(LE, 2, $1, $3); }
		| expr NE expr          { $$ = operator(NE, 2, $1, $3); }
		| expr EQ expr          { $$ = operator(EQ, 2, $1, $3); }
		| '(' expr ')'	        { $$ = $2; }
		;
%%

#define SIZEOF_PARSERNODE ((char*)&p->constant - (char*)p)

ParserNode* constant(int value) {
	ParserNode *p;
	size_t node_size = SIZEOF_PARSERNODE + sizeof(ConstantNode);

	if((p = malloc(node_size)) == NULL)
		yyerror("Out of memory :(");

	p->type = TypeConstant;
	p->constant.value = value;

	return p;
}

ParserNode *identifier(int i) {
	ParserNode	*p;

	size_t node_size = SIZEOF_PARSERNODE + sizeof(IdentifierNode);
	if((p = malloc(node_size)) == NULL)
		yyerror("Out of memory :(");

	p->type = TypeIdentifier;
	p->identifier.symbol_table_index = i;
	
	return p;
}

ParserNode *operator(int oper, int num_operands, ...) {
	va_list ap;
	ParserNode *p;
	size_t node_size = SIZEOF_PARSERNODE + sizeof(OperatorNode) + (num_operands - 1) * sizeof(ParserNode*);

	if((p = malloc(node_size)) == NULL)
		yyerror("Out of memory :(");

	p->type = TypeOperator;
	p->opr.oper = oper;
	p->opr.num_operands = num_operands;

	va_start(ap, num_operands);
	for(int i = 0; i < num_operands; i++) 
		p->opr.operands[i] = va_arg(ap, ParserNode*);
	va_end(ap);

	return p;
}

void free_node(ParserNode* p) {
	if(!p) return;
	if(p->type == TypeOperator) {
		for(int i = 0; i < p->opr.num_operands; i++)
			free_node(p->opr.operands[i]);
	}

	free(p);
}

// Error callback provided by yacc.
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}

// Invoke yacc to parse from the standard input.
int main(void) {
	yyparse();
	return 0;
}
