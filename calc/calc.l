%{
	#include <stdlib.h>   // Needed for `atoi`
	void yyerror(char *); // We use that below.

	// Includes the tokens defined in `calc.l`
	// yyval is the value of the token currently being parsed.
	// It's default type is `int`.
	#include "calc.tab.h" 
%}

%%
[a-z] 		{
	// Match single character lowercase variable names.
	// Convert the matched character to an index into our symbol table by subtracting from 'a'.
	// This follows from the C's ASCII representation of characters where 'a' = 97, 'z'=122
	// This maps them to 0 -> 26.
	yylval = *yytext - 'a';
	return VARIABLE;
}

[0-9]+ 		{
	// Match integer numbers, set `yyval` to their integer value, and 
	// return an integer token.
	yylval = atoi(yytext);
	return INTEGER;
}

[-+()=/*\n] {
	// Match operators as they are.
	return *yytext;
}

[ \t]  		;                             /* skip whitespace */

.	   			yyerror("invalid character"); /* `.` matches everything else. */

%%

// Return 1 if you don't have more work to do after lexing finishes,
// 0 otherwise.
int yywrap(void) {
	return 1;
}
