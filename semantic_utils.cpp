#include "semantic_utils.h"
#include "nodes.h"

std::string getReturnType(nodeType* returnStatement) {
    auto& retOpr = returnStatement->as<UnOp>();
    auto returnType = semantic_analysis(retOpr.operand);

    // At this point, the function should be semantically fine, so no need to
    // check for the failure case?
    return std::get<SuccessType>(returnType);
}

std::vector<nodeType*> getFnReturnStatements(nodeType* fnStatements) {
    // Don't recurse into child functions. This is for cases where you have a
    // function defined inside another. We don't need to check the return type
    // of those.
    if(fnStatements->is<functionNodeType>()) { return {}; }

    if(auto* statements = fnStatements->asPtr<StatementList>(); statements) {
        std::vector<nodeType*> ret;

        for (auto *statement : statements->statements) {
            auto *uop = statement->asPtr<UnOp>();
            if (uop != nullptr && uop->op == UnOper::Return) {
                return {statement};
            }

            auto statementRet = getFnReturnStatements(statement);
            ret.insert(ret.end(), statementRet.begin(), statementRet.end());
        }

        return ret;
    } else if(auto* ifNode = fnStatements->asPtr<IfNode>(); (ifNode != nullptr)) {
        auto ifBranchRets = getFnReturnStatements(ifNode->ifCode);
        if(ifNode->elseCode != nullptr) {
            auto elseBranchRets = getFnReturnStatements(ifNode->elseCode);
            ifBranchRets.insert(ifBranchRets.end(), elseBranchRets.begin(), elseBranchRets.end());
        }
        return ifBranchRets;
    } else if(auto* w = fnStatements->asPtr<whileNodeType>(); (w!=nullptr)) {
        return getFnReturnStatements(w->loop_body);
    } else if(auto* dw = fnStatements->asPtr<doWhileNodeType>(); (dw!=nullptr)) {
        return getFnReturnStatements(dw->loop_body);
    } else if(auto* f = fnStatements->asPtr<forNodeType>(); (f!=nullptr)) {
        return getFnReturnStatements(f->loop_body);
    } else if(auto* sw = fnStatements->asPtr<switchNodeType>(); (sw!=nullptr)) {
        std::vector<nodeType*> caseReturns;
        auto caseList = sw->caseListTail->as<caseNodeType>().toVec();
        for(const auto& cs: caseList) {
            auto csReturn = getFnReturnStatements(cs->caseBody);
            caseReturns.insert(caseReturns.end(), csReturn.begin(), csReturn.end());
        }

        return caseReturns;
    }

    return {};
}