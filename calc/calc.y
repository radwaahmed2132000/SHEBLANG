%token INTEGER VARIABLE

// Define operator associativity.
// Note that operators defined later (i.e. multiplication and division) have higher precedence.
// This helps since our grammar currently is ambiguous with respect to operator precedence.
%left '+' '-'
%left '*' '/'

// These includes/declarations are copy-pasted into the parser code as is.
%{
			#include <stdio.h> // Must include this since we use `fprintf` below.

			// A warning is emitted if these aren't declared
			int yylex(void); 			
			void yyerror(char *);
			int sym[26];
%}
%%

/* 
	Recursively expand, evaluate, then print terms. 
	Expands to the left.
*/
program:
			 program statement '\n'
			|
			;

statement:
				 expr										{ printf("%d\n", $1); } // Evaluate the expression and print it.
				 // Assign to the value in the symbol table.
				 // $1			$2	$3
				 //	v				v		v
				 | VARIABLE '=' expr 		{ sym[$1] = $3; }				
				 ;
			
// Expands expressions to either integer literals, or add/subtract expressions.
expr:
		INTEGER
		| VARIABLE			{ $$ = sym[$1]; } // Return the value corresponding to the variable from the symbol table.
		| expr '+' expr { $$ = $1 + $3; }
		| expr '-' expr { $$ = $1 - $3; }
		| expr '*' expr { $$ = $1 * $3; }
		| expr '/' expr { $$ = $1 / $3; }
		| '(' expr ')'	{ $$ = $2; }
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
