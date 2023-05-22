debug=$2

help() {
	echo "USAGE: $0 {interpreter|graph|compiler|generate_code} [--debug]"
}

clean() {
	rm y.tab.c y.tab.h lex.yy.c shbl_interpreter shbl_compiler shbl_graph
}

generate_code() {
	set -xe
	bison -y -d $debug cl.y            # create y.tab.h, y.tab.c
	lex cl.l                           # create lex.yy.c
}

build() {
	clang++-16 -ggdb -stdlib=libc++ -std=c++17 value_operators.cpp node_constructors.cpp cls.cpp lex.yy.c y.tab.c $1 -o $2 -Wno-write-strings 		# compile/link
}

interpreter() {
	generate_code
	build cli.cpp shbl_interpreter 
}

graph() {
	generate_code
	build clg.c shbl_graph 
}

compiler() {
	generate_code
	build clc.cpp shbl_compiler 
}

case $1 in 
	# pattern) command ;;
	interpreter|i) interpreter ;;
	compiler|c) compiler ;;
	graph) graph ;;
	generate_code) generate_code ;;
	clean) clean;;
	*) help ;;
esac 


