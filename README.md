# SHEBLANG

<div><img src="SHEBLANG.png" style="margin:auto"></div>

SHEBLANG is a prototype interpretered language built for CMP23's fourth year compiler project. It can be interpretered and compiled to instructions for a stack machine. Its syntax is similar to C but it has some niceties like being able to define functions/enums inside of inner scopes. `testcases` contains our example/testcases. 

## Examples

### Fibbonaci 
```
fn fib(int n) int {
    if(n == 1 || n == 0) return 1;

    return fib(n-2) + fib(n-1);
}

print fib(20); #! should print 10946
```

### Everything in action
```
enum EnumTest { value1, value2, value3 };
const float pi = 3.14;

fn compute_area(float radius) float {
	float r_squared = radius * radius;
	return pi * r_squared;
}

#! Comments are written like this
fn main() {
	EnumTest e1 = EnumTest::value1;

	int	a  = 1;
	float c = 1.0;
	int _underscored = 0;

	float area_c = compute_area(c);

	float d = a * c;

	int accumulator = 0;
	for(int i = 0; i < 10; i += 1) {
		if (accumulator % 2 == 0) {
			accumulator *= 7;
		} else {
			accumulator += i;
		}
	}

	while(accumulator > 0) {
		accumulator -= i;
	}

	do {
		accumulator += 1;
	} while(accumulator < 10);

	bool flag = 0;
	switch(accumulator) {
		case 10: {
			flag = 1;
			accumulator = 0;
		}
		default: {
			accumulator = 10;
		}
	}

    print accumulator;
    print flag;
    print "area_c = ";
    print area_c;
}

#! Main is just a function like any other, you must call it 
main();
```

## Prerequisites:
- Have Flex/Lex and Yacc/Bison installed.
- Have Clang installed with support for C++20 (tested with clang 16).
  -   If you don't have clang installed, you can run `install_clang.sh`
  
## Project Architecture:
- Lexical Analysis which identifies tokens for our language.
- Syntax Analysis which is used to check if the written code for our language has correct syntax.
- Parsing Analysis.
- Semantic Analysis which is used to check if the written code for our language is semantically correct:
    - Undeclared variables, length of parameters of functions when calling, assigning variables with their defined type.
- Interpreter for executing the code.
- Compiler for generating machine code language.

### How to run:
- Run `./build.sh` with the arguments `interpreter` or `compiler` (or `i`/`c`) for short.
- You should get a binary called `shbl_interpreter` or `shbl_compiler` if everything goes well.

- You can then run `test.py` with with the arguments `interpreter` or `compiler` to test either.
- You can also choose to run only the test cases in a certain file by running `./test.py interpreter symantic` for example.
- There are four files to choose from: `logical`, `semantic`, `scopes` and `syntax`. 
- You can also run `./shbl_interpreter < path/to/testcase.shbl` to run a specific testcase.
- Similarly, you can run `./shbl_compiler < path/to/testcase.shbl`.

## How to debug with VSCode
- Download the [CodeLLDB extension](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)
- Build the interpreter or compiler using the instructions from the previous 
- Modify `launch.json` to fit your use case. You'll mostly change `stdio`, which controls the files stdin, stdout, and stderr write to respectively. null means that the program will write to the default file/stream (usually the terminal).

## Collaborators
<!-- readme: collaborators -start -->
<table>
<tr>
    <td align="center">
        <a href="https://github.com/Hero2323">
            <img src="https://avatars.githubusercontent.com/u/58619697?v=4" width="100;" alt="Abdelrahman Jamal"/>
            <br />
            <sub><b>Abdelrahman Jamal</b></sub>
        </a>
    </td>
    <td align="center">
        <a href="https://github.com/Iten-No-404">
            <img src="https://avatars.githubusercontent.com/u/56697800?v=4" width="100;" alt="Iten-No-404"/>
            <br />
            <sub><b>Iten Elhak</b></sub>
        </a>
    </td>
    <td align="center">
        <a href="https://github.com/radwaahmed2132000">
            <img src="https://avatars.githubusercontent.com/u/56734728?v=4" width="100;" alt="radwaahmed2132000"/>
            <br />
            <sub><b>Radwa Ahmed</b></sub>
        </a>
    </td>
    <td align="center">
        <a href="https://github.com/KnockerPulsar">
            <img src="https://avatars.githubusercontent.com/u/12754772?v=4" width="100;" alt="Tarek Yasser"/>
            <br />
            <sub><b>Tarek Yasser</b></sub>
        </a>
    </td></tr>
</table>
<!-- readme: collaborators -end -->
