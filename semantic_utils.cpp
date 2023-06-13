#include "semantic_utils.h"
#include "cl.h"
#include "nodes.h"

Type getReturnType(Node* returnStatement) {
    auto& retOpr = returnStatement->as<UnOp>();
    auto returnType = semantic_analysis(retOpr.operand);

    // At this point, the function should be semantically fine, so no need to
    // check for the failure case?
    return std::get<Type>(returnType);
}

std::vector<Node*> getFnReturnStatements(Node* fnStatements) {
    // Don't recurse into child functions. This is for cases where you have a
    // function defined inside another. We don't need to check the return type
    // of those.
    if(fnStatements->is<FunctionDefn>()) { return {}; }

    if(auto* statements = fnStatements->asPtr<StatementList>(); statements) {
        std::vector<Node*> ret;

        for (auto *statement : statements->statements) {
            auto *uop = statement->asPtr<UnOp>();
            if (uop != nullptr && uop->op == UnOper::Return) {
                return {statement};
            }

            Utils::extendVector(ret, getFnReturnStatements(statement));
        }

        return ret;
    } else if(auto* ifNode = fnStatements->asPtr<IfNode>(); (ifNode != nullptr)) {
        auto ifBranchRets = getFnReturnStatements(ifNode->ifCode);
        if(ifNode->elseCode != nullptr) {
            Utils::extendVector(ifBranchRets, getFnReturnStatements(ifNode->elseCode));
        }
        return ifBranchRets;
    } else if(auto* w = fnStatements->asPtr<whileNodeType>(); (w!=nullptr)) {
        return getFnReturnStatements(w->loop_body);
    } else if(auto* dw = fnStatements->asPtr<doWhileNodeType>(); (dw!=nullptr)) {
        return getFnReturnStatements(dw->loop_body);
    } else if(auto* f = fnStatements->asPtr<forNodeType>(); (f!=nullptr)) {
        return getFnReturnStatements(f->loop_body);
    } else if(auto* sw = fnStatements->asPtr<switchNodeType>(); (sw!=nullptr)) {
        std::vector<Node*> caseReturns;
        auto caseList = sw->caseListTail->as<caseNodeType>().toVec();
        for(const auto& cs: caseList) {
            Utils::extendVector(caseReturns, getFnReturnStatements(cs->caseBody));
        }

        return caseReturns;
    }

    return {};
}
struct ArrayLiteralVisitor {
    Node* p;

    int operator()(conNodeType& al) const {
        return 0;
    }

    int operator()(ArrayLiteral& al) const {
        int maxDim = 0;
        for(auto* expr: al.expressions) {
            int dim = getArrayDepth(expr);
            maxDim = std::max(dim, maxDim);
        }

        return 1 + maxDim;
    }

    int operator()(idNodeType& id) const {
        auto* scope = getSymbolScope(id.id, p->currentScope);
        if(scope != nullptr) {
            return scope->sym2[id.id].type.depth;
        }

        return 0;
    }

    template<typename Default> 
    int operator()(Default& def) const { return 0; }
};

int getArrayDepth(Node* expr) {
    return std::visit(ArrayLiteralVisitor{expr}, *expr);
}