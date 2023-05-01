#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <variant>
#include <algorithm>

#include "cl.h"
#include "y.tab.h"

std::string ex(nodeType* p);

int evaluate_switch(switchNodeType& sw) {
    int matching_case_index = -1;
    int default_case_index = -1;
    int var_value = std::stoi(ex(sw.var));

    assert(sw.case_list_head->type == typeCase);
    std::vector<caseNodeType> cases;

    nodeType* head = sw.case_list_head;

    // Since we have each switch case pointing at the one
    // before it, we need to collect this list and walk it
    // in reverse to have the switch case in the proper
    // order.
    do {
        cases.push_back(std::get<caseNodeType>(head->un));
        head = std::get<caseNodeType>(head->un).prev;
    } while(head != NULL);
    std::reverse(cases.begin(), cases.end());

    for(int i = 0; i < cases.size() && !sw.break_encountered; i++) {
        int case_value = std::stoi(ex(cases[i].self));
        auto opr = std::get<oprNodeType>(cases[i].self->un);

        if(opr.oper == DEFAULT) {
            default_case_index = i;
        } else if (case_value == var_value || (matching_case_index != -1)) {
            // Once you find a matching case, you execute
            // the following cases until you find a break
            // statement or you exhaust all the remaining
            // cases.

            matching_case_index = i;
            ex(opr.op[1]);
        }
    }

    if(matching_case_index == -1 && !sw.break_encountered) {
        auto opr = std::get<oprNodeType>(cases[default_case_index].self->un);
        ex(opr.op[0]);
    }

    return 0;
}

// TODO: Implement function logic.
std::string evaluate_function(functionNodeType& fn) {
    return std::to_string(0);
}

struct ex_visitor {
    std::string operator()(conNodeType& con) {
        switch(con.conType) {
            case intType: return std::to_string(con.iValue);
            case floatType: return std::to_string(con.fValue);
            case boolType: 
            {
                if(con.bValue)
                    return "true";
                return "false";
            }
            case charType: return std::string() + con.cValue;
            case stringType: return std::string(con.sValue);
            default:
                    return std::to_string(0);
        }
    }

    std::string operator()(idNodeType& identifier) {
        return sym2[identifier.id].second;
    }

    std::string operator()(caseNodeType& identifier) {
        printf("Case list nodes should never be evaluated alone. Please evaluate self and next.");
        exit(EXIT_FAILURE);
    }

    std::string operator()(switchNodeType& sw) {
        return std::to_string(evaluate_switch(sw));
    }

    std::string operator()(breakNodeType& br) {
        auto parent_switch = std::get<switchNodeType>(br.parent_switch->un);
        parent_switch.break_encountered = true;
        return std::to_string(0);
    }

    std::string operator()(functionNodeType& fn) {
        return evaluate_function(fn);
    }

    std::string operator()(oprNodeType& opr) {
        switch(opr.oper) {
            case WHILE:     while(std::stof(ex(opr.op[0]))) ex(opr.op[1]); return std::to_string(0);
            case DO:        do { ex(opr.op[1]); } while(std::stof(ex(opr.op[0]))); return std::to_string(0);
            case IF:        if (std::stof(ex(opr.op[0])))
                                ex(opr.op[1]);
                            else if (opr.op.size() > 2)
                                ex(opr.op[2]);
                            return std::to_string(0);
            case FOR:       
                            for(std::stof(ex(opr.op[0])); std::stof(ex(opr.op[1])); std::stof(ex(opr.op[2]))) {
                                ex(opr.op[3]);
                            }
                            return std::to_string(0);

            case DEFAULT:   break;
            case CASE:      return ex(opr.op[0]);

            case PRINT:  
            {
                printf("%s\n", ex(opr.op[0]).c_str());
                return std::to_string(0);
            }
            case ';':       
                // Need to check bounds for cases where the semi
                if(!opr.op.empty()) ex(opr.op[0]); 
                if(opr.op.size() > 1)
                    return ex(opr.op[1]);
                return std::to_string(0);

            case '=':       return sym2[std::get<idNodeType>(opr.op[0]->un).id].second = ex(opr.op[1]);
            case PP:        return sym2[std::get<idNodeType>(opr.op[0]->un).id].second = std::to_string(std::stof(ex(opr.op[0])) + 1);
            case MM:        return sym2[std::get<idNodeType>(opr.op[0]->un).id].second = std::to_string(std::stof(ex(opr.op[0])) - 1);
            case UPLUS:     return ex(opr.op[0]);
            case UMINUS:    return std::to_string(-std::stof(ex(opr.op[0])));
            case '!':       return std::to_string(!std::stof(ex(opr.op[0])));
            case '~':       return std::to_string(~std::stoi(ex(opr.op[0])));
            case '&':       return std::to_string(std::stoi(ex(opr.op[0])) & std::stoi(ex(opr.op[1])));
            case '|':       return std::to_string(std::stoi(ex(opr.op[0])) | std::stoi(ex(opr.op[1])));
            case '^':       return std::to_string(std::stoi(ex(opr.op[0])) ^ std::stoi(ex(opr.op[1])));
            case LS:        return std::to_string(std::stoi(ex(opr.op[0])) << std::stoi(ex(opr.op[1])));
            case RS:        return std::to_string(std::stoi(ex(opr.op[0])) >> std::stoi(ex(opr.op[1])));
            case '+':       return std::to_string(std::stof(ex(opr.op[0])) + std::stof(ex(opr.op[1])));
            case '-':       return std::to_string(std::stof(ex(opr.op[0])) - std::stof(ex(opr.op[1])));
            case '*':       return std::to_string(std::stof(ex(opr.op[0])) * std::stof(ex(opr.op[1])));
            case '/':       return std::to_string(std::stof(ex(opr.op[0])) / std::stof(ex(opr.op[1])));
            case '%':       return std::to_string(std::stoi(ex(opr.op[0])) % std::stoi(ex(opr.op[1])));
            case AND:       return std::to_string(std::stoi(ex(opr.op[0])) && std::stoi(ex(opr.op[1])));
            case OR:        return std::to_string(std::stoi(ex(opr.op[0])) || std::stoi(ex(opr.op[1])));
            case '<':       return std::to_string(std::stof(ex(opr.op[0])) < std::stof(ex(opr.op[1])));
            case '>':       return std::to_string(std::stof(ex(opr.op[0])) > std::stof(ex(opr.op[1])));
            case GE:        return std::to_string(std::stof(ex(opr.op[0])) >= std::stof(ex(opr.op[1])));
            case LE:        return std::to_string(std::stof(ex(opr.op[0])) <= std::stof(ex(opr.op[1])));
            case NE:        return std::to_string(std::stof(ex(opr.op[0])) != std::stof(ex(opr.op[1])));
            case EQ:        return std::to_string(std::stof(ex(opr.op[0])) == std::stof(ex(opr.op[1])));
        }

        return std::to_string(0);
    }

    // the default case:
    template<typename T> std::string operator()(T const &) const { return std::to_string(0); } 

};

std::string ex(nodeType *p) {
    if (p == nullptr) return std::to_string(0);
    return std::visit(ex_visitor(), p->un);
}
