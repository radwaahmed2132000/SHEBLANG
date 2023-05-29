set -xe

debug=$2

help() {
	# $0 is the script name
	echo "USAGE: $0 {(i)nterpreter|(c)ompiler|clean} [--debug]"
}

cmake_prep() {
	mkdir -p build
	cd build
	cmake ../ -DBISON_DEBUG:STRING="$debug" -DCMAKE_BUILD_TYPE=Debug
}

interpreter() {
	cmake_prep
	make -j$(nproc) SHEBLANG_interpreter 
    cd ../
    cp ./debug/SHEBLANG_interpreter ./
}

compiler() { 
	cmake_prep
	make -j$(nproc) SHEBLANG_compiler 
}

clean() {
	cd build
	make clean
}

case $1 in 
	# pattern) command ;;
	interpreter|i) interpreter ;;
	compiler|c) compiler ;;
	clean) clean ;;
	*) help ;;
esac 
