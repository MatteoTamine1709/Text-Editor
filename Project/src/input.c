#include "MyVim.h"

/*** input ***/

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
    size_t bufsize = 128;
    char *buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        editorSetStatusMessage(prompt, buf);
        editorRefreshScreen();

        int c = editorReadKey();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            editorSetStatusMessage("");
            if (callback) callback(buf, c);
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                if (callback) callback(buf, c);
                editorSetStatusMessage("");
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }

        if (callback) callback(buf, c);
    }
}

void editorMoveCursor(int key) {
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int prevY = E.cy;

    switch (key) {
        case ALT_ARROW_LEFT:
            if (E.cx != 0)
                E.cx = 0;
            else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case CTRL_ARROW_LEFT:
            if (E.cx == 0 && E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
                row = &E.row[E.cy];
                if (!row) break;
            }
            while (E.cx > 0 && (row->chars[E.cx - 1] == ' ' ||
                                row->chars[E.cx - 1] == '\t'))
                E.cx--;
            while (E.cx > 0 && row->chars[E.cx - 1] != ' ' &&
                   row->chars[E.cx - 1] != '\t')
                E.cx--;
            break;
        case SHIFT_ARROW_LEFT:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (E.cx != 0)
                E.cx--;
            else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case CTRL_SHIFT_ARROW_LEFT:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (E.cx == 0 && E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
                row = &E.row[E.cy];
                if (!row) break;
            }
            while (E.cx > 0 && (row->chars[E.cx - 1] == ' ' ||
                                row->chars[E.cx - 1] == '\t'))
                E.cx--;
            while (E.cx > 0 && row->chars[E.cx - 1] != ' ' &&
                   row->chars[E.cx - 1] != '\t')
                E.cx--;
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case ALT_SHIFT_ARROW_LEFT:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (E.cx != 0)
                E.cx = 0;
            else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case ARROW_LEFT:
            if (E.startselectedx != -1 || E.startselectedy != -1) {
                E.cx = (E.startselectedx < E.endselectedx ? E.startselectedx
                                                          : E.endselectedx) -
                       E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
                E.cy = E.startselectedy < E.endselectedy ? E.startselectedy
                                                         : E.endselectedy;
            } else {
                if (E.cx != 0)
                    E.cx--;
                else if (E.cy > 0) {
                    E.cy--;
                    E.cx = E.row[E.cy].size;
                }
            }
            break;
        case ALT_ARROW_RIGHT:
            if (row && E.cx < row->size)
                E.cx = row->size;
            else if (row && E.cx == row->size && E.cy < E.numrows - 1) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case CTRL_ARROW_RIGHT:
            if (!row) break;
            if (E.cx == row->size && E.cy < E.numrows - 1) {
                E.cy++;
                E.cx = 0;
                row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
                if (!row) break;
            }
            while (E.cx < row->size &&
                   (row->chars[E.cx] == ' ' || row->chars[E.cx] == '\t'))
                E.cx++;
            while (E.cx < row->size) {
                E.cx++;
                if (row->chars[E.cx] == ' ' || row->chars[E.cx] == '\t' ||
                    E.cx == row->size)
                    break;
            }
            break;
        case SHIFT_ARROW_RIGHT:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (row && E.cx < row->size)
                E.cx++;
            else if (row && E.cx == row->size && E.cy < E.numrows - 1) {
                E.cy++;
                E.cx = 0;
            }
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case CTRL_SHIFT_ARROW_RIGHT:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (!row) break;
            if (E.cx == row->size && E.cy < E.numrows - 1) {
                E.cy++;
                E.cx = 0;
                row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
                if (!row) break;
            }
            while (E.cx < row->size &&
                   (row->chars[E.cx] == ' ' || row->chars[E.cx] == '\t'))
                E.cx++;
            while (E.cx < row->size) {
                E.cx++;
                if (row->chars[E.cx] == ' ' || row->chars[E.cx] == '\t' ||
                    E.cx == row->size)
                    break;
            }
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case ALT_SHIFT_ARROW_RIGHT:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (row && E.cx < row->size)
                E.cx = row->size;
            else if (row && E.cx == row->size && E.cy < E.numrows - 1) {
                E.cy++;
                E.cx = 0;
            }
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case ARROW_RIGHT:
            if (E.startselectedx != -1 || E.startselectedy != -1) {
                E.cx = (E.startselectedx > E.endselectedx ? E.startselectedx
                                                          : E.endselectedx) -
                       E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
                E.cy = E.startselectedy > E.endselectedy ? E.startselectedy
                                                         : E.endselectedy;
            } else {
                if (row && E.cx < row->size)
                    E.cx++;
                else if (row && E.cx == row->size && E.cy < E.numrows - 1) {
                    E.cy++;
                    E.cx = 0;
                }
            }
            break;
        case ALT_ARROW_UP: {
            if (!row) break;
            if (E.cy != 0) E.cy--;
            erow *row2 = &E.row[E.cy];
            erow tmp = *row2;
            *row2 = *row;
            *row = tmp;
            E.dirty++;
            break;
        }
        case SHIFT_ARROW_UP:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx = editorRowCxToRx(row, E.cx);
                E.startselectedy = E.cy;
            }
            if (E.cy != 0) E.cy--;
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
            if (E.cy != 0) E.cy--;
            break;
        case ARROW_UP:
            if (E.startselectedx != -1 || E.startselectedy != -1)
                E.cy = E.startselectedy < E.endselectedy ? E.startselectedy
                                                         : E.endselectedy;
            if (E.cy != 0) E.cy--;
            break;
        case ALT_ARROW_DOWN: {
            if (!row) break;
            if (E.cy < E.numrows - 1) E.cy++;
            erow *row2 = &E.row[E.cy];
            erow tmp = *row2;
            *row2 = *row;
            *row = tmp;
            E.dirty++;
            break;
        }
        case SHIFT_ARROW_DOWN:
            if (E.startselectedx == -1 || E.startselectedy == -1) {
                E.startselectedx =
                    editorRowCxToRx(row,
                                    E.cx);  // - E.options.shownumberline *
                                            // KILO_SHOW_LINE_NUMBER_SPACE;
                E.startselectedy = E.cy;
            }
            if (E.cy < E.numrows) E.cy++;
            E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx);
            E.endselectedy = E.cy;
            break;
        case ARROW_DOWN:
            if (E.startselectedx != -1 || E.startselectedy != -1)
                E.cy = E.startselectedy > E.endselectedy ? E.startselectedy
                                                         : E.endselectedy;
            if (E.cy < E.numrows - 1) E.cy++;
            break;
    }

    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    E.hoverchar = row && E.cx < row->size ? row->chars[E.cx] : E.hoverchar;
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
}

