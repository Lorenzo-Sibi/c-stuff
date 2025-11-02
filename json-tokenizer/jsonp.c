#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "jsont.h"

typedef enum {
    JND_ROOT_OBJ, JND_OBJ,
    JND_PROPERTY, 
    JND_ARRAY,
    JND_VALUE_STRING,
    JND_VALUE_NUMBER,
    JND_VALUE_BOOL,
    JND_VALUE_NULL,
} JNodeType;

typedef struct {
    char* value_s;  // TODO: change this and value_n with something more solid (maybe union??)
    double value_n;
    struct JNode *first_child;
    struct JNode *next_sibling;
    JNodeType type;
} JNode;

typedef struct {
    JToken **tokens;
    JToken *p, *end;
    size_t size;
} JParser;



static inline bool jp_at_end(JParser *jp) { return jp->p >= jp->end; }
static inline JToken *jp_peek(JParser *jp) {return jp_at_end(jp) ? NULL : jp->p; }  // Returns the current char pointed by the lexer, '\0' otherwise

static inline JToken* jp_adv(JParser *jp) {
    if (jp_at_end(jp)) {
        assert(jp->end->type == JTK_EOF && "The token list should end with an EOF token (inside jp_adv)");
        return jp->end;
    }
    JToken *t = jp->p++;
    return t;
}


JNode* jp_next_child(JNode *father) {
    
}

JNode* jp_next_sibling(JNode *father) {
    
}

void jp_root_init(JNode *root) { 
    if (!root) return; 
    root->type = JND_ROOT_OBJ;
    root->next_sibling = root->first_child = NULL;
}

void jp_node_init(JNode *n, JNodeType type) {
    if (!n) return;
    if (type == JND_ROOT_OBJ) jp_root_init(n);
    n->first_child = n->next_sibling = NULL;
    n->type = type;
}

void jp_init_parser(JParser *jp, JToken **tokens, size_t size) {
    jp->tokens = tokens;
    jp->p = tokens[0];
    jp->size = size;
    jp->end = tokens[size - 1];
    assert(jp->end->type == JTK_EOF && "The token list should end with an EOF token");
}

void jp_parse_value(JParser *jp) {
    return;
}

void jp_parse_array(JParser *jp) {
    return;
}

// TODO: find a way to handle the errors which should be compatible with the error handling in jp_parse
void jp_parse_obj(JParser *jp, JNode *obj_n) {
    obj_n->type = obj_n->type == JND_ROOT_OBJ ? JND_ROOT_OBJ : JND_OBJ; // right??
    JToken *tok = jp_adv(jp);

    // trivial case "{}"
    if (tok->type == JTK_RBRACE) {
        obj_n->first_child = NULL;

    }

    while (1) {
        JToken *key = jp_adv(jp);
        if (key->type == JTK_STRING) tok = jp_adv(jp);
        else return; // Error "Expected string key, got: ..."
        if (tok != JTK_COLON) return; // Error "Expected colon key, got: ..."

        JToken *value = jp_adv(jp);
        if(value->type == JTK_LBRACK) jp_parse_array(jp);
        else jp_parse_jp(value);

    }


}

// IDEA: since parsing in the usual way could consume the memory since nested objects and arrays consume the stack
// we can do the following:
//      1. Through a simple state machine verify beforehand if the token list leads to a valid JSON syntax;do not proceed if an error occurs. 
//         Should be O(t) where t is the number of tokens 
//      2. "chunking" of objects and array: we can construct the AST for the simplest ones (the ones not nested) and then backtraking to the ones
//         at the top. In this way, we build increasingly complex ASTs for object/array nodes, avoiding stack saturation in a single chain of calls.

// Return the root JNode JND_ROOT_OBJ of the parsed JSON string 
// TODO: modify the datastructure so to have a error code/msg and not just a NULL
JNode* jp_parse(JNode *root, JParser *jp) {
    if (jp_at_end(jp)) return NULL;
    
    JToken *tok = jp_adv(jp);
    if (tok->type != JTK_LBRACE || tok->type != JTK_LBRACK) return NULL;

    // Let's implement only the case with Bracets "{ ... }"
    jp_parse_obj(jp, root);

}