%{
	#include <stdlib.h>   // Needed for `atoi`
	void yyerror(char *); // We use that below.

	// Includes the tokens defined in `calc.l`
	// yyval is the value of the token currently being parsed.
	// It's default type is `int`.
	#include "calc.tab.h" 
%}

%%
[0-9]+ 		{
						// Match integer numbers, set `yyval` to their integer value, and 
						// return an integer token.
	   				yylval = atoi(yytext);
	   				return INTEGER;
					}

[-+\n] 		return *yytext ;              /* Pluses, minuses, and newlines are returned as is.
																					 yytext is a pointer to the string that matched, *yytext here
																					 returns the character we matched. */

[ \t]  		;                             /* skip whitespace */

.	   			yyerror("invalid character"); /* `.` matches everything else. */

%%

// Return 1 if you don't have more work to do after lexing finishes,
// 0 otherwise.
int yywrap(void) {
	return 1;
}
