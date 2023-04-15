if [ "$#" != 1 ]; then
	echo "USAGE: build <program name>"
	exit 1
fi

set -xe

bison -d $1.y									  # create y.tab.h, y.tab.c
lex $1.l                        # create lex.yy.c
gcc lex.yy.c $1.tab.c -o $1     # compile/link
