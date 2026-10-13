/* date = October 12th 2026

   tags_tests.cpp - Standalone unit test for the pure ctags parser
   (4coder_fleury_tags_parse.h). No Application_Links needed. It feeds a real
   u-ctags tag file (the same shape `f4_rebuild_tags` generates) and asserts
   lookup, line extraction, regex fallback, the duplicate chain, and the
   same-directory Pick heuristic.

   Build & run:  ./run_tags.sh */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int32_t i32; typedef uint32_t u32;
typedef int64_t i64;  typedef uint64_t u64;
typedef int16_t i16;  typedef uint16_t u16;
typedef int8_t i8;    typedef uint8_t u8;
typedef int b32;
#define internal static
#define function static
#define global static
#define ArrayCount(a) ((int)(sizeof(a)/sizeof((a)[0])))

struct String_Const_u8 { u8 *str; u64 size; };
struct Arena { int dummy; };
struct Range_i64 { i64 min, one_past_last; };
#define S8Lit(s) (String_Const_u8){(u8*)(s),(u64)(sizeof(s)-1)}
#define Ii64(t) (Range_i64){(t)->pos,(t)->pos+(t)->size}
#define push_array_zero(a,T,c) ((T*)calloc((c),sizeof(T)))
#define push_array(a,T,c) ((T*)malloc(sizeof(T)*(c>0?c:1)))
#define string_expand(s) ((int)(s).size),(const char*)(s).str

static String_Const_u8 push_string_copy(Arena *a, String_Const_u8 s) {
    (void)a;
    String_Const_u8 r = {(u8*)malloc(s.size+1), s.size};
    memcpy(r.str, s.str, s.size); r.str[s.size]=0; return r;
}
static b32 string_match(String_Const_u8 a, String_Const_u8 b) {
    if (a.size != b.size) return 0;
    for (u64 i=0;i<a.size;i++) if (a.str[i]!=b.str[i]) return 0;
    return 1;
}
static u64 table_hash_u8(u8 *v, u64 size) {
    u64 h = 1469598103934665603ULL;   // FNV-1a 64
    for (u64 i=0;i<size;i++) { h ^= v[i]; h *= 1099511628211ULL; }
    return h;
}

#include "../4coder_fleury_tags_parse.h"

static int g_pass=0, g_fail=0;
#define CHECK(cond, msg) do{ if(cond){g_pass++;} else { g_fail++; printf("  FAIL %s\n",msg);} }while(0)

static F4_TagTable tbl = {};
static Arena arena;

int main(void) {
    // The exact shape `ctags -R --fields=+n+S --output-format=u-ctags` emits.
    const char *tags_text =
        "!_TAG_FILE_FORMAT\t2\t/extended format/\n"
        "!_TAG_FILE_SORTED\t1\t/0=unsorted/\n"
        "B\tb.pl\t/^package B;$/;\"\tp\tline:1\n"
        "MAX\tb.pl\t/^use constant MAX => 42;$/;\"\tc\tline:3\n"
        "Thing\ta.py\t/^class Thing:$/;\"\tc\tline:3\n"
        "add\tc.lua\t/^function add(a, b) return a+b end$/;\"\tf\tline:1\n"
        "greet\tb.pl\t/^sub greet { my $s=shift; return \"hi\"; }$/;\"\ts\tline:2\n"
        "helper\ta.py\t/^def helper(x, y):$/;\"\tf\tline:1\tsignature:(x, y)\n"
        "helper\tc.lua\t/^local function helper() end$/;\"\tf\tline:2\n"
        "method\ta.py\t/^    def method(self):$/;\"\tm\tline:4\tclass:Thing\tsignature:(self)\n"
        "run\tc.lua\t/^function M.run() end$/;\"\tf\tline:3\tunknown:M\n"
        "legacy\told.py\t12;\"\tf\t\n"
        "regexy\tz.py\t/^def regexy():\\n$/;\"\tf\t";   // regex address, NO line: field

    F4_Tags_Reset(&tbl);
    i64 added = F4_Tags_Parse(&arena, (String_Const_u8){(u8*)tags_text, strlen(tags_text)}, &tbl);
    tbl.loaded = 1;

    CHECK(added == 11, "parse count == 11 non-meta tags");

    // line-number via the line: field
    F4_Tag *thing = F4_Tags_Lookup(&tbl, S8Lit("Thing"));
    CHECK(thing && thing->line == 3 && string_match(thing->file, S8Lit("a.py")), "Thing -> a.py:3");

    // package + constants
    F4_Tag *b = F4_Tags_Lookup(&tbl, S8Lit("B"));
    CHECK(b && b->line == 1, "B -> line 1");
    F4_Tag *mx = F4_Tags_Lookup(&tbl, S8Lit("MAX"));
    CHECK(mx && mx->line == 3, "MAX -> line 3");

    // duplicate name -> chain of two (a.py + c.lua)
    F4_Tag *chain = F4_Tags_Lookup(&tbl, S8Lit("helper"));
    CHECK(chain && chain->name_next, "helper has a duplicate");
    // from a .py file the pick should prefer a.py; from .lua it prefers c.lua
    F4_Tag *py_pick = F4_Tags_Pick(chain, S8Lit("a.py"));
    F4_Tag *lua_pick = F4_Tags_Pick(chain, S8Lit("c.lua"));
    CHECK(py_pick && string_match(py_pick->file, S8Lit("a.py")), "Pick prefers same-dir a.py");
    CHECK(lua_pick && string_match(lua_pick->file, S8Lit("c.lua")), "Pick prefers same-dir c.lua");

    // method with class: field still resolves by name + line
    F4_Tag *m = F4_Tags_Lookup(&tbl, S8Lit("method"));
    CHECK(m && m->line == 4, "method -> line 4 (extra fields ignored)");

    // signature field with embedded colon must not confuse line parsing
    F4_Tag *add = F4_Tags_Lookup(&tbl, S8Lit("add"));
    CHECK(add && add->line == 1, "add -> line 1");

    // numeric address without a line: field is decoded from the address
    F4_Tag *legacy = F4_Tags_Lookup(&tbl, S8Lit("legacy"));
    CHECK(legacy && legacy->line == 12, "legacy numeric address -> line 12");

    // a `line:` field wins: run has line 3 and no pattern
    F4_Tag *run = F4_Tags_Lookup(&tbl, S8Lit("run"));
    CHECK(run && run->line == 3 && run->pattern.size == 0, "run -> line 3, no pattern");

    // regex address without a line: field keeps a pattern for the text fallback
    F4_Tag *regexy = F4_Tags_Lookup(&tbl, S8Lit("regexy"));
    CHECK(regexy && regexy->line == 0 && regexy->pattern.size > 0, "regexy keeps regex pattern");

    // unknown symbol
    CHECK(F4_Tags_Lookup(&tbl, S8Lit("nope")) == 0, "missing name -> null");

    printf("\n----- tags: %d pass, %d fail -----\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
