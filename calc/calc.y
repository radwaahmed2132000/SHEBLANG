// These includes/declarations are copy-pasted into the parser code as is.
%{
			#include <stdio.h> // Must include this since we use `fprintf` below.

			// A warning is emitted if these aren't declared
			int yylex(void); 			
			void yyerror(char *);
%}

%token INTEGER

%%

/* 
	Recursively expand, evaluate, then print terms. 
	Expands to the left.
*/
program:
		program expr '\n' { printf("%d\n", $2); }
		|
		;
			
// Expands expressions to either integer literals, or add/subtract expressions.
expr:
		INTEGER			{ $$ = $1; }
		| expr '+' expr { $$ = $1 + $3; }
		| expr '-' expr { $$ = $1 - $3; }
		;
%%

// Error callback provided by yacc.
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}

// Invoke yacc to parse from the standard input.
int main(void) {
	yyparse();
	return 0;
}
