#include <vector>
#include <variant>
#include <string>
#include <unordered_map>

typedef enum { typeCon, typeId, typeOpr, typeSwitch, typeCase, typeBreak } nodeEnum;

/* constants */
typedef struct {
    int value;                  /* value of constant */
} conNodeType;

/* identifiers */
typedef struct {
    std::string id;
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

#define NODE_TYPES conNodeType, idNodeType, oprNodeType, switchNodeType, caseNodeType, breakNodeType

typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node. Still need it since std::visit seems to not play well with recursive functions */
    std::variant<NODE_TYPES> un;
} nodeType;

extern int sym[26];
extern std::unordered_map<std::string, int> sym2;
