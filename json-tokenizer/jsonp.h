#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "jsont.h"

#ifndef JSONP_H
#define JSONP_H

#define MAX_TOKENS 1024
#define MAX_NESTING 64
#define MAX_TEXT_SIZE 
#define MAX_RANGE_NUM

typedef enum {
    JND_ROOT_OBJ, JND_OBJ,
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
        int64_t ivalue;
        const char *svalue;
        const char err_msg[256];
    } value;
    struct JsonNode *first_child; // JDN_PROPERTY can ONLY have 2 children (one for the key and one for the value) while for JND_STRING, JND_NUMBER, JND_BOOL, and JND_NULL should be NULL
    struct JsonNode *next_sibling;
    char *key; // Only used if the node is child of an JND_OBJ
    JNodeType type;
    bool is_integer;        // Valid only for JND_NUMBER: true => ivalue is set, false => nvalue is set
    size_t child_size;      // For JND_STRING, JND_NUMBER, JND_BOOL, and JND_NULL must be 0 while for JND_PROPERTY must be 2
} JsonNode;

extern JsonNode error_node;


// Parsing APIs
JsonNode* jp_parse(const char *json_str, size_t len);
JsonNode* jp_parse_file(const char*filename);
JsonNode* jp_parse_obj(JLexer *lx, unsigned short int depth);
JsonNode* jp_parse_array(JLexer*lx, unsigned short int depth);
JsonNode* jp_parse_value(JLexer *lx, unsigned short int depth);
void jp_free_ast(JsonNode *root);
static JsonNode* jp_create_node(JNodeType type);
void jp_print_ast(JsonNode *root);

// Enhance the error handling.
//  - Fix the parsing function and point out in which part of the token list and json string the error occurred.
static inline void jp_error_set_msg(const char *err_msg) {
    if (!err_msg) return;
    strncpy((char*)error_node.value.err_msg, err_msg, 256);
}

static inline JsonNode* jp_error(const char* err_msg) {
    jp_error_set_msg(err_msg);
    return &error_node;
}

static inline bool jp_is_error(JsonNode *node) {
    return node && node->type == JND_ERROR;
}


static inline const char* nname(const JsonNode *node){
    if (!node) return "NULL";
    JNodeType k = node->type;
    switch(k){
        case JND_ROOT_OBJ: return "ROOT"; case JND_OBJ : return "OBJ"; case JND_ARRAY: return "ARRAY";
        case JND_STRING: return "STRING"; case JND_NUMBER: return "NUMBER"; case JND_ERROR: return "ERROR";
        case JND_BOOL: return "BOOL"; case JND_NULL: return "NULL";
    } return "?";
}

static inline char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("open");
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

#endif