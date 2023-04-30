Design a suitable programming language; you may use an existing one. The important constructs to be considered are:
1. Variables and Constants declaration:
	* Variable/Constant Names: [a-zA-Z_][a-zA-Z0-9_]* (1-225 char)
    1. Variable Types:
		* int ✅
		* float
		* bool
		* char
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
	1. [x] If-then-else statement
	2. [x] While loops
	3. [x] Repeat-until loops [i.e. do-while loops]
	4. [x] For loops
        - Limitation: can't do for(;;) {...}, for(; expr; expr) {...}, etc..., mut always be for(expr; expr; expr) {...}
        - Can omit the braces if there's only one statement.
	5. [x] Switch statement
5. Block structure (nested scopes where variables may be
declared at the beginning of blocks):
	* Handle { }
	* Handle Function Scope
6. Enums:
	* Syntax: enum enumerated-type-name{value1=1, value2, value3};
	* ex: enum suit{heart, diamond=8, spade=3, club}; ==> 0,8,3,4
    * Currently implemented as `enum IDENTIFIER { val1, val2, ...};` won't probably allow assigning values to different members for simplicity.
7. Functions:
	* Function Declaration (optional)
	* Return Type
	* Function Name
	* Parameters (+ their data types and default values if available) [Assume all are by value
	* Function Body
	* Function call
### Helpful Sources/References:
* https://www.tutorialspoint.com/cplusplus/cpp_operators.htm
* https://en.cppreference.com/w/cpp/language/operator_precedence
