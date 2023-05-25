#include "semantic_utils.h"

std::string getReturnType(nodeType* returnStatement) {
    auto& returnOpr = returnStatement->as<oprNodeType>();
    auto returnType = semantic_analysis(returnOpr.op[0]);

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

        for (const auto *statement : statements->toVec()) {
            auto *opr = statement->statementCode->asPtr<oprNodeType>();
            if (opr != nullptr && opr->oper == RETURN) {
                return {statement->statementCode};
            }

            auto statementRet = getFnReturnStatements(statement->statementCode);
            ret.insert(ret.end(), statementRet.begin(), statementRet.end());
        }

        return ret;
    } else if(auto* opr = fnStatements->asPtr<oprNodeType>(); (opr != nullptr) && opr->oper == IF) {
        auto ifBranchRets = getFnReturnStatements(opr->op[1]);
        if(opr->op.size() == 3) {
            auto elseBranchRets = getFnReturnStatements(opr->op[2]);
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