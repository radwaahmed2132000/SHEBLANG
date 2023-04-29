%{
#include <stdlib.h>
#include "cl.h"
#include "y.tab.h"
void yyerror(char *);
nodeTypeTag *id(const char* id);
%}

%%

0           {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[1-9][0-9]* {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[-()<>=+*/%!~&|^;{}.:] {
                return *yytext;
             }

"++"            return PP;
"--"            return MM;
"&&"            return AND;
"||"            return OR;
"<<"            return LS;
">>"            return RS;
">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
"+="            return PA;
"-="            return SA;
"*="            return MA;
"/="            return DA;
"%="            return RA;
"<<="           return LSA;
">>="           return RSA;
"&="            return ANDA;
"^="            return EORA;
"|="            return IORA;
"while"         return WHILE;
"do"            return DO;
"if"            return IF;
"else"          return ELSE;
"print"         return PRINT;
"for"           return FOR;
"switch"        return SWITCH;
"case"          return CASE;
"default"       return DEFAULT;
"break"         return BREAK;

[a-z_][a-z_0-9]* { 
                yylval.nPtr = id(yytext);
                return IDENTIFIER;
            }


[ \t\n]+        ;       /* ignore whitespace */

.               yyerror("Unknown character");
%%
int yywrap(void) {
    return 1;
}
