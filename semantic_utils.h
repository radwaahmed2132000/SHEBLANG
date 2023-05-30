#include "cl.h"
#include "parser.h"

std::string getReturnType(Node* returnStatement);

// Recurses into a function, checking statements, if conditions, while loops,
// for loops, do while loops, and switch cases.
//
// fn foo(int a, int b) int {
//      if(a > 0) {
//          return 2.0 * a;
//      } else {
//          b *= 2;
//      }
//
//      for(int c = 0; c < b; c++) {
//          if( c - a  == 0) {
//              a += 1;
//          } else {
//              return b;
//          }
//      }
// }
//
std::vector<Node*> getFnReturnStatements(Node* fnStatements);