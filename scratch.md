Need to support the following:
- Variables and Constants declaration.
- Mathematical and logical expressions.
- Assignment statement.
- If-then-else statement, while loops, repeat-until loops, for loops, switch statement.
- Block structure (nested scopes where variables may be declared at the beginning of blocks).
- Enums
- Functions

# Tokens:
- Integers: [0-9]+
- Identifiers: [_a-zA-Z][_0-9a-zA-Z]*

# Files
## cl.y
Seems to define a bunch of function prototypes for constructing different types of tree nodes:
	- Operators (>, +, =, etc...)
	- Identifiers (a-z only)
	- Constants / literals (integers only)
Defines the symbol table (a-z entries only).
Defines a union of {integer, symbol table index, nodePtr}. This should be the return from the lexer?
Defines the grammar for a C like toy language.

All prototypes are defined in the same file except `ex` and `yylex`
	- `ex` is defined in `clc.c`
		It seems to be a recursive function that takes a root node and recursively emit assembly like instructions.

# Other notes:
- [Manual page 8] Function yywrap is called by lex when input is exhausted. Return 1 if you are done, or 0 if more processing is required.
- [Manual page 9] Includes variables and functions predefined by lex.
