%{
#include <stdlib.h>
#include <cstring>
#include <string>
#include "cl.h"
#include "y.tab.h"
void yyerror(char *);
nodeType *id(const char* id);
%}
%option yylineno
%%

[']([^']?|[\\][nt'])['] {
                yylval.lineNo = yylineno;
                yylval.cValue = std::string(yytext)[1];
                return CHARACTER;
            }

[']([^']?|[\\][nt'])*['] {
                std::string mes = "Syntax Error at line "+std::to_string(yylineno)+"!\nCannot use single quotes for strings. Instead use \"\" ";
                printf("%s",mes.c_str());
            }

[']([^']?|[\\][nt'])[^'] {
                std::string mes = "Syntax Error! Missing Single Quote (\') at line "+std::to_string(yylineno)+"!\n";
                printf("%s",mes.c_str());
            }

["]([^"]|[\\][nt"])*["] {
                yylval.lineNo = yylineno;
                std::string temp = std::string(yytext);
                temp = temp.substr(1,temp.size()-2);
                yylval.sValue = new char[temp.length()];
                strcpy(yylval.sValue, temp.c_str());
                return STR;
            }

["]([^"]|[\\][nt"])*[^"] {
                std::string mes = "Syntax Error! Missing Double Quote (\") at line "+std::to_string(yylineno-1)+"!\n";
                printf("%s",mes.c_str());
            }

0           {
                yylval.lineNo = yylineno;
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[1-9][0-9]* {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[0-9]+[.][0-9]+ {
                yylval.lineNo = yylineno;
                yylval.fValue = atof(yytext);
                return REAL;
            }

([0-9]+([.][0-9]*)?([eE][+-]?[0-9]+)?|[.][0-9]+([eE][+-]?[0-9]+)?) {
                yylval.fValue = atof(yytext);
                return REAL;
            }

[-()<>=+*/%!~&|^;{}.,:] {
                yylval.lineNo = yylineno;
                return *yytext;
             }

"true"      {
                yylval.lineNo = yylineno;
                yylval.bValue = true;
                return BOOLEAN;
            }

"false"     {
                yylval.lineNo = yylineno;
                yylval.bValue = false;
                return BOOLEAN;
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
"const"         return CONST;
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
"enum"          return ENUM;
"fn"            return FN;
"return"        return RETURN;
"::"            return SCOPE_RES;

[a-zA-Z_][a-zA-Z_0-9]* { 
                yylval.lineNo = yylineno;
                yylval.nPtr = id(yytext);
                return IDENTIFIER;
            }

[ \t]+        ;       /* ignore whitespace */
"#!".*        ;       /* Ignore comments `#! this is a comment!` */
[\n]            currentLineNo++;  
.               yyerror("Unknown character");
%%
int yywrap(void) {
    return 1;
}
