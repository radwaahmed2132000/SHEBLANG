/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    INTEGER = 259,
    REAL = 260,
    BOOLEAN = 261,
    CHARACTER = 262,
    STR = 263,
    WHILE = 264,
    IF = 265,
    PRINT = 266,
    DO = 267,
    FOR = 268,
    SWITCH = 269,
    CASE = 270,
    DEFAULT = 271,
    CASE_LIST = 272,
    BREAK = 273,
    ENUM = 274,
    FN = 275,
    RETURN = 276,
    CONST = 277,
    INT = 278,
    FLOAT = 279,
    BOOL = 280,
    CHAR = 281,
    STRING = 282,
    IFX = 283,
    ELSE = 284,
    PA = 285,
    SA = 286,
    MA = 287,
    DA = 288,
    RA = 289,
    LSA = 290,
    RSA = 291,
    ANDA = 292,
    EORA = 293,
    IORA = 294,
    OR = 295,
    AND = 296,
    EQ = 297,
    NE = 298,
    GE = 299,
    LE = 300,
    LS = 301,
    RS = 302,
    UPLUS = 303,
    UMINUS = 304,
    PP = 305,
    MM = 306
  };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define INTEGER 259
#define REAL 260
#define BOOLEAN 261
#define CHARACTER 262
#define STR 263
#define WHILE 264
#define IF 265
#define PRINT 266
#define DO 267
#define FOR 268
#define SWITCH 269
#define CASE 270
#define DEFAULT 271
#define CASE_LIST 272
#define BREAK 273
#define ENUM 274
#define FN 275
#define RETURN 276
#define CONST 277
#define INT 278
#define FLOAT 279
#define BOOL 280
#define CHAR 281
#define STRING 282
#define IFX 283
#define ELSE 284
#define PA 285
#define SA 286
#define MA 287
#define DA 288
#define RA 289
#define LSA 290
#define RSA 291
#define ANDA 292
#define EORA 293
#define IORA 294
#define OR 295
#define AND 296
#define EQ 297
#define NE 298
#define GE 299
#define LE 300
#define LS 301
#define RS 302
#define UPLUS 303
#define UMINUS 304
#define PP 305
#define MM 306

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 39 "cl.y"

    int iValue;                 /* integer value */
    float fValue;               /* double value */
    bool bValue;                /* boolean value */
    char cValue;                /* char value */
    char* sValue;               /* string value */
    char* idName;               /* identifier name */
    nodeType *nPtr;             /* node pointer */

#line 169 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
