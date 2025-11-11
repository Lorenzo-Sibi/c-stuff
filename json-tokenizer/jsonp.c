#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "jsont.h"


#define MAX_TOKENS 1024 

typedef enum {
    JND_ROOT_OBJ, JND_OBJ,
    JND_PROPERTY, 
    JND_ARRAY,
    JND_STRING,
    JND_NUMBER,
    JND_BOOL,
    JND_NULL,
    JND_ERROR,
} JNodeType;

typedef struct JNode {
    union {
        bool bvalue;
        double nvalue;
        const char *svalue;
        const char *err_msg;
    } value;

    struct JNode *first_child; // JDN_PROPERTY can ONLY have 2 children (one for the key and one for the value) while for JND_STRING, JND_NUMBER, JND_BOOL, and JND_NULL should be NULL
    struct JNode *next_sibling;
    JNodeType type;
    size_t child_size;      // For JND_STRING, JND_NUMBER, JND_BOOL, and JND_NULL must be 0 while for JND_PROPERTY must be 2
} JNode;

typedef struct {
    JToken **tokens;
    JToken *p, *end;
    size_t size;
} JParser;


JNode* jp_error(JNode *node, char* err_msg);

static inline bool jp_at_end(JParser *jp) { return !jp->p || jp->p >= jp->end; }
static inline JToken *jp_peek(JParser *jp) {return jp_at_end(jp) ? jp->end : jp->p; }  // Returns the current char pointed by the lexer, '\0' otherwise
static inline JToken* jp_adv(JParser *jp) {
    if (jp_at_end(jp)) {
        assert(jp->end->type == JTK_EOF && "The token list should end with an EOF token (inside jp_adv)");
        return jp->end;
    }
    JToken *t = jp->p++;
    return t;
}

void jp_add_child(JNode *father, JNode *child) {
    if(!father || !child) return;
    child->next_sibling = NULL; // reset next_sibling anyway to NULL
    JNode *tmp = (JNode*)father->first_child;
    if (!tmp) {
        assert(father->child_size == 0 && "Bad father node initialization. Node's first_child is NULL but child_size not zero");
        father->first_child = child;
        father->child_size++;
        return;
    }
    while(tmp->next_sibling)
        tmp = tmp->next_sibling;
    tmp->next_sibling = child;
    father->child_size++;
}

void jp_node_init(JNode *n, JNodeType type) {
    if (!n) return;
    n->first_child = n->next_sibling = NULL;
    n->type = type;
    n->child_size = 0;
}

void jp_init_parser(JParser *jp, JToken **tokens, size_t size) {
    assert(size > 0 && "Parser initialized with size <= 0");
    int i = 0;
    while (tokens[i]) i++;
    assert(size == i && "Size argument mismatch");
    assert(tokens[size - 1]->type == JTK_EOF && "The JSON token list should end with an EOF token");
    jp->tokens = tokens;
    jp->p = tokens[0]; jp->end = tokens[size - 1]; jp->size = size;
}

static const char* nname(JNodeType k){
    switch(k){
        case JND_ROOT_OBJ: return "ROOT"; case JND_OBJ : return "OBJ"; case JND_ARRAY: return "ARRAY"; case JND_PROPERTY: return "PROPERTY";
        case JND_STRING: return "STRING"; case JND_NUMBER: return "NUMBER"; case JND_ERROR: return "ERROR";
        case JND_BOOL: return "BOOL"; case JND_NULL: return "NULL";
    } return "?";
}

void jp_free_ast(JNode *root) {
    if (!root) return;
    JNode *tmp = root;
    while (tmp) {
        tmp = tmp->first_child;
        jp_free_ast(tmp);
        tmp = tmp ? tmp->next_sibling : NULL;
        jp_free_ast(tmp);
    }
    printf("[DEBUG] Freeig %s node.\n", nname(root->type));
    free(root);
}

// TODO: change it to another name I don't like the value since here we're parsing STRINGS, NUMBERS and PRIMITIVES (BOOL/NULL)
JNode* jp_parse_value(JToken *tok) {
    JNode *value = (JNode*)malloc(sizeof(JNode));
    switch (tok->type) {
    case JTK_STRING:
        jp_node_init(value, JND_STRING);
        value->value.svalue = jl_string_to_utf8(tok, NULL);
        break;
    case JTK_NUMBER:
        jp_node_init(value, JND_NUMBER);
        jl_number_to_double(tok, &(value->value.nvalue));
        break;
    case JTK_FALSE:
        jp_node_init(value, JND_BOOL);
        value->value.bvalue = 0;
        break;
    case JTK_TRUE:
        jp_node_init(value, JND_BOOL);
        value->value.bvalue = 1;
        break;
    case JTK_NULL:
        jp_node_init(value, JND_NULL);
        value->value.bvalue = -1;
        break;
    default:
        value = jp_error(value, "Error during value parsing. Not a valid value.");
        break;
    }
    return value;
}

JNode* jp_parse_array(JParser *jp) {
    return NULL;
}

JNode* jp_error(JNode *node, char* err_msg) {
    assert(node && "error node cannot be NULL");
    node->type = JND_ERROR;
    node->value.err_msg = err_msg;
    return node;
}

void jp_print_ast(JNode* root, int tabs) {
    JNode *tmp = root;
    while(tmp) {
        for(int i=0; i < tabs; i++)
            printf("    ");
        printf("\u2514\u2500");
        printf(" %s \n", nname(tmp->type));
        JNode* child = tmp->first_child;
        while(child) { // print children
            jp_print_ast(child, tabs + 1);
            child = child->next_sibling;
        }
        tmp = tmp->next_sibling;
        jp_print_ast(tmp, tabs);
    }
}

