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

typedef struct JsonNode {
    union {
        bool bvalue;
        double nvalue;
        const char *svalue;
        const char *err_msg;
    } value;
    struct JsonNode *first_child; // JDN_PROPERTY can ONLY have 2 children (one for the key and one for the value) while for JND_STRING, JND_NUMBER, JND_BOOL, and JND_NULL should be NULL
    struct JsonNode *next_sibling;
    char *key; // Only used if the node is child of an JND_OBJ
    JNodeType type;
    size_t child_size;      // For JND_STRING, JND_NUMBER, JND_BOOL, and JND_NULL must be 0 while for JND_PROPERTY must be 2
} JsonNode;

JsonNode error_node = { 
    .type = JND_ERROR, 
    .value.err_msg = "Error message not set", 
    .first_child = NULL, 
    .next_sibling = NULL, 
    .child_size = 0, 
    .key = NULL 
};

// Parsing APIs
JsonNode* jp_parse(const char *json_str, size_t len);
JsonNode* jp_parse_file(const char*filename);
JsonNode* jp_parse_obj(JLexer *lx);
JsonNode* jp_parse_array(JLexer*lx);
JsonNode* jp_parse_value(JLexer *lx);
JsonNode* jp_error(const char* err_msg);
void jp_error_set_msg(const char* err_msg);
bool jp_is_error(JsonNode *node);
void jp_free_ast(JsonNode *root);

// 
static JsonNode* jp_create_node(JNodeType type);
void jp_print_ast(JsonNode *root);

// Utility functions
char* read_file(const char *filename);

// Enhance the error handling.
//  DONE 1. the error node is already defined globally
//  DONE - 3. The functino should return a pointer to the error node
//  4. Fix the parsing function and point out in which part of the token list and json string the error occurred.
inline JsonNode* jp_error(const char* err_msg) { jp_error_set_msg(err_msg); return &error_node; }
inline void jp_error_set_msg(const char *err_msg) { err_msg = err_msg ? err_msg : "Error message not set"; char *copy = (char*)strcpy(err_msg, error_node.value.err_msg); }
inline bool jp_is_error(JsonNode *node) { return node && node->type == JND_ERROR; }


static JsonNode* jp_create_node(JNodeType type) {
    JsonNode *node = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!node) return NULL;
    node->type = type;
    return node;
}

static const char* nname(const JsonNode *node){
    if (!node) return "NULL";
    JNodeType k = node->type;
    switch(k){
        case JND_ROOT_OBJ: return "ROOT"; case JND_OBJ : return "OBJ"; case JND_ARRAY: return "ARRAY"; case JND_PROPERTY: return "PROPERTY";
        case JND_STRING: return "STRING"; case JND_NUMBER: return "NUMBER"; case JND_ERROR: return "ERROR";
        case JND_BOOL: return "BOOL"; case JND_NULL: return "NULL";
    } return "?";
}

static void jp_print_ast_rec(JsonNode *node, bool last_flags[], unsigned depth) {
    if (!node) return;
    for (unsigned i = 0; i < depth; i++)
        printf("%s", last_flags[i] ? "   " : "│  ");
    printf("%s", (const char *)(depth ? (last_flags[depth] ? "└─ " : "├─ ") : "└─ ")); // branch connector

    // node label + value
    printf("%s", nname(node));
    if (node->key) printf(" key=\"%s\"", node->key);
    switch (node->type) {
        case JND_STRING: printf(" : \"%s\"", node->value.svalue); break;
        case JND_NUMBER: printf(" : %g", node->value.nvalue); break;
        case JND_BOOL:   printf(" : %s", node->value.bvalue ? "true" : "false"); break;
        case JND_NULL:   printf(" : null"); break;
        case JND_ERROR:  printf(" : ERROR: %s", node->value.err_msg); break;
        default: break; // objects/arrays already described by type
    }
    printf("\n");

    // children
    JsonNode *child = node->first_child;
    while (child) {
        last_flags[depth + 1] = (child->next_sibling == NULL);
        jp_print_ast_rec(child, last_flags, depth + 1);
        child = child->next_sibling;
    }
}

void jp_print_ast(JsonNode *root) {
    bool last_flags[64] = {true}; // adjust depth if you expect deeper trees
    printf("\n\n=== JSON AST ===\n");
    jp_print_ast_rec(root, last_flags, 0);
}

void jp_free_ast(JsonNode *root) {
    if (!root) return;
    JsonNode *child = root->first_child;
    while (child) {
        JsonNode *next = child->next_sibling;
        jp_free_ast(child);
        child = next;
    }
    printf("[DEBUG] Freeing %s node.\n", nname(root));
    if (root->key) free(root->key);
    free(root);
}

// TODO: change it to another name I don't like the value since here we're parsing STRINGS, NUMBERS and PRIMITIVES (BOOL/NULL)
JsonNode* jp_parse_value(JLexer *lx) {
    // I know it's ugly but it's fuctional :) (maybe I' gonna change it later ... or maybe no!)
    jl_skip_ws(lx);
    if (jl_peek(lx) == '{')
        return jp_parse_obj(lx);
    else if (jl_peek(lx) == '[')
        return jp_parse_array(lx);

    JToken tok = jl_next(lx);

    switch (tok.type) { // primitive values
        case JTK_STRING: {
            JsonNode *n = jp_create_node(JND_STRING);
            n->value.svalue = jl_string_to_utf8(&tok, NULL);
            return n;
        }
        case JTK_NUMBER: {
            JsonNode *n = jp_create_node(JND_NUMBER);
            jl_number_to_double(&tok, &n->value.nvalue);
            return n;
        }
        case JTK_TRUE: {
            JsonNode *n = jp_create_node(JND_BOOL);
            n->value.bvalue = true;
            return n;
        }
        case JTK_FALSE: {
            JsonNode *n = jp_create_node(JND_BOOL);
            n->value.bvalue = false;
            return n;
        }
        case JTK_NULL: return jp_create_node(JND_NULL);
        default: return jp_error("Error invalue parsing."); // Error
    }
}

