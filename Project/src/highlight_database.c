#include "MyVim.h"

struct editorSyntax *HLDB = NULL;

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
char *C_HL_keywords[] = {
    "switch",    "if",      "while", "for",     "break",   "continue", "return",
    "else",      "struct",  "union", "typedef", "static",  "enum",     "class",
    "case",      "NULL",    "int|",  "long|",   "double|", "float|",   "char|",
    "unsigned|", "signed|", "void|", NULL};

void initHLDB() {
    HLDB = malloc(sizeof(struct editorSyntax) * HLDB_ENTRIES);
    HLDB[0] = (struct editorSyntax){
        "c",
        C_HL_extensions,
        C_HL_keywords,
        "//",
        "/*",
        "*/",
        "#",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_PREPROC};
};

int editorSyntaxToColor(int hl) {
    switch (hl & 0x0f) {
        case HL_COMMENT:
        case HL_MLCOMMENT:
            return 36;
        case HL_PREPROC:
            return 92;
        case HL_KEYWORD1:
            return 33;
        case HL_KEYWORD2:
            return 32;
        case HL_STRING:
            return 35;
        case HL_NUMBER:
            return 31;
        case HL_MATCH:
            return 34;
        default:
            return 37;
    }
}

int editorBGSyntaxToColor(int hl) {
    switch (hl - (hl & 0x0f)) {
        case HL_BG_OPEN_CLOSE_TAG:
            return 44;
        case HL_BG_SELECTED_TEXT:
            return 41;
        default:
            return 40;
    }
}