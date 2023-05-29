/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 ".//parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unordered_map>
#include <string>
#include "cl.h"
#include "node_constructors.h"
#include <variant>


/* prototypes */
void freeNode(nodeType *p);
Value ex(nodeType *p);
void printSymbolTables();
void appendSymbolTable(int i);

int yylex(void);
void yyerror(char *s);
extern int yylineno;            /* from lexer */


#line 94 "/home/tarek/code-projects/SHEBLANG/parser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_WHILE = 3,                      /* WHILE  */
  YYSYMBOL_IF = 4,                         /* IF  */
  YYSYMBOL_PRINT = 5,                      /* PRINT  */
  YYSYMBOL_DO = 6,                         /* DO  */
  YYSYMBOL_FOR = 7,                        /* FOR  */
  YYSYMBOL_SWITCH = 8,                     /* SWITCH  */
  YYSYMBOL_CASE = 9,                       /* CASE  */
  YYSYMBOL_DEFAULT = 10,                   /* DEFAULT  */
  YYSYMBOL_CASE_LIST = 11,                 /* CASE_LIST  */
  YYSYMBOL_BREAK = 12,                     /* BREAK  */
  YYSYMBOL_ENUM = 13,                      /* ENUM  */
  YYSYMBOL_FN = 14,                        /* FN  */
  YYSYMBOL_RETURN = 15,                    /* RETURN  */
  YYSYMBOL_CONST = 16,                     /* CONST  */
  YYSYMBOL_INT = 17,                       /* INT  */
  YYSYMBOL_FLOAT = 18,                     /* FLOAT  */
  YYSYMBOL_BOOL = 19,                      /* BOOL  */
  YYSYMBOL_CHAR = 20,                      /* CHAR  */
  YYSYMBOL_STRING = 21,                    /* STRING  */
  YYSYMBOL_SCOPE_RES = 22,                 /* SCOPE_RES  */
  YYSYMBOL_IFX = 23,                       /* IFX  */
  YYSYMBOL_ELSE = 24,                      /* ELSE  */
  YYSYMBOL_25_ = 25,                       /* '='  */
  YYSYMBOL_PA = 26,                        /* PA  */
  YYSYMBOL_SA = 27,                        /* SA  */
  YYSYMBOL_MA = 28,                        /* MA  */
  YYSYMBOL_DA = 29,                        /* DA  */
  YYSYMBOL_RA = 30,                        /* RA  */
  YYSYMBOL_LSA = 31,                       /* LSA  */
  YYSYMBOL_RSA = 32,                       /* RSA  */
  YYSYMBOL_ANDA = 33,                      /* ANDA  */
  YYSYMBOL_EORA = 34,                      /* EORA  */
  YYSYMBOL_IORA = 35,                      /* IORA  */
  YYSYMBOL_OR = 36,                        /* OR  */
  YYSYMBOL_AND = 37,                       /* AND  */
  YYSYMBOL_38_ = 38,                       /* '|'  */
  YYSYMBOL_39_ = 39,                       /* '^'  */
  YYSYMBOL_40_ = 40,                       /* '&'  */
  YYSYMBOL_EQ = 41,                        /* EQ  */
  YYSYMBOL_NE = 42,                        /* NE  */
  YYSYMBOL_GE = 43,                        /* GE  */
  YYSYMBOL_LE = 44,                        /* LE  */
  YYSYMBOL_45_ = 45,                       /* '>'  */
  YYSYMBOL_46_ = 46,                       /* '<'  */
  YYSYMBOL_LS = 47,                        /* LS  */
  YYSYMBOL_RS = 48,                        /* RS  */
  YYSYMBOL_49_ = 49,                       /* '+'  */
  YYSYMBOL_50_ = 50,                       /* '-'  */
  YYSYMBOL_51_ = 51,                       /* '*'  */
  YYSYMBOL_52_ = 52,                       /* '/'  */
  YYSYMBOL_53_ = 53,                       /* '%'  */
  YYSYMBOL_UPLUS = 54,                     /* UPLUS  */
  YYSYMBOL_UMINUS = 55,                    /* UMINUS  */
  YYSYMBOL_56_ = 56,                       /* '!'  */
  YYSYMBOL_57_ = 57,                       /* '~'  */
  YYSYMBOL_PP = 58,                        /* PP  */
  YYSYMBOL_MM = 59,                        /* MM  */
  YYSYMBOL_IDENTIFIER = 60,                /* IDENTIFIER  */
  YYSYMBOL_INTEGER = 61,                   /* INTEGER  */
  YYSYMBOL_REAL = 62,                      /* REAL  */
  YYSYMBOL_BOOLEAN = 63,                   /* BOOLEAN  */
  YYSYMBOL_CHARACTER = 64,                 /* CHARACTER  */
  YYSYMBOL_STR = 65,                       /* STR  */
  YYSYMBOL_66_ = 66,                       /* ';'  */
  YYSYMBOL_67_ = 67,                       /* '('  */
  YYSYMBOL_68_ = 68,                       /* ')'  */
  YYSYMBOL_69_ = 69,                       /* '{'  */
  YYSYMBOL_70_ = 70,                       /* '}'  */
  YYSYMBOL_71_ = 71,                       /* ':'  */
  YYSYMBOL_72_ = 72,                       /* ','  */
  YYSYMBOL_YYACCEPT = 73,                  /* $accept  */
  YYSYMBOL_program = 74,                   /* program  */
  YYSYMBOL_var_decl = 75,                  /* var_decl  */
  YYSYMBOL_var_defn = 76,                  /* var_defn  */
  YYSYMBOL_stmt = 77,                      /* stmt  */
  YYSYMBOL_return_statement = 78,          /* return_statement  */
  YYSYMBOL_case = 79,                      /* case  */
  YYSYMBOL_case_list = 80,                 /* case_list  */
  YYSYMBOL_literal = 81,                   /* literal  */
  YYSYMBOL_stmt_list = 82,                 /* stmt_list  */
  YYSYMBOL_expr = 83,                      /* expr  */
  YYSYMBOL_enum_use = 84,                  /* enum_use  */
  YYSYMBOL_function_call = 85,             /* function_call  */
  YYSYMBOL_expr_list = 86,                 /* expr_list  */
  YYSYMBOL_enum_defn = 87,                 /* enum_defn  */
  YYSYMBOL_identifier_list = 88,           /* identifier_list  */
  YYSYMBOL_function_return_type = 89,      /* function_return_type  */
  YYSYMBOL_function_parameter_list = 90,   /* function_parameter_list  */
  YYSYMBOL_function_defn = 91              /* function_defn  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  70
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   883

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  91
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  194

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   307


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,     2,     2,     2,    53,    40,     2,
      67,    68,    51,    49,    72,    50,     2,    52,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    66,
      46,    25,    45,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,    38,    70,    57,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    41,    42,    43,    44,    47,    48,    54,    55,
      58,    59,    60,    61,    62,    63,    64,    65
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    61,    61,    87,    91,    95,    99,   100,   104,   105,
     106,   107,   108,   109,   110,   111,   115,   119,   120,   121,
     122,   123,   124,   125,   129,   133,   134,   135,   139,   140,
     144,   144,   144,   144,   144,   144,   147,   148,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   193,   196,   201,
     202,   203,   207,   214,   215,   216,   220,   221,   225,   226,
     227,   230
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "WHILE", "IF", "PRINT",
  "DO", "FOR", "SWITCH", "CASE", "DEFAULT", "CASE_LIST", "BREAK", "ENUM",
  "FN", "RETURN", "CONST", "INT", "FLOAT", "BOOL", "CHAR", "STRING",
  "SCOPE_RES", "IFX", "ELSE", "'='", "PA", "SA", "MA", "DA", "RA", "LSA",
  "RSA", "ANDA", "EORA", "IORA", "OR", "AND", "'|'", "'^'", "'&'", "EQ",
  "NE", "GE", "LE", "'>'", "'<'", "LS", "RS", "'+'", "'-'", "'*'", "'/'",
  "'%'", "UPLUS", "UMINUS", "'!'", "'~'", "PP", "MM", "IDENTIFIER",
  "INTEGER", "REAL", "BOOLEAN", "CHARACTER", "STR", "';'", "'('", "')'",
  "'{'", "'}'", "':'", "','", "$accept", "program", "var_decl", "var_defn",
  "stmt", "return_statement", "case", "case_list", "literal", "stmt_list",
  "expr", "enum_use", "function_call", "expr_list", "enum_defn",
  "identifier_list", "function_return_type", "function_parameter_list",
  "function_defn", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-162)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     208,   -41,   -37,   799,   208,   -24,   -19,   -50,   -13,   -10,
     799,    -5,   799,   799,   799,   799,   101,  -162,  -162,  -162,
    -162,  -162,  -162,   799,   208,    54,   -20,  -162,  -162,  -162,
    -162,   208,   489,  -162,  -162,  -162,  -162,   799,   766,   254,
     520,    57,    -5,   799,  -162,    -8,     1,   551,     9,    42,
     -18,   -18,   -18,   -18,    10,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,  -162,   799,   291,   140,
    -162,   799,  -162,  -162,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,  -162,  -162,  -162,   324,   208,   357,  -162,     4,
      47,   799,   390,    13,    -5,  -162,   799,  -162,   675,   675,
     675,   675,   675,   675,   675,   675,   675,   675,   675,   675,
     -51,  -162,  -162,   582,   698,   720,   741,   761,   794,   824,
     824,   -39,   -39,   -39,   -39,   -14,   -14,     0,     0,   -18,
     -18,   -18,   208,  -162,   208,   799,   613,    -3,  -162,   -47,
    -162,   -44,   644,  -162,   799,  -162,  -162,    61,   423,   799,
     799,    16,    -3,  -162,    22,    29,    31,    -5,  -162,   675,
     208,    46,   456,   255,   208,  -162,    -6,  -162,  -162,  -162,
      44,  -162,  -162,  -162,   208,   208,  -162,  -162,  -162,   208,
    -162,  -162,   179,  -162
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    35,    30,    31,    32,
      33,    34,     6,     0,     0,     0,     0,    19,    36,    23,
      38,     2,     0,    76,    75,    21,    22,     0,     0,    35,
       0,     0,     0,     0,    13,     0,     0,     0,     0,     0,
      41,    42,    43,    44,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,    81,     0,     0,
       1,     0,    18,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    39,    40,    12,     0,     0,     0,    14,     0,
       0,     0,     0,    85,    90,    24,     0,    77,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    80,
       0,    74,    17,     0,    69,    68,    57,    58,    56,    73,
      72,    70,    71,    67,    66,    59,    60,    61,    62,    63,
      64,    65,     0,     8,     0,     0,     0,     0,    84,     0,
      89,     0,     0,    78,     0,     5,    15,     9,     0,     0,
       0,     0,     0,    11,     0,     0,    87,     0,    20,    79,
       0,     0,     0,     0,     0,    28,     0,    82,    83,    86,
       0,    88,    10,    16,     0,     0,    26,    27,    29,     0,
       7,    25,     0,    91
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -162,  -162,     7,    72,    -4,  -162,  -161,  -162,  -162,   -22,
      19,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    25,    26,    27,    28,    29,   163,   176,    30,    31,
      32,    33,    34,   120,    35,   149,   180,   151,    36
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      41,   175,    69,   160,   161,    71,   160,   161,    85,    86,
      87,    88,    89,    90,    91,   188,    44,   153,    49,    92,
      93,   154,    40,   164,   166,   165,    37,    73,   167,    47,
      38,    50,    51,    52,    53,    87,    88,    89,    90,    91,
      92,    93,    68,    42,    92,    93,    72,    45,    43,   100,
      46,    89,    90,    91,    70,    48,    95,    97,    92,    93,
      99,   103,   102,   162,   187,    73,   162,   106,   104,    66,
     107,   145,    71,   148,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   170,   119,   174,   177,   178,
     123,   179,   143,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   150,   183,   189,   101,     0,     0,     0,     0,     0,
     146,     0,     0,    54,     0,   152,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,     0,   156,     0,
     157,     0,     0,     1,     2,     3,     4,     5,     6,     0,
       0,     0,     7,     8,     9,    10,    11,     0,     0,     0,
       0,    66,     0,     0,   158,     0,   182,   192,    67,     0,
     186,     0,     0,   169,   181,     0,     0,     0,   172,   173,
     190,   191,     1,     2,     3,     4,     5,     6,    73,    12,
      13,     7,     8,     9,    10,    11,    14,    15,     0,     0,
      16,    17,    18,    19,    20,    21,    22,    23,     0,    24,
     122,     1,     2,     3,     4,     5,     6,     0,     0,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,    13,
       0,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,    18,    19,    20,    21,    22,    23,     0,    24,   193,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,     0,    14,    15,     0,     0,    16,    17,
      18,    19,    20,    21,    22,    23,    54,    24,     0,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
       0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,    92,    93,     0,     0,     0,     0,     0,
       0,    67,     0,     0,     0,     0,   185,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,    92,
      93,     0,     0,     0,     0,     0,     0,     0,     0,   121,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,    92,    93,     0,     0,     0,     0,     0,     0,
       0,     0,   142,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,    92,    93,     0,     0,     0,
       0,     0,     0,     0,     0,   144,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,   147,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,    92,    93,     0,     0,     0,     0,     0,     0,     0,
       0,   171,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,    92,    93,     0,     0,     0,     0,
       0,     0,     0,     0,   184,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,    92,    93,     0,
       0,     0,     0,     0,     0,    94,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,    92,    93,
       0,     0,     0,     0,     0,     0,    98,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,    92,
      93,     0,     0,     0,     0,     0,     0,   105,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
      92,    93,     0,     0,     0,     0,     0,     0,   155,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,    92,    93,     0,     0,     0,     0,     0,     0,   159,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,    92,    93,     0,     0,     0,     0,     0,     0,
     168,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,    92,    93,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,    92,    93,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,    92,    93,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,    92,
      93,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    12,    13,     0,     0,    92,
      93,     0,    14,    15,     0,     0,    39,    17,    18,    19,
      20,    21,     0,    23,    96,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    12,    13,
       0,     0,    92,    93,     0,    14,    15,     0,     0,    39,
      17,    18,    19,    20,    21,     0,    23,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,    92,    93
};

static const yytype_int16 yycheck[] =
{
       4,   162,    24,     9,    10,    25,     9,    10,    47,    48,
      49,    50,    51,    52,    53,   176,    66,    68,    11,    58,
      59,    72,     3,    70,    68,    72,    67,    31,    72,    10,
      67,    12,    13,    14,    15,    49,    50,    51,    52,    53,
      58,    59,    23,    67,    58,    59,    66,    60,    67,    42,
      60,    51,    52,    53,     0,    60,    37,    38,    58,    59,
       3,    69,    43,    69,    70,    69,    69,    25,    67,    60,
      60,    67,    25,    60,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    24,    67,    71,    66,    60,
      71,    60,    96,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,   104,    66,    69,    42,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    22,    -1,   106,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,   142,    -1,
     144,    -1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    12,    13,    14,    15,    16,    -1,    -1,    -1,
      -1,    60,    -1,    -1,   145,    -1,   170,   189,    67,    -1,
     174,    -1,    -1,   154,   167,    -1,    -1,    -1,   159,   160,
     184,   185,     3,     4,     5,     6,     7,     8,   192,    49,
      50,    12,    13,    14,    15,    16,    56,    57,    -1,    -1,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      12,    13,    14,    15,    16,    -1,    -1,    -1,    49,    50,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    -1,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    -1,
      -1,    -1,    -1,    -1,    56,    57,    -1,    -1,    60,    61,
      62,    63,    64,    65,    66,    67,    22,    69,    -1,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    71,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,    58,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    -1,    58,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    66,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    58,    59,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    58,    59,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,    59,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,
      59,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    49,    50,    -1,    -1,    58,
      59,    -1,    56,    57,    -1,    -1,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    49,    50,
      -1,    -1,    58,    59,    -1,    56,    57,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    58,    59
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    12,    13,    14,
      15,    16,    49,    50,    56,    57,    60,    61,    62,    63,
      64,    65,    66,    67,    69,    74,    75,    76,    77,    78,
      81,    82,    83,    84,    85,    87,    91,    67,    67,    60,
      83,    77,    67,    67,    66,    60,    60,    83,    60,    75,
      83,    83,    83,    83,    22,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    60,    67,    83,    82,
       0,    25,    66,    77,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    58,    59,    66,    83,    68,    83,    66,     3,
      75,    76,    83,    69,    67,    66,    25,    60,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      86,    68,    70,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    68,    77,    68,    67,    83,    68,    60,    88,
      75,    90,    83,    68,    72,    66,    77,    77,    83,    66,
       9,    10,    69,    79,    70,    72,    68,    72,    66,    83,
      24,    68,    83,    83,    71,    79,    80,    66,    60,    60,
      89,    75,    77,    66,    68,    71,    77,    70,    79,    69,
      77,    77,    82,    70
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    73,    74,    74,    75,    76,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    78,    79,    79,    79,    80,    80,
      81,    81,    81,    81,    81,    81,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    84,    85,    86,
      86,    86,    87,    88,    88,    88,    89,    89,    90,    90,
      90,    91
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     4,     1,     8,     4,     5,
       7,     5,     2,     2,     3,     5,     7,     3,     2,     1,
       5,     1,     1,     1,     3,     4,     3,     3,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     2,
       2,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     3,     4,     3,
       1,     0,     6,     3,     1,     0,     1,     0,     3,     1,
       0,     9
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: stmt_list  */
#line 61 ".//parser.y"
                  { 
                /* First, create all the Symbol Tables. */
                setup_scopes((yyvsp[0].node));
                /* Perform Semantic Analysis on the code. */
                auto result = semantic_analysis((yyvsp[0].node));
                /* Print warnings if exist. */
                if (warningsOutput.sizeError > 0) {
                    for (auto& warning : std::get<ErrorType>(warningsOutput)) {
                        printf("%s\n", warning.c_str());
                    }
                }
                /* Print semantic errors if exist. */
                if (errorsOutput.sizeError > 0) {
                    for (auto& error : std::get<ErrorType>(errorsOutput)) {
                        printf("%s\n", error.c_str());
                    }
                    exit(1);
                }
                /* Excute the code using the interpreter. */
                ex((yyvsp[0].node));
                /* Print the final version of the symbol tables. */
                printSymbolTables();
                /* Free Memory used. */
                freeNode((yyvsp[0].node));
                exit(0);
            }
