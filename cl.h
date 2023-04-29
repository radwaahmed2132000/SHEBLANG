#include <vector>
#include <variant>
#include <string>
#include <unordered_map>

typedef enum { typeCon, typeId, typeOpr, typeSwitch, typeCase, typeBreak } nodeEnum;
typedef enum { intType, floatType, boolType, charType, stringType } typeEnum;

/* constants */
typedef struct {
    int iValue;                 /* integer value of constant */
    float fValue;               /* float value of constant */
    // int bValue;                 /* boolean value of constant */
    bool bValue;                /* boolean value of constant */
    char cValue;                /* char value of constant */
    std::string sValue;         /* string value of constant */
    typeEnum conType;           /* type of constant */
} conNodeType;

/* identifiers */
typedef struct {
    std::string id = "";        /* key/index to sym array */
    typeEnum idType;            /* type of variable */
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    std::vector<struct nodeTypeTag*> op;
// int nops;                   /* number of operands */
//  struct nodeTypeTag *op[1];	/* operands, extended at runtime */
} oprNodeType;

typedef struct {
    struct nodeTypeTag* self, *prev;
} caseNodeType;

typedef struct {
    int exit_label;
    int break_encountered;
    struct nodeTypeTag* var, *case_list_head;
} switchNodeType;

typedef struct {
    struct nodeTypeTag* parent_switch;
} breakNodeType;

#define NODE_TYPES conNodeType, idNodeType, oprNodeType, switchNodeType, caseNodeType, breakNodeType

typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node. Still need it since std::visit seems to not play well with recursive functions */
    std::variant<NODE_TYPES> un;

    // union {
    //     conNodeType con;        /* constants */
    //     idNodeType id;          /* identifiers */
    //     oprNodeType opr;        /* operators */
    //     switchNodeType sw;      /* switch cases */
    //     caseNodeType cs;        /* cases inside switch cases */
    //     breakNodeType br;       /* break statements in switch cases */
    // };
} nodeType;

extern float sym[26];
extern std::unordered_map<std::string, float> sym2;

#define MAX_SWITCH_CASES 16

