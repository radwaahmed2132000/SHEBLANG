# Compiler-Project

## Prerequisites:
- Have Flex/Lex and Yacc/Bison installed.
- Have GCC installed.

### How to run:
1. Syntax Analysis (Parsing): Run in the command prompt in the project directory the following command. It will compile the yacc file to generate the y.tab.c and y.tab.c files.
```
bison -y -d cl.y
```
2. Lexical Analysis (Scanning): Next, run the following command to compile the lex file to generate the lex.yy.c file.
```
flex cl.l
```
3. Last but not least, compile all using gcc to generate the .exe file. We have 3 modes of operation:
* Interpreter mode: treats the code as an interpreted language and prints its results inline.
```
gcc cli.c y.tab.c lex.yy.c
```
* Compiler mode: compiles the code into assembly-like language.
```
gcc clc.c y.tab.c lex.yy.c
```
* Graph mode: draws a graph of the parse tree of the expressions.
```
gcc clg.c y.tab.c lex.yy.c
```
4. Run the a.exe file and try the input you like. Note that you may need to turn off your antivirus to be able to run the .exe file.