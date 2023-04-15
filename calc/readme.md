This directory includes a small example from the manual found in `docs/` with some added comments to explain what's going on. This calculator matches only simple addition and subtraction in the form `1 + 2` or `1 - 3`. It does not support negation (`-1` for example).

As for the flow of creating the parser/lexer. The manual introduced lex, then yacc. But the more logical flow (in my opinion) is creating the yacc file and defining the grammar and token types, followed by creating the lex file where we match the patterns and their values and use the tokens.

Note that the previous might only apply to this simple example, complex examples might require a differnt approach,
