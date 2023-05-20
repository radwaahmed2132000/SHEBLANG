#include <variant>
#include <algorithm>
#include <iostream>

#include "cl.h"
#include "y.tab.h"

struct semantic_analysis_visitor {
    Value operator()(conNodeType& con) { return Value(0); }

    Value operator()(idNodeType& identifier) { return Value(0); }

    Value operator()(caseNodeType& identifier) { return Value(0); }

    Value operator()(switchNodeType& sw) { return Value(0); }

    Value operator()(breakNodeType& br) { return Value(0); }

    Value operator()(functionNodeType& fn) { return Value(0); }

    Value operator()(doWhileNodeType& dw) { return Value(0); }

    Value operator()(whileNodeType& w) { return Value(0); }

    Value operator()(forNodeType& f) { return Value(0); }

    Value operator()(oprNodeType& opr) { return Value(0); }

    // the default case:
    template<typename T> 
    Value operator()(T const & /*UNUSED*/ ) const { return Value(0); } 
};

Value semantic_analysis(nodeType *p) {    
    // std::cout << "Semantic analysis running\n";
    if (p == nullptr) return Value(0);
    return std::visit(semantic_analysis_visitor(), p->un);
}


