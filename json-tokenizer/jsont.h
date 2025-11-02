#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>


typedef enum {
    JTK_EOF = 0, JTK_ERROR,
    JTK_LBRACE, JTK_RBRACE, JTK_LBRACK, JTK_RBRACK,
    JTK_COLON, JTK_COMMA,
    JTK_STRING, JTK_NUMBER, JTK_TRUE, JTK_FALSE, JTK_NULL
} JTokenType;

typedef struct {
    JTokenType type;
    const char  *start;
    size_t      length;
    size_t line, column;
    const char *err_msg;
 } JToken;


typedef struct {
    const char *buf, *p, *end;
    size_t line, col;
} JLexer;


static inline bool jl_at_end(JLexer *lx);
static inline char jl_peek(JLexer *lx); // Returns the current char pointed by the lexer, '\0' otherwise
static inline char jl_adv(JLexer *lx);
static inline bool jl_match(JLexer *lx , char c); // Makes the lexer's pointer advance. Returns true if the current char pointed by the lexer matches the given char, false otherwise.

static JToken jl_create_token(JLexer *lx, JTokenType type, const char *start, size_t line, size_t col);
static JToken jl_error(JLexer *lx, const char *start, size_t line, size_t col, const char *msg);
void jl_init(JLexer *lx, const char *data, size_t len);

static void jl_skip_ws(JLexer *lx);


// ---- UTF-8 helpers (for \uXXXX decoding)
static void utf8_emit(uint32_t cp, char **out);
static int hex_val(char c);


// Decode a STRING token to UTF-8 malloc'd buffer.
// Pass token that still includes the surrounding quotes.
char* jl_string_to_utf8(const JToken *t, const char **err_msg_out);

// Decode a JSON string slice (without quotes) into a freshly malloc'd UTF-8 buffer.
// Returns NULL on error; sets *err_msg if provided.
static char* json_decode_string(const char *s, size_t n, const char **err_msg);

// Parser a JSON string token (including quotes), but doesn't allocate.
// Returns a token slice; decoding can be done separately with json_decode_string
static JToken jl_scan_string(JLexer *lx);

// Convert a NUMBER token to double (strict strtod on a temporary null-terminated copy).
bool jl_number_to_double(const JToken *t, double *out);

static JToken jl_scan_number(JLexer *lx, char first);

static bool jl_consume_kw(JLexer *lx, const char *kw);


JToken jl_next(JLexer *lx);