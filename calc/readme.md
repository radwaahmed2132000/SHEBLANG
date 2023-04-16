This directory includes a small example from the manual found in `docs/` with
some added comments to explain what's going on. The calculator currently
supports basic arithmetic expressions, variables, if conditions, and while
loops.

How to build:
Just run `build_interpreter.sh`.

How to use:
You can `cat script.sbhl | ./calc_interpreter`, or run the interpreter normally
and type commands by hand.

As for the flow of creating the parser/lexer. The manual usually introduces lex
code, then yacc code. But the more logical flow (in my opinion) is creating the
yacc file and defining the grammar and token types, followed by creating the
lex file where we match the patterns and their values and use the tokens.

Note that the previous might only apply to this simple example, complex
examples might require a differnt approach, Things that are missing for the
project:
- Variable names larger than one character. This is not explicitly required in
- the project document so it might npt be needed. Logical expressions ( &&, ||,
- ^, !, etc..) Should be similar to mathematical expressions. Repeat until
- loops (AKA do-while). Switch statements. Nested scopes (scope checking to be
- precise, nested scopes in while loops work) Enums Functions Symbol table
- format Error handling Semantic analyzer (contains a bunch of stuff) Variable
- declaration conflicts. i.e. multiple declarations of the same variable.
		If we decide to be dynamic (a la Python), this shouldn't matter.
	- Improper usage of variables regarding their type.
			Also affected by whether we're dynamic or not.
	- Variables used before being initialized and unused variables. The addition
	- of type conversion quadruples to match operators’ semantic requirements,
	- i.e. converting integer to real, … Warning in case of if statement whose
	- condition is always false.
			The simplest way is to check if the condition is a constant. A more
			thorough check can see if the variable used in the statement changes
			anywhere or not.

For Phase 1:
- Need to finalize the lexer (Agree on what our tokens will look like). Need to
- finalize our grammar (Of course, guided by the requirements mentioned above).
