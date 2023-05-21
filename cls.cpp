#include <variant>
#include <algorithm>
#include <iostream>

#include "cl.h"
#include "y.tab.h"

#include "result.h"


struct semantic_analysis_visitor {
    Result operator()(conNodeType &con){ return Result::Success(""); }

    Result operator()(idNodeType &identifier) { return Result::Success("");; }

    Result operator()(caseNodeType& identifier) { return Result::Success("");; }

    Result operator()(switchNodeType& sw) { return Result::Success("");; }

    Result operator()(breakNodeType& br) { return Result::Success("");; }

    Result operator()(functionNodeType& fn) { return Result::Success("");; }

    Result operator()(doWhileNodeType& dw) { return Result::Success("");; }

    Result operator()(whileNodeType& w) { return Result::Success("");; }

    Result operator()(forNodeType& f) { return Result::Success("");; }

    Result operator()(oprNodeType& opr) { return Result::Success("");; }

    // the default case:
    template<typename T> 
    Result operator()(T const & /*UNUSED*/ ) const { return Result::Success("");; } 
};

Result semantic_analysis(nodeType *p) {    
    // std::cout << "Semantic analysis running\n";
    if (p == nullptr) return Result::Success("");;
    return std::visit(semantic_analysis_visitor(), p->un);
}


