#include "jsonp.h"

static JsonNode* jp_create_node(JNodeType type) {
    JsonNode *node = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!node) return NULL;
    node->type = type;
    return node;
}

static void jp_print_ast_rec(JsonNode *node, unsigned depth) {
    if (!node) return;
    for (unsigned i = 0; i < depth; i++) printf("%s", "   ");
    printf("%s", (const char *)(depth ? ((node->next_sibling == NULL) ? "└─ " : "├─ ") : "└─ ")); // branch connector

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
        jp_print_ast_rec(child, depth + 1);
        child = child->next_sibling;
    }
}

void jp_print_ast(JsonNode *root) {
    if(!root) return;
    printf("\n\n=== JSON AST ===\n");
    jp_print_ast_rec(root, 0);
}

void jp_free_ast(JsonNode *root) {
    if (!root) return;
    if (jp_is_error(root)) return; // !! Do not free global error node
    JsonNode *child = root->first_child;
    while (child) {
        JsonNode *next = child->next_sibling;
        jp_free_ast(child);
        child = next;
    }
    #ifdef DEBUG
        printf("[DEBUG] Freeing %s node.\n", nname(root));
    #endif
    if (root->key) free(root->key);
    free(root);
}

// TODO: change it to another name I don't like the value since here we're parsing STRINGS, NUMBERS and PRIMITIVES (BOOL/NULL)
JsonNode* jp_parse_value(JLexer *lx, unsigned short int depth) {
    // I know it's ugly but it's fuctional :) (maybe I' gonna change it later ... or maybe no!)
    if (depth > MAX_NESTING) return jp_error("maximum nesting reached.");
    jl_skip_ws(lx);
    if (jl_peek(lx) == '{')
        return jp_parse_obj(lx, depth + 1);
    else if (jl_peek(lx) == '[')
        return jp_parse_array(lx, depth + 1);

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

JsonNode* jp_parse_array(JLexer *lx, unsigned short int depth) {
    JsonNode *array = jp_create_node(JND_ARRAY);
    JsonNode *tail = NULL;

    if (!array) return jp_error("oom (array)");
    if (depth > MAX_NESTING) {
        jp_error_set_msg("maximum nesting reached.");
        goto error;
    }

    JToken tok = jl_next(lx);
    if (tok.type != JTK_LBRACK) return jp_error("Error. Expected [' while array parsing.");
    jl_skip_ws(lx);
    if (jl_peek(lx) == ']') { // trivial case "[]"
        jl_next(lx); // consume RBRACK
        return array;
    }
    while(1) {
        JsonNode *val = jp_parse_value(lx, depth);
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
JsonNode* jp_parse_obj(JLexer *lx, unsigned short int depth) {
    JsonNode *obj = jp_create_node(JND_OBJ);
    JsonNode *tail = NULL;
    if (!obj) jp_error("oom (object parsing).");
    if (depth > MAX_NESTING) {
        jp_error_set_msg("maximum nesting reached.");
        goto error;
    }

    JToken tok = jl_next(lx);
    
    if (tok.type != JTK_LBRACE) return jp_error("Expected {");

    char *key = NULL;
    char fflag = 1;
    while(1) {
        tok = jl_next(lx);
        if (tok.type == JTK_RBRACE) break;
        if(!fflag) {
            if (tok.type != JTK_COMMA) {
                jp_error_set_msg("Error. Expected COMMA between properties.");
                goto error;
            }
            tok = jl_next(lx);
        }
        fflag = 0;
        
        if (tok.type != JTK_STRING) {
            jp_error_set_msg("Error. Expected STRING as property key.");
            goto error;
        }
        
        key = jl_string_to_utf8(&tok, NULL);
        
        tok = jl_next(lx);
        if (tok.type != JTK_COLON)
            goto error_k;
        
        JsonNode *val = jp_parse_value(lx, depth);
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
    JsonNode *root = jp_parse_obj(&lx, 0);
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

int main(int argc, char *argv[]) {
    
    /*
    if (argc != 2) {
        printf("Usage: %s <json_file>\n", argv[0]);
        return 1;
    }
    */
    
    char *filename = "./tests/test.json";
    
    char *json = read_file(filename);
    if(!json) {
        return 1;
    }
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