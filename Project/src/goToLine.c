#include "MyVim.h"

/*** go to line ***/

void editorGoToLineCallback(char *query, int key) {
    if (atoi(query) > 0 && atoi(query) <= E.numrows) {
        E.cy = atoi(query) - 1;
        E.cx = E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
    }
}

void editorGoToLine() {
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    char *query =
        editorPrompt("Open: %s (Use ESC/Enter)", editorGoToLineCallback);
    if (query)
        free(query);
    else {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}