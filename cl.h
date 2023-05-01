#include <vector>
#include <variant>
#include <string>
#include <unordered_map>

typedef enum { typeCon, typeId, typeOpr, typeSwitch, typeCase, typeBreak, typeFunction } nodeEnum;
typedef enum { intType, floatType, boolType, charType, stringType } conTypeEnum;

/* constants */
typedef struct {
    union {
        int iValue;                 /* integer value of constant */
        float fValue;               /* float value of constant */
        bool bValue;                /* boolean value of constant */
        char cValue;                /* char value of constant */
        // std::string sValue;         /* string value of constant */ */
    };
    conTypeEnum conType;           /* type of constant */
} conNodeType;

/* identifiers */
typedef struct {
    std::string id;             /* key/index to sym array */
    conTypeEnum idType;            /* type of variable */
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    std::vector<struct nodeTypeTag*> op;
} oprNodeType;

typedef struct {
    struct nodeTypeTag* self, *prev;
} caseNodeType;

typedef struct {
    int exit_label;
    bool break_encountered;
    struct nodeTypeTag* var, *case_list_head;
} switchNodeType;

typedef struct {
    struct nodeTypeTag* parent_switch;
} breakNodeType;

typedef struct {
    struct nodeTypeTag* return_type;
    struct nodeTypeTag* name;
    // TODO: some sort of list for parameters
    struct nodeTypeTag* statemenst;
} functionNodeType;

#define NODE_TYPES conNodeType, idNodeType, oprNodeType, switchNodeType, caseNodeType, breakNodeType, functionNodeType

typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node. Still need it since std::visit seems to not play well with recursive functions */
    std::variant<NODE_TYPES> un;
} nodeType;

extern float sym[26];
extern std::unordered_map<std::string, float> sym2;