// TODO: find a way to handle the errors which should be compatible with the error handling in jp_parse
JNode* jp_parse_obj(JParser *jp) {
    JNode *obj = (JNode*)malloc(sizeof(JNode));
    if (!obj) return NULL;

    jp_node_init(obj, JND_OBJ);

    JToken *tok = jp_adv(jp);
    if (tok->type != JTK_LBRACE) return jp_error(obj, "Expected {");

    tok = jp_peek(jp);
    // trivial case "{}"
    if (tok->type == JTK_RBRACE) {
        tok = jp_adv(jp);
        if (tok->type == JTK_EOF) return obj;
        else return jp_error(obj, "\"{}\" JSON not terminating with EOF");
    }

    while (1) { 
        JToken *tkey = jp_adv(jp);

        if (tkey->type == JTK_RBRACE) {
            tok = jp_adv(jp);
            if (tok->type == JTK_EOF) return obj;
            else return jp_error(obj, "\"{}\" JSON not terminating with EOF");
        }

        if (tkey->type != JTK_STRING) return jp_error(obj, "Expected string key");

        JNode *key = jp_parse_value(tkey);
        if (!key) return jp_error(obj, "Failed to parse key");
        
        tok = jp_adv(jp);  
        if (tok->type != JTK_COLON) {
            jp_free_ast(key);
            return jp_error(obj, "Expected colon key, got: ..."); // Error
        }

        JToken *tvalue = jp_adv(jp);
        JNode *value = NULL;

        if(tvalue->type == JTK_LBRACK) value = jp_parse_array(jp);
        else if (tvalue->type == JTK_LBRACE) value = jp_parse_obj(jp);
        else value = jp_parse_value(tvalue); 

        if (!value) {
            jp_free_ast(key);
            return jp_error(obj, "Failed to parse value");
        }

        JNode *property = (JNode*)malloc(sizeof(JNode));
        if(!property) {
            jp_free_ast(key);
            jp_free_ast(value);
            return jp_error(obj, "Memory allocation failed for propert node");
        }

        jp_node_init(property, JND_PROPERTY);
        jp_add_child(property, key); 
        jp_add_child(property, value);
        assert(property->child_size == 2 && property->first_child->type == JND_STRING && "PropertyNode should have 2 children and a JDN_STRING as first_child");
    
        jp_add_child(obj, property);

        // ok here we exit for sure
        if (tok->type == JTK_RBRACE) break;
        if (tok->type != JTK_COMMA) return jp_error(obj, "Expected comma or closing brace");
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
JNode* jp_parse(JParser *jp, JNode *root) {
    if (jp_at_end(jp)) return jp_error(root, "Empty JSON file (EOF)");
    
    JToken *tok = jp_peek(jp);
    if (tok->type != JTK_LBRACE && tok->type != JTK_LBRACK) return jp_error(root, "Bad starting brace/bracket");

    // Let's implement only the case with Bracets "{ ... }"
    root = jp_parse_obj(jp);
    root->type = JND_ROOT_OBJ;
    return root;

}


char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory and read
    char *content = malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0';
    }
    
    fclose(file);
    return content;
}

int main(int argc, char *argv[]) {

    JParser jp;
    JToken tokens[MAX_TOKENS];
    char *json = NULL;
    /*
    if (argc > 2) {
        if(strcmp(argv[1], "-f") == 0) { // from file
        json = read_file(argv[2]);
    } else if (strcmp(argv[1], "-l") == 0) { // from line
    json = argv[2];
}
} else {
    fprintf(stderr, "Usage: %s [-f] [-l] [FILENAME] [JSON STRING]\n", argv[0]);
    exit(1);
}
*/

    json = read_file("tests/test.json");

    JLexer lx;
    jl_init(&lx, json, strlen(json));
    int i = 0;

    for(;;){
        tokens[i] = jl_next(&lx);
        JToken t = tokens[i];
        i++;
        printf("%zu:%zu %-6s  '%.*s'\n", t.line, t.column, kname(t.type), (int)t.length, t.start);
        if (t.type == JTK_STRING){
            const char *err=NULL; char *s = jl_string_to_utf8(&t, &err);
            if (s){ printf("       decoded: \"%s\"\n", s); free(s); }
            else   printf("       decode error: %s\n", err?err:"(unknown)");
        }
        if (t.type == JTK_NUMBER){ double v; if(jl_number_to_double(&t,&v)) printf("       number: %g\n", v); }
        if (t.type == JTK_ERROR){ fprintf(stderr, "ERROR: %s\n", t.err_msg ? t.err_msg : "token error"); break; }
        if (t.type == JTK_EOF) break;
    }

    JToken *token_ptrs[MAX_TOKENS] = { NULL };
    for (int j = 0; j < i; j++)
        token_ptrs[j] = &tokens[j];

    jp_init_parser(&jp, token_ptrs, i);

    JNode *root = (JNode*)malloc(sizeof(JNode));
    if(!root) {
        perror("Failed allocating root");
        exit(1);
    }

    root = jp_parse(&jp, root);

    JNode root1, key1, key2, value1, value2;
    jp_node_init(&root1, JND_ROOT_OBJ);
    jp_node_init(&key1, JND_STRING);
    jp_node_init(&key2, JND_STRING);
    jp_node_init(&value1, JND_STRING);
    jp_node_init(&value2, JND_STRING);

    root1.first_child = &key1; root1.child_size = 1;
    key1.first_child = &value1; key1.child_size = 1; key1.next_sibling = &key2;
    key2.first_child = &value2; key2.child_size = 1;

    jp_print_ast(root, 0);

    printf("Node type %s\n", nname(root->type));
    if (root->type == JND_ERROR)
        printf("Error message: %s\n", root->value.err_msg);
    return 0;

    jp_free_ast(root);
}