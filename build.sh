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
	gcc -g lex.yy.c y.tab.c $1 -o $2 		# compile/link
}

interpreter() {
	generate_code
	build cli.c shbl_interpreter 
}

graph() {
	generate_code
	build clg.c shbl_graph 
}

compiler() {
	generate_code
	build clc.c shbl_compiler 
}

case $1 in 
	# pattern) command ;;
	interpreter) interpreter ;;
	graph) graph ;;
	compiler) compiler ;;
	generate_code) generate_code ;;
	clean) clean;;
	*) help ;;
esac 


