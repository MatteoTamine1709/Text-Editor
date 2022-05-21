#include "MyVim.h"

/*** replace ***/

static char *search = NULL;

void editor1stCallback(char *query, int key) {}
void editorReplaceCallback(char *query, int key) {
    if (key == '\r') {
        for (int i = 0; i < E.numrows; i++) {
            erow *row = &E.row[i];
            char *match = strstr(row->chars, search);
            while (match) {
                E.cy = i;
                E.cx = match - row->chars;
                for (int i = strlen(search); i > 0; --i) {
                    E.cx = match - row->chars + i;
                    editorDelChar();
                }
                E.rowoff = E.numrows;
                match = strstr(match, search);
                for (int j = 0; j < strlen(query); ++j)
                    editorInsertChar(query[j]);
            }
        }
    }
}

void editorReplace() {
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    search =
        editorPrompt("Search: %s (Use ESC/Arrows/Enter)", editor1stCallback);
    if (!search) {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
        return;
    }
    char *replace =
        editorPrompt("Replace: %s (Use ESC/Arrows/Enter)", editorReplaceCallback);
    if (replace) {
        free(search);
        free(replace);
    } else {
        if (search)
            free(search);
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}