#line 1577 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 4: /* var_decl: IDENTIFIER IDENTIFIER  */
#line 91 ".//parser.y"
                                    { (yyval.node) = varDecl((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1583 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 5: /* var_defn: var_decl '=' expr ';'  */
#line 95 ".//parser.y"
                                     { 
            (yyval.node) = varDefn((yyvsp[-3].node), (yyvsp[-1].node), false);
        }
#line 1591 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 6: /* stmt: ';'  */
#line 99 ".//parser.y"
                                                { (yyval.node) = opr(';', 0); }
#line 1597 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 7: /* stmt: FOR '(' var_defn expr ';' expr ')' stmt  */
#line 100 ".//parser.y"
                                                  { 
                (yyval.node) = for_loop((yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); 
                set_break_parent((yyvsp[0].node), (yyval.node));
        }
#line 1606 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 8: /* stmt: IF '(' ')' stmt  */
#line 104 ".//parser.y"
                                                  { (yyval.node) = (yyvsp[0].node); printf("%s",std::string("Syntax Error at line "+std::to_string(yylineno)+". If's condition can\'t be empty.\n").c_str()); }
#line 1612 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 9: /* stmt: IF '(' expr ')' stmt  */
#line 105 ".//parser.y"
                                                  { currentLineNo = ((yylsp[-4])).first_line; (yyval.node) = opr(IF, 2, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1618 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 10: /* stmt: IF '(' expr ')' stmt ELSE stmt  */
#line 106 ".//parser.y"
                                                  { (yyval.node) = opr(IF, 3, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1624 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 11: /* stmt: SWITCH '(' expr ')' case  */
#line 107 ".//parser.y"
                                                  { currentLineNo = (yylsp[-4]).first_line; (yyval.node) = sw((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1630 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 12: /* stmt: expr ';'  */
#line 108 ".//parser.y"
                                                  { (yyval.node) = (yyvsp[-1].node); }
#line 1636 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 13: /* stmt: BREAK ';'  */
#line 109 ".//parser.y"
                                                  { (yyval.node) = br(); }
#line 1642 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 14: /* stmt: PRINT expr ';'  */
#line 110 ".//parser.y"
                                                  { (yyval.node) = opr(PRINT, 1, (yyvsp[-1].node)); }
#line 1648 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 15: /* stmt: WHILE '(' expr ')' stmt  */
#line 111 ".//parser.y"
                                                  { 
                (yyval.node) = while_loop((yyvsp[-2].node), (yyvsp[0].node)); 
                set_break_parent((yyvsp[0].node), (yyval.node));
        }
#line 1657 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 16: /* stmt: DO stmt WHILE '(' expr ')' ';'  */
#line 115 ".//parser.y"
                                                  { 
                (yyval.node) = do_while_loop((yyvsp[-2].node), (yyvsp[-5].node)); 
                set_break_parent((yyvsp[-5].node), (yyval.node));
        }
#line 1666 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 17: /* stmt: '{' stmt_list '}'  */
#line 119 ".//parser.y"
                                                  { (yyvsp[-1].node)->addNewScope = true; (yyval.node) = (yyvsp[-1].node); }
#line 1672 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 18: /* stmt: var_decl ';'  */
#line 120 ".//parser.y"
                                                  { (yyval.node) = (yyvsp[-1].node); }
#line 1678 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 19: /* stmt: var_defn  */
#line 121 ".//parser.y"
                                                  { (yyval.node) = (yyvsp[0].node); }
#line 1684 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 20: /* stmt: CONST var_decl '=' expr ';'  */
#line 122 ".//parser.y"
                                                  { (yyval.node) = varDefn((yyvsp[-3].node), (yyvsp[-1].node), true); }
#line 1690 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 24: /* return_statement: RETURN expr ';'  */
#line 129 ".//parser.y"
                                { (yyval.node) = opr(RETURN, 1, (yyvsp[-1].node)); }
#line 1696 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 25: /* case: CASE expr ':' stmt  */
#line 133 ".//parser.y"
                                { (yyval.node) = cs((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1702 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 26: /* case: DEFAULT ':' stmt  */
#line 134 ".//parser.y"
                                { (yyval.node) = cs(nullptr, (yyvsp[0].node)); }
#line 1708 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 27: /* case: '{' case_list '}'  */
#line 135 ".//parser.y"
                                { (yyvsp[-1].node)->addNewScope = true; (yyval.node) = (yyvsp[-1].node); }
#line 1714 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 28: /* case_list: case  */
#line 139 ".//parser.y"
                                  { (yyval.node) = linkedListStump<caseNodeType>((yyvsp[0].node)); }
#line 1720 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 29: /* case_list: case_list case  */
#line 140 ".//parser.y"
                                  { (yyval.node) = appendToLinkedList<caseNodeType>((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1726 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 36: /* stmt_list: stmt  */
#line 147 ".//parser.y"
                                { (yyval.node) = linkedListStump<StatementList>(statementList((yyvsp[0].node))); }
#line 1732 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 37: /* stmt_list: stmt_list stmt  */
#line 148 ".//parser.y"
                                { (yyval.node) = appendToLinkedList<StatementList>((yyvsp[-1].node), statementList((yyvsp[0].node))); }
#line 1738 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 38: /* expr: literal  */
#line 151 ".//parser.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1744 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 39: /* expr: expr PP  */
#line 152 ".//parser.y"
                                        { (yyval.node) = opr(PP, 1, (yyvsp[-1].node)); }
#line 1750 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 40: /* expr: expr MM  */
#line 153 ".//parser.y"
                                        { (yyval.node) = opr(MM, 1, (yyvsp[-1].node)); }
#line 1756 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 41: /* expr: '+' expr  */
#line 154 ".//parser.y"
                                        { (yyval.node) = opr(UPLUS, 1, (yyvsp[0].node)); }
#line 1762 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 42: /* expr: '-' expr  */
#line 155 ".//parser.y"
                                        { (yyval.node) = opr(UMINUS, 1, (yyvsp[0].node)); }
#line 1768 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 43: /* expr: '!' expr  */
#line 156 ".//parser.y"
                                        { (yyval.node) = opr('!', 1, (yyvsp[0].node)); }
#line 1774 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 44: /* expr: '~' expr  */
#line 157 ".//parser.y"
                                        { (yyval.node) = opr('~', 1, (yyvsp[0].node)); }
#line 1780 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 45: /* expr: IDENTIFIER '=' expr  */
#line 158 ".//parser.y"
                                        { (yyval.node) = BinOp::assign((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1786 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 46: /* expr: IDENTIFIER PA expr  */
#line 159 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::Add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1792 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 47: /* expr: IDENTIFIER SA expr  */
#line 160 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::Sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1798 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 48: /* expr: IDENTIFIER MA expr  */
#line 161 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::Mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1804 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 49: /* expr: IDENTIFIER DA expr  */
#line 162 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::Div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1810 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 50: /* expr: IDENTIFIER RA expr  */
#line 163 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::Mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1816 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 51: /* expr: IDENTIFIER LSA expr  */
#line 164 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::LShift,  (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1822 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 52: /* expr: IDENTIFIER RSA expr  */
#line 165 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::RShift,  (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1828 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 53: /* expr: IDENTIFIER ANDA expr  */
#line 166 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::BitAnd, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1834 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 54: /* expr: IDENTIFIER EORA expr  */
#line 167 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::BitXor, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1840 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 55: /* expr: IDENTIFIER IORA expr  */
#line 168 ".//parser.y"
                                        { (yyval.node) = BinOp::opAssign(BinOper::BitOr, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1846 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 56: /* expr: expr '&' expr  */
#line 169 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::BitAnd, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1852 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 57: /* expr: expr '|' expr  */
#line 170 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::BitOr, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1858 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 58: /* expr: expr '^' expr  */
#line 171 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::BitXor, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1864 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 59: /* expr: expr LS expr  */
#line 172 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::LShift, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1870 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 60: /* expr: expr RS expr  */
#line 173 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::RShift, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1876 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 61: /* expr: expr '+' expr  */
#line 174 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1882 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 62: /* expr: expr '-' expr  */
#line 175 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1888 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 63: /* expr: expr '*' expr  */
#line 176 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1894 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 64: /* expr: expr '/' expr  */
#line 177 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1900 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 65: /* expr: expr '%' expr  */
#line 178 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1906 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 66: /* expr: expr '<' expr  */
#line 179 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::LessThan, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1912 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 67: /* expr: expr '>' expr  */
#line 180 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::GreaterThan, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1918 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 68: /* expr: expr AND expr  */
#line 181 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::And, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1924 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 69: /* expr: expr OR expr  */
#line 182 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1930 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 70: /* expr: expr GE expr  */
#line 183 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::GreaterEqual, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1936 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 71: /* expr: expr LE expr  */
#line 184 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::LessEqual, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1942 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 72: /* expr: expr NE expr  */
#line 185 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::NotEqual, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1948 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 73: /* expr: expr EQ expr  */
#line 186 ".//parser.y"
                                        { (yyval.node) = BinOp::node(BinOper::Equal, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1954 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 74: /* expr: '(' expr ')'  */
#line 187 ".//parser.y"
                                        { (yyval.node) = (yyvsp[-1].node); }
#line 1960 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 75: /* expr: function_call  */
#line 188 ".//parser.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1966 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 77: /* enum_use: IDENTIFIER SCOPE_RES IDENTIFIER  */
#line 193 ".//parser.y"
                                        { (yyval.node) = enum_use((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1972 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 78: /* function_call: IDENTIFIER '(' expr_list ')'  */
#line 196 ".//parser.y"
                                          { 
                     (yyval.node) = functionCall((yyvsp[-3].node), (yyvsp[-1].node));
             }
#line 1980 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 79: /* expr_list: expr_list ',' expr  */
#line 201 ".//parser.y"
                            { (yyval.node) = appendToLinkedList<ExprListNode>((yyvsp[-2].node), exprListNode((yyvsp[0].node))); }
#line 1986 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 80: /* expr_list: expr  */
#line 202 ".//parser.y"
                            { (yyval.node) = linkedListStump<ExprListNode>(exprListNode((yyvsp[0].node))); }
#line 1992 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 81: /* expr_list: %empty  */
#line 203 ".//parser.y"
                            { (yyval.node) = linkedListStump<ExprListNode>(nullptr); }
#line 1998 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 82: /* enum_defn: ENUM IDENTIFIER '{' identifier_list '}' ';'  */
#line 207 ".//parser.y"
                                                     { 
                auto idListEnd =  ((yyvsp[-2].node))->as<IdentifierListNode>();
                auto enumMembers = idListEnd.toVec();
                (yyval.node) = enum_defn((yyvsp[-4].node), enumMembers);
         }
#line 2008 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 83: /* identifier_list: identifier_list ',' IDENTIFIER  */
#line 214 ".//parser.y"
                                                { (yyval.node) = appendToLinkedList<IdentifierListNode>((yyvsp[-2].node), identifierListNode((yyvsp[0].node), false)); }
#line 2014 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 84: /* identifier_list: IDENTIFIER  */
#line 215 ".//parser.y"
                                                { (yyval.node) = linkedListStump<IdentifierListNode>(identifierListNode((yyvsp[0].node), false)); }
#line 2020 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 85: /* identifier_list: %empty  */
#line 216 ".//parser.y"
                                                { (yyval.node) = linkedListStump<IdentifierListNode>(nullptr); }
#line 2026 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 86: /* function_return_type: IDENTIFIER  */
#line 220 ".//parser.y"
                                        {  (yyval.node) = (yyvsp[0].node); }
#line 2032 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 87: /* function_return_type: %empty  */
#line 221 ".//parser.y"
                                        {  (yyval.node) = id("void"); }
#line 2038 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 88: /* function_parameter_list: function_parameter_list ',' var_decl  */
#line 225 ".//parser.y"
                                                             { (yyval.node) = appendToLinkedList<VarDecl>((yyvsp[-2].node), (yyvsp[0].node)); }
#line 2044 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 89: /* function_parameter_list: var_decl  */
#line 226 ".//parser.y"
                                                             { (yyval.node) = linkedListStump<VarDecl>((yyvsp[0].node)); }
#line 2050 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 90: /* function_parameter_list: %empty  */
#line 227 ".//parser.y"
                                                             { (yyval.node) = linkedListStump<VarDecl>(nullptr); }
#line 2056 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;

  case 91: /* function_defn: FN IDENTIFIER '(' function_parameter_list ')' function_return_type '{' stmt_list '}'  */
#line 230 ".//parser.y"
                                                                                                  { 
                     auto* head = ((yyvsp[-5].node))->asPtr<VarDecl>();
                     (yyval.node) = statementList(fn((yyvsp[-7].node), head, (yyvsp[-3].node), (yyvsp[-1].node)));
             }
#line 2065 "/home/tarek/code-projects/SHEBLANG/parser.cpp"
    break;


#line 2069 "/home/tarek/code-projects/SHEBLANG/parser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 235 ".//parser.y"


// De-allocates a node and all of its children (if any).
void freeNode(nodeType *p) {
    int i;

    if (!p) return;
    free (p);
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
