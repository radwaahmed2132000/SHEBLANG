#include "node.h"

Node::Node(std::variant<NODE_TYPES> inner_union, int lineNo)
    : std::variant<NODE_TYPES>(inner_union), lineNo(lineNo) {}

Node::Node( std::variant<NODE_TYPES> inner_union, int lineNo, bool addNewScope)
    : std::variant<NODE_TYPES>(inner_union), lineNo(lineNo), addNewScope(addNewScope) {}