void editorProcessKeypress() {
    static int quit_times = QUIT_TIMES;
    int c = editorReadKey();

    if (c == '\t' && USE_SPACE_AS_TABS) {
        for (int i = 0; i < TAB_STOP; i++) editorInsertChar(' ');
        return;
    }

    switch (c) {
        case '\r':
            editChangeStack(c, E.cy);
            editorInsertNewline();
            break;
        case CTRL_KEY('q'):
            if (E.dirty && quit_times > 0) {
                editorSetStatusMessage(
                    "WARNING!!! File has unsaved changes. Press Ctrl-Q %d more "
                    "times "
                    "to quit.",
                    quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case CTRL_KEY('s'):
            editorSave();
            break;

        case CTRL_KEY('c'):
            editorCopy();
            break;
        case CTRL_KEY('v'):
            editorPaste();
            break;

        case HOME_KEY:
            E.cx = 0;
            break;
        case END_KEY:
            if (E.cy < E.numrows) E.cx = E.row[E.cy].size;
            break;

        case CTRL_KEY('f'):
            editorFind();
            break;

        case CTRL_KEY('r'):
            editorReplace();
            break;

        case CTRL_KEY('o'):
            editorOpen();
            break;

        case CTRL_KEY('g'):
            editorGoToLine();
            break;

        case CTRL_KEY('z'):
            editorUndo();
            editChangeStack(c, E.cy);
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            editChangeStack(c, E.cy);
            removeTextIfSelected();
            if (c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                E.cy = E.rowoff;
            } else if (c == PAGE_DOWN) {
                E.cy = E.rowoff + E.screenrows - 1;
                if (E.cy > E.numrows) E.cy = E.numrows - 1;
            }
            int times = E.screenrows;
            while (times--)
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        } break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case CTRL_ARROW_RIGHT:
        case CTRL_ARROW_LEFT:
        case ALT_ARROW_RIGHT:
        case ALT_ARROW_LEFT:
        case ALT_ARROW_UP:
        case ALT_ARROW_DOWN:
        case SHIFT_ARROW_LEFT:
        case SHIFT_ARROW_RIGHT:
        case SHIFT_ARROW_UP:
        case SHIFT_ARROW_DOWN:
        case CTRL_SHIFT_ARROW_UP:
        case CTRL_SHIFT_ARROW_DOWN:
        case CTRL_SHIFT_ARROW_RIGHT:
        case CTRL_SHIFT_ARROW_LEFT:
        case ALT_SHIFT_ARROW_UP:
        case ALT_SHIFT_ARROW_DOWN:
        case ALT_SHIFT_ARROW_RIGHT:
        case ALT_SHIFT_ARROW_LEFT:
            editorMoveCursor(c);
            break;

        case CTRL_KEY('l'):
        case '\x1b':
            break;

        default:
            editChangeStack(c, E.cy);
            editorInsertChar(c);
            break;
    }
    if (c != SHIFT_ARROW_DOWN && c != SHIFT_ARROW_LEFT &&
        c != SHIFT_ARROW_RIGHT && c != SHIFT_ARROW_UP &&
        c != CTRL_SHIFT_ARROW_RIGHT && c != CTRL_SHIFT_ARROW_LEFT &&
        c != ALT_SHIFT_ARROW_RIGHT && c != ALT_SHIFT_ARROW_LEFT &&
        c != ALT_SHIFT_ARROW_UP && c != ALT_SHIFT_ARROW_DOWN &&
        c != CTRL_KEY('c') &&
        !(c == '\t' && E.startselectedy != -1 &&
          E.startselectedy != E.endselectedy)) {
        E.startselectedx = -1;
        E.startselectedy = -1;
    }
    quit_times = QUIT_TIMES;
    refreshAllSyntax();
}