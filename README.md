# Compiler-Project

## Prerequisites:
- Have Flex/Lex and Yacc/Bison installed.
- Have Clang installed with support for C++20 (tested with clang 16).
  -   If you don't have clang installed, you can run `install_clang.sh`

### How to run:
- Run `./build.sh` with the arguments `interpreter` or `compiler` (or `i`/`c`) for short.
- You should get a binary called `shbl_interpreter` or `shbl_compiler` if everything goes well.

- You can then run `test.py` with with the arguments `interpreter` or `compiler` to test either.
- You can also choose to run only the test cases in a certain file by running `test.py interpreter symantic` for example.
- There are four files to choose from: `logical`, `semantic`, `scopes` and `syntax`. 
- You can also run `shb_interpreter < path/to/testcase.shbl` to run a specific testcase.

## How to debug with VSCode
- Download the [CodeLLDB extension](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)
- Build the interpreter or compiler using the instructions from the previous 
- Modify `launch.json` to fit your use case. You'll mostly change `stdio`, which controls the files stdin, stdout, and stderr write to respectively. null means that the program will write to the default file/stream (usually the terminal).