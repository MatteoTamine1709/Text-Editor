#include "MyVim.h"

/*** open ***/

void editorOpenCallback(char *filename, int key) {
    if (key != '\r' || key == '\x1b') return;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        editorSetStatusMessage("File not found");
        return;
    }
    free(E.filename);
    for (int i = 0; i < E.numrows; i++) {
        free(E.row[i].chars);
        free(E.row[i].render);
        free(E.row[i].hl);
    }

    E.numrows = 0;
    E.cx = 0;  // E.options.shownumberline * KILO_SHOW_LINE_NUMBER_SPACE;
    E.cy = 0;
    E.filename = strdup(filename);

    editorSelectSyntaxHighlight();

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 &&
               (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        editorInsertRow(E.numrows, line, linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

void editorOpen() {
    static int quit_times = QUIT_TIMES;
    if (E.dirty && quit_times > 0) {
        editorSetStatusMessage(
            "WARNING!!! File has unsaved changes. Press Ctrl-O %d more times "
            "to quit.",
            quit_times);
        quit_times--;
        return;
    }
    char *query = editorPrompt("Open: %s (Use ESC/Enter)", editorOpenCallback);
    quit_times = QUIT_TIMES;
    if (!query) return;
    FILE *fp = fopen(query, "r");
    if (!fp) {
        editorSetStatusMessage("File not found");
        return;
    }
    free(query);
}