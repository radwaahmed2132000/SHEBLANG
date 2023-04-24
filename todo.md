Design a suitable programming language; you may use an existing one. The important constructs to be considered are:
1. Variables and Constants declaration:
	* Variable/Constant Names: [a-zA-Z_][a-zA-Z0-9_]* (1-225 char)
    1. Variable Types:
		* bool
		* char
		* int ✅
		* float
		* double
		* void
		* string
	2. Constant:
		* Using const keyword.
		* Using #define preprocessor.
2. Mathematical and logical expressions:
    1. Arithmetic Operators:
		* \+ ✅
		* \- ✅
		* \* ✅
		* / ✅
		* % ✅
		* ++ & --
			* Prefix: ❌ Buggy
			* Postfix
    2. Relational Operators: ✔
		* == ✅
		* != ✅
		* \> ✅
		* < ✅
		* \>= ✅
		* <= ✅
    3. Logical Operators: ✔
		* && ✅
		* || ✅
		* ! ✅
    4. Bitwise Operators: ✔
		* & ✅
		* | ✅
		* ^ ✅
		* ~ ✅
		* << ✅
		* \>> ✅
	5. Other: ✔
		* Unary + ✅
		* Unary - ✅
3. Assignment statement: ✔
	* = ✅
	* += ✅
	* -= ✅
	* *= ✅
	* /= ✅
	* %= ✅
	* <<= ✅
	* \>>= ✅
	* &= ✅
	* ^= ✅
	* |= ✅
4. Logic Blocks:
	1. If-then-else statement
	2. While loops
	3. Repeat-until loops [i.e. do-while loops]
	4. For loops
	5. Switch statement
5. Block structure (nested scopes where variables may be
declared at the beginning of blocks):
	* Handle { }
	* Handle Function Scope
6. Enums:
	* Syntax: enum enumerated-type-name{value1=1, value2, value3};
	* ex: enum suit{heart, diamond=8, spade=3, club}; ==> 0,8,3,4
7. Functions:
	* Function Declaration (optional)
	* Return Type
	* Function Name
	* Parameters (+ their data types and default values if available) [Assume all are by value]
	* Function Body
	* Function call
### Helpful Sources/References:
* https://www.tutorialspoint.com/cplusplus/cpp_operators.htm
* https://en.cppreference.com/w/cpp/language/operator_precedence