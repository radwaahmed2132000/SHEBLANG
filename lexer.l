%{
#include <iostream>

#include "cl.h"
#include "parser.h"
%}

%option yylineno
%option noyywrap

CHAR_OR_ESCAPE [^']?|\\[nt']
STRING_OR_ESCAPE [^"]|\\[nt\\"]

/* Matches single quotes `''` containing `\n`, `\t`, `\'`, or any character other than
 * `'` (`'''` is illegal, but `'\''` is not) */ 
CHAR                '({CHAR_OR_ESCAPE})'

/* Same as a character, but contains multiple ones (illegal). For example `'bad char'` */
SINGLE_QUOTE_STRING '({CHAR_OR_ESCAPE})*'

/* Matches strings like the following: `'unclosed char` (note the missing closing `'`) */
UNCLOSED_CHAR       '({CHAR_OR_ESCAPE})[^'] 

/* Like characters, but uses `""` and can contain 0 or more characters. Note
 * that the outer double quotes must be surrounded by [] `["]`*/
STRING              ["]({STRING_OR_ESCAPE})*["]

/* Same as normal strings, but missing the closing string */
UNCLOSED_STRING     ["]({STRING_OR_ESCAPE})*[^"] 

DIGIT               [0-9]

INTEGER_NUMBER      {DIGIT}{DIGIT}*

/* Matches floats like `.9`, `0.2`, `1.0` */
FLOAT_NUMBER        {DIGIT}*\.{DIGIT}+


SCIENTIFIC_EXPONENTIAL ([eE][+-]?[0-9]+)

/* Constists of two rules, the first matches `2.e+9` or `2.1e-3` for example.
 * While the second matches .9e12 */
SCIENTIFIC_NOTATION ({DIGIT}+(\.{DIGIT}*)?{SCIENTIFIC_EXPONENTIAL}?|\.{DIGIT}+{SCIENTIFIC_EXPONENTIAL}?)

IDENT [a-zA-Z_][a-zA-Z_0-9]* 

COMMENT "#!".*
%%

{CHAR} {
    yylval.lineNo = yylineno;
    yylval.cValue = yytext[1]; // yytext[0] seems to contain nothing?
    return CHARACTER;
}

{SINGLE_QUOTE_STRING} {
    std::cerr << "Syntax error at line " << yylineno  << ": "
              << "Cannot use single quotes for strings. Instead use \"\"\n";
}

{UNCLOSED_CHAR} {
    std::cerr << "Syntax error at line " << yylineno << ": " 
              << "Missing closing quote (\')\n";
}

{STRING} {
    yylval.lineNo = yylineno;

    int stringLength = std::strlen(yytext);
    yylval.sValue = new char[stringLength];

    std::strcpy(yylval.sValue, yytext);

    return STR;
}

{UNCLOSED_STRING} {
    std::cerr << "Syntax error at line " << yylineno << ": "
              << "Missing closing double quote (\")!\n";
}

{INTEGER_NUMBER} {
    yylval.iValue = std::atoi(yytext);
    return INTEGER;
}


{FLOAT_NUMBER}|{SCIENTIFIC_NOTATION} {
    yylval.lineNo = yylineno;
    yylval.fValue = std::atof(yytext);
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

{IDENT} { 
    yylval.lineNo = yylineno;
    yylval.nPtr = id(yytext);
    return IDENTIFIER;
}

[ \t]+          ;       /* ignore whitespace */
{COMMENT}       ;       /* Ignore comments `#! this is a comment!` */
[\n]            currentLineNo++;  
.               yyerror("Unknown character");

%%