JsonNode* jp_parse_array(JLexer *lx) {
    JsonNode *array = jp_create_node(JND_ARRAY);
    JsonNode *tail = NULL;

    if (!array) return jp_error("oom (array)");
    JToken tok = jl_next(lx);
    if (tok.type != JTK_LBRACK) return jp_error("Error. Expected '[' while array parsing.");
    jl_skip_ws(lx);
    if (jl_peek(lx) == ']') { // trivial case "[]"
        jl_next(lx); // consume RBRACK
        return array;
    }
    while(1) {
        JsonNode *val = jp_parse_value(lx);
        if (jp_is_error(val) || !val) goto error;

        if (tail) tail->next_sibling = val;
        else array->first_child = val;
        tail = val;

        tok = jl_next(lx);
        if (tok.type == JTK_RBRACK) break;
        if (tok.type == JTK_COMMA) continue;
        goto error;

    }
    return array;
error:
    jp_free_ast(array);
    return jp_error("Error while parsing array.");
}

// TODO: find a way to handle the errors which should be compatible with the error handling in jp_parse
JsonNode* jp_parse_obj(JLexer *lx) {
    JsonNode *obj = jp_create_node(JND_OBJ);
    JsonNode *tail = NULL;
    if (!obj) return jp_error("oom (object parsing)");

    JToken tok = jl_next(lx);
    
    if (tok.type != JTK_LBRACE) return jp_error("Expected {");

    char *key = NULL;
    char fflag = 1;
    while(1) {
        tok = jl_next(lx);
        if (tok.type == JTK_RBRACE) break;
        if(!fflag) {
            if (tok.type != JTK_COMMA) goto error;  
            tok = jl_next(lx);
        }
        fflag = 0;
        
        if (tok.type != JTK_STRING) goto error;
        
        key = jl_string_to_utf8(&tok, NULL);
        
        tok = jl_next(lx);
        if (tok.type != JTK_COLON)
            goto error_k;
        
        JsonNode *val = jp_parse_value(lx);
        if (jp_is_error(val) || !val)
            goto error_k;
        val->key = key;

        if(tail) 
            tail->next_sibling = val;
        else
            obj->first_child = val;
        tail = val;
    }
    return obj;
error_k:
    free(key);
error:
    jp_free_ast(obj);
    return jp_error(NULL);
}

JsonNode* jp_parse(const char *json_str, size_t len) {
    JLexer lx;
    jl_init(&lx, json_str, len);

    JToken tokens[MAX_TOKENS];
    int i = 0;
    for(;;){
        tokens[i] = jl_next(&lx);
        JToken t = tokens[i];
        i++;
        if (t.type == JTK_ERROR) {
            fprintf(stderr, "ERROR: %s\n", t.err_msg ? t.err_msg : "token error");
            return jp_error("JSON validation error.");
        }
        if (t.type == JTK_EOF) break;
    }

    jl_init(&lx, json_str, len); // re-initialize lexer for parsing
    JsonNode *root = jp_parse_obj(&lx);
    return root;
}

JsonNode* jp_parse_file(const char *filename) {
    char *json_str = read_file(filename);
    if (!json_str) {
        free(json_str);
        return jp_error("Error. Could not read file.");
    }
    size_t len = strlen(json_str);
    JsonNode *root = jp_parse(json_str, len);
    free(json_str);
    return root;
}

char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory and read
    char *content = malloc(length + 1);
    if (content) {
        unsigned int r = fread(content, 1, length, file);
        if(r <= 0) {
            if(ferror(file))
                fprintf(stderr, "Error reading file %s\n", filename);
            else
                fprintf(stderr, "Error: No data read from file %s\n", filename);
            fclose(file);
            free(content);
            return NULL;
        }
        content[length] = '\0';
    }
    fclose(file);
    return content;
}

int main(int argc, char *argv[]) {
    
    /*
    if (argc != 2) {
        printf("Usage: %s <json_file>\n", argv[0]);
        return 1;
    }
    */
    
    char *filename = "./tests/test.json";
    
    char *json = read_file(filename);
    size_t json_len = strlen(json);
    
    JToken tokens[MAX_TOKENS];
    
    { // Lexing demo
        JLexer lx;
        jl_init(&lx, json, json_len);
        int i = 0;
        for(;;){
            tokens[i] = jl_next(&lx);
            JToken t = tokens[i];
            i++;
            printf("%zu:%zu %-6s  '%.*s'\n", t.line, t.column, kname(t.type), (int)t.length, t.start);
            if (t.type == JTK_STRING){
                const char *err = NULL;
                char *s = jl_string_to_utf8(&t, &err);
                if (s){ printf("       decoded: \"%s\"\n", s); free(s); }
                else   printf("       decode error: %s\n", err ? err : "(unknown)");
            }
            if (t.type == JTK_NUMBER){ double v; if(jl_number_to_double(&t,&v)) printf("       number: %g\n", v); }
            if (t.type == JTK_ERROR){ fprintf(stderr, "ERROR: %s\n", t.err_msg ? t.err_msg : "token error"); break; }
            if (t.type == JTK_EOF) break;
        }
    }

    JsonNode *root = jp_parse(json, json_len);

    jp_print_ast(root);
    if (jp_is_error(root))
        printf("Error message: %s\n", root->value.err_msg);
    printf("\n\n======== Node type: %s\n", nname(root));
    
    jp_free_ast(root);

    return 0;
}