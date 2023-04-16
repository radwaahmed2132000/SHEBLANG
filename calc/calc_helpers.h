// Changed some names here to hopefully be easier to understand. See page 23 of the manual for the 
// original implementation.

typedef enum { TypeConstant, TypeIdentifier, TypeOperator } NodeEnum;

typedef struct {
    int value;
} ConstantNode;

typedef struct {
    int symbol_table_index;
} IdentifierNode;

typedef struct {
    int oper;                      // Operator
    int num_operands;
    struct ParserNode *operands[1]; // Expandable at runtime
} OperatorNode;

typedef struct ParserNode {
    NodeEnum type;

    // Union needs to be the last item in the struct since operatorNode might dynamically increase
    // in size at runtime.
    union {
        ConstantNode constant;
        IdentifierNode identifier;
        OperatorNode opr;
    };
} ParserNode;

extern int sym[26];
