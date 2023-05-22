#include <variant>

template<typename ...NodeTypes>
typedef struct nodeType {
    std::variant<NodeTypes...> un;
	nodeType(std::variant<NodeTypes...> inner_union) : un(inner_union) {}
} nodeType;

