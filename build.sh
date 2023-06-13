set -xe

debug_build=$2

# set to "--debug" for bison debugging
debug_bison=""

help() {
	# $0 is the script name
	echo "USAGE: $0 {(i)nterpreter|(c)ompiler|clean} [--debug|--release]"
}

cmake_prep() {
	mkdir -p build
	cd build

	if [ "$debug_build" == "--release" ]; then
		echo "Creating a release build"
		cmake ../ -DBISON_DEBUG:STRING=$debug_bison -DCMAKE_BUILD_TYPE=RelWithDebInfo
	else
		echo "Creating a debug build"
		cmake ../ -DBISON_DEBUG:STRING=$debug_bison -DCMAKE_BUILD_TYPE=Debug
	fi
}

interpreter() {
	cmake_prep
	make -j$(nproc) SHEBLANG_interpreter 
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
