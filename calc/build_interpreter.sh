set -xe

bison -d calc.y									  # create y.tab.h, y.tab.c
lex calc.l                        # create lex.yy.c
gcc lex.yy.c calc.tab.c calc_interpreter.c -o calc_interperter     # compile/link
