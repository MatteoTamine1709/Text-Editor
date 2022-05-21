#include "MyVim.h"

/*** terminal ***/

void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
        if (nread == -1 && errno != EAGAIN) die("read");

    if (c == '\x1b') {
        char seq[5];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1':
                            return HOME_KEY;
                        case '3':
                            return DEL_KEY;
                        case '4':
                            return END_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        case '7':
                            return HOME_KEY;
                        case '8':
                            return END_KEY;
                    }
                } else if (seq[2] == ';') {
                    if (read(STDIN_FILENO, &seq[3], 1) != 1) return '\x1b';
                    if (seq[3] == '5') {
                        if (read(STDIN_FILENO, &seq[4], 1) != 1) return '\x1b';
                        switch (seq[4]) {
                            case 'A':
                                return CTRL_ARROW_UP;
                            case 'B':
                                return CTRL_ARROW_DOWN;
                            case 'C':
                                return CTRL_ARROW_RIGHT;
                            case 'D':
                                return CTRL_ARROW_LEFT;
                            case 'H':
                                return CTRL_HOME_KEY;
                            case 'F':
                                return CTRL_END_KEY;
                        }
                    } else if (seq[3] == '3') {
                        if (read(STDIN_FILENO, &seq[4], 1) != 1) return '\x1b';
                        switch (seq[4]) {
                            case 'A':
                                return ALT_ARROW_UP;
                            case 'B':
                                return ALT_ARROW_DOWN;
                            case 'C':
                                return ALT_ARROW_RIGHT;
                            case 'D':
                                return ALT_ARROW_LEFT;
                            case 'H':
                                return ALT_HOME_KEY;
                            case 'F':
                                return ALT_END_KEY;
                        }
                    } else if (seq[3] == '2') {
                        if (read(STDIN_FILENO, &seq[4], 1) != 1) return '\x1b';
                        switch (seq[4]) {
                            case 'A':
                                return SHIFT_ARROW_UP;
                            case 'B':
                                return SHIFT_ARROW_DOWN;
                            case 'C':
                                return SHIFT_ARROW_RIGHT;
                            case 'D':
                                return SHIFT_ARROW_LEFT;
                        }
                    } else if (seq[3] == '6') {
                        if (read(STDIN_FILENO, &seq[4], 1) != 1) return '\x1b';
                        switch (seq[4]) {
                            case 'A':
                                return CTRL_SHIFT_ARROW_UP;
                            case 'B':
                                return CTRL_SHIFT_ARROW_DOWN;
                            case 'C':
                                return CTRL_SHIFT_ARROW_RIGHT;
                            case 'D':
                                return CTRL_SHIFT_ARROW_LEFT;
                        }
                    } else if (seq[3] == '4') {
                        if (read(STDIN_FILENO, &seq[4], 1) != 1) return '\x1b';
                        switch (seq[4]) {
                            case 'A':
                                return ALT_SHIFT_ARROW_UP;
                            case 'B':
                                return ALT_SHIFT_ARROW_DOWN;
                            case 'C':
                                return ALT_SHIFT_ARROW_RIGHT;
                            case 'D':
                                return ALT_SHIFT_ARROW_LEFT;
                        }
                    }
                }

            } else if (seq[0] == 'O') {
                switch (seq[1]) {
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                }
            } else {
                switch (seq[1]) {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                }
            }
        }
        return '\x1b';
    } else {
        return c;
    }
}

int getCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** row operation ***/

int editorRowCxToRx(erow *row, int cx) {
    int rx = 0;
    int j;

    for (j = 0; j < cx && j < row->rsize; j++) {
        if (row->chars[j] == '\t')
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx + E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
}

int editorRowRxToCx(erow *row, int rx) {
    int cur_rx = 0;
    int cx;
    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t')
            cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
        cur_rx++;
        if (cur_rx > rx) return cx;
    }
    return cx + E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
}

void editorUpdateRow(erow *row, int y) {
    int tabs = 0;
    int j;

    for (j = 0; j < row->size; j++)
        if (row->chars[j] == '\t') tabs++;
    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;

    editorUpdateSyntax(row, y);
}

void editorInsertRow(int at, char *s, size_t len) {
    if (at < 0 || at > E.numrows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
    for (int j = at + 1; j <= E.numrows; j++) E.row[j].idx++;

    E.row[at].idx = at;

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    E.row[at].hl = NULL;
    E.row[at].hl_open_comment = 0;
    editorUpdateRow(&E.row[at], at);

    E.numrows++;
    E.dirty++;
}

void editorFreeRow(erow *row) {
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void editorDelRow(int at) {
    if (at < 0 || at >= E.numrows) return;

    editorFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    for (int j = at; j < E.numrows - 1; j++) E.row[j].idx--;
    E.numrows--;
    E.dirty++;
}

void editorRowInsertChar(erow *row, int at, int c, int y) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row, y);
    E.dirty++;
}

void editorRowAppendString(erow *row, char *s, size_t len, int y) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row, y);
    E.dirty++;
}

void editorRowAppendStringAt(erow *row, char *s, int at, size_t len, int y) {
    char *tmp = NULL;
    if (row->size > at) {
        tmp = malloc(row->size - at);
        memcpy(tmp, row->chars + at, row->size - at);
    }
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[at], s, len);
    if (row->size > at) {
        memcpy(&row->chars[at + len], tmp, row->size - at);
        free(tmp);
    }
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row, y);
    E.dirty++;
}

void editorRowDelChar(erow *row, int at, int y) {
    if (at < 0 || at >= row->size) return;

    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editorUpdateRow(row, y);
    E.dirty++;
}

/*** editor operations ***/

void editorInsertChar(int c) {
    if (!(c == '\t' && E.startselectedy != -1 &&
          E.startselectedy != E.endselectedy))
        removeTextIfSelected();
    if (E.cy == E.numrows) editorInsertRow(E.numrows, "", 0);
    if (c == '\t' && E.startselectedy != -1 &&
        E.startselectedy != E.endselectedy) {
        for (int y = E.startselectedy; y <= E.endselectedy; ++y) {
            if (E.row[y].size > 0) editorRowInsertChar(&E.row[y], 0, c, y);
        }
        E.endselectedx = editorRowCxToRx(&E.row[E.cy], E.cx + 1);
    } else
        editorRowInsertChar(&E.row[E.cy], E.cx, c, E.cy);
    E.cx++;
}

void editorInsertNewline() {
    removeTextIfSelected();
    int newCXpos = 0;
    if (E.cx == 0) {
        editorInsertRow(E.cy, "", 0);
    } else {
        erow *row = &E.row[E.cy];
        editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row, E.cy);
        int idx = 0;
        erow *newRow = &E.row[E.cy + 1];
        while (idx < row->size) {
            if (row->chars[idx] == '\t') {
                editorRowInsertChar(newRow, 0, '\t', E.cy + 1);
                newCXpos++;
            } else if (row->chars[idx] == ' ') {
                editorRowInsertChar(newRow, 0, ' ', E.cy + 1);
                newCXpos++;
            } else {
                break;
            }
            idx++;
        }
        int end = row->rsize - 1;
        while (end >= 0 && (row->render[end] == '\n' || row->render[end] == '\r' ||
               row->render[end] == ' ' || row->render[end] == '\t')) {
            end--;
        }
        if (end >= 0 && row->render[end] == '{') {
            if (USE_SPACE_AS_TABS) {
                for (int i = 0; i < TAB_STOP; i++) {
                    editorRowInsertChar(newRow, 0, ' ', E.cy + 1);
                    newCXpos++;
                }
            } else {
                editorRowInsertChar(newRow, 0, '\t', E.cy + 1);
                newCXpos++;
            }
        }
    }
    E.cy++;
    E.cx = newCXpos;
}

void editorDelChar() {
    if (E.cy == E.numrows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow *row = &E.row[E.cy];
    if (E.cx > 0) {
        editorRowDelChar(row, E.cx - 1, E.cy);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size;
        editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size,
                              E.cy - 1);
        editorDelRow(E.cy);
        E.cy--;
    }
}

/*** file i/o ***/

char *editorRowsToString(int *buflen) {
    int totlen = 0;
    int j;
    for (j = 0; j < E.numrows; j++) totlen += E.row[j].size + 1;
    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for (j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

void editorSave() {
    if (E.filename == NULL) {
        E.filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);
        if (E.filename == NULL) {
            editorSetStatusMessage("Save aborted");
            return;
        }
        editorSelectSyntaxHighlight();
    }
    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                E.dirty = 0;
                editorSetStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

void editorCopy() {
    int startselectedy = E.startselectedy;
    int startselectedx = E.startselectedx;
    if (startselectedy != -1 && startselectedx != -1) {
        int endselectedy = E.endselectedy;
        int endselectedx = E.endselectedx;
        if (startselectedy > endselectedy) {
            startselectedy = E.endselectedy;
            startselectedx = E.endselectedx;
            endselectedy = E.startselectedy;
            endselectedx = E.startselectedx;
        } else if (startselectedy == endselectedy) {
            if (startselectedx > endselectedx) {
                startselectedx = E.endselectedx;
                endselectedx = E.startselectedx;
            }
        }

        startselectedx =
            editorRowRxToCx(&E.row[startselectedy], startselectedx);
        endselectedx = editorRowRxToCx(&E.row[startselectedy], endselectedx);

        startselectedx -=
            E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
        endselectedx -= E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;

        E.copynbline = endselectedy - startselectedy + 1;
        E.copylens = realloc(E.copylens, sizeof(int) * E.copynbline);
        memset(E.copylens, 0, sizeof(int) * E.copynbline);
        for (int y = startselectedy; y <= endselectedy; ++y) {
            erow *row = &E.row[y];
            E.copylens[y - startselectedy] =
                (y == endselectedy ? endselectedx : row->rsize) -
                (y == startselectedy ? editorRowCxToRx(row, startselectedx) -
                                           E.options.shownumberline *
                                               SHOW_LINE_NUMBER_SPACE
                                     : 0) +
                (startselectedy == endselectedy) -
                (editorRowCxToRx(row, endselectedx) ==
                     row->rsize + E.options.shownumberline *
                                      SHOW_LINE_NUMBER_SPACE &&
                 editorRowCxToRx(row, startselectedx) !=
                     row->rsize + E.options.shownumberline *
                                      SHOW_LINE_NUMBER_SPACE &&
                 startselectedy == endselectedy);
        }
        E.copybuffer = realloc(E.copybuffer, sizeof(char *) * E.copynbline);
        memset(E.copybuffer, 0, sizeof(char *) * E.copynbline);

        for (int y = startselectedy; y <= endselectedy; ++y) {
            erow *row = &E.row[y];
            E.copybuffer[y - startselectedy] =
                realloc(E.copybuffer[y - startselectedy],
                        E.copylens[y - startselectedy] + 1);
            memcpy(E.copybuffer[y - startselectedy],
                   &row->chars[y == startselectedy
                                   ? editorRowCxToRx(row, startselectedx) -
                                         E.options.shownumberline *
                                             SHOW_LINE_NUMBER_SPACE
                                   : 0],
                   E.copylens[y - startselectedy]);
            memset(&E.copybuffer[y - startselectedy]
                                [E.copylens[y - startselectedy]],
                   0, 1);
        }
    }
}

void editorPaste() {
    for (int i = 0; i < E.copynbline; ++i) {
        if (i == E.copynbline - 1) {
            editorRowAppendStringAt(&E.row[E.cy], E.copybuffer[i],
                                    i == 0 ? E.cx : 0, E.copylens[i], E.cy);
            E.cy += i < E.copynbline - 1;
            E.cx = E.copylens[i] + (i == 0 ? E.cx : 0);
        } else {
            erow *row =
                (E.cy >= E.numrows) && (E.cy <= 0) ? NULL : &E.row[E.cy];
            if (row) {
                if (row->size >= E.cx) {
                    char *startLine = malloc(E.cx + E.copylens[i]);
                    char *endLine = malloc(row->size - E.cx);
                    memcpy(startLine, row->chars, E.cx);
                    memcpy(endLine, row->chars + E.cx, row->size - E.cx);
                    memcpy(startLine + E.cx, E.copybuffer[i], E.copylens[i]);
                    editorInsertRow(E.cy, endLine, row->size - E.cx);
                    editorInsertRow(E.cy, startLine, E.cx + E.copylens[i]);
                    editorDelRow(E.cy + 2);
                } else {
                    editorInsertRow(E.cy, E.copybuffer[i], E.copylens[i]);
                    E.cx = E.copylens[i] + (i == 0 ? E.cx : 0);
                }
            } else {
                editorInsertRow(E.cy, "", 0);
                E.cx = 0;
            }
            E.cy++;
        }
    }
}

/*** append buffer ***/

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab) { free(ab->b); }

/*** output ***/

void editorScroll() {
    E.rx = E.cx;
    if (E.cy < E.numrows) {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) {
        E.coloff = E.rx - E.screencols + 1;
    }
}

void editorDrawRows(struct abuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if (E.options.shownumberline && filerow < E.numrows) {
            char number[80];
            int len = snprintf(number, sizeof(number), "%-4d", filerow + 1);
            if (len > E.screencols) len = E.screencols;
            abAppend(ab, number, len);
        }

        if (filerow >= E.numrows) {
            if (E.numrows == 0 && y == E.screenrows / 3) {
                char welcome[80];
                int welcomelen =
                    snprintf(welcome, sizeof(welcome),
                             "Kilo editor -- version %s", VERSION);
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) abAppend(ab, " ", 1);
                abAppend(ab, welcome, welcomelen);
            } else {
                abAppend(ab, "~", 1);
            }
        } else {
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > E.screencols) len = E.screencols;
            char *c = &E.row[filerow].render[E.coloff];
            unsigned char *hl = &E.row[filerow].hl[E.coloff];
            int current_color = -1;
            int current_bgcolor = -1;
            int j;
            for (j = 0; j < len; j++) {
                if (iscntrl(c[j])) {
                    char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                    abAppend(ab, "\x1b[7m", 4);
                    abAppend(ab, &sym, 1);
                    abAppend(ab, "\x1b[m", 3);
                    if (current_color != -1 || current_bgcolor != -1) {
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%d;%dm",
                                            current_color, current_bgcolor);
                        abAppend(ab, buf, clen);
                    }
                } else if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        abAppend(ab, "\x1b[0m", 4);
                        current_color = -1;
                    }
                    if (current_bgcolor != -1) {
                        abAppend(ab, "\x1b[0m", 4);
                        current_bgcolor = -1;
                    }
                    abAppend(ab, &c[j], 1);
                } else {
                    int color = editorSyntaxToColor(hl[j]);
                    int bgcolor = editorBGSyntaxToColor(hl[j]);
                    if (color != current_color || bgcolor != current_bgcolor) {
                        current_color = color;
                        current_bgcolor = bgcolor;
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%d;%dm",
                                            color, bgcolor);
                        abAppend(ab, buf, clen);
                    }
                    abAppend(ab, &c[j], 1);
                }
            }
            abAppend(ab, "\x1b[0m", 4);
        }
        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2);
    }
}

void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                       E.filename ? E.filename : "[No Name]", E.numrows,
                       E.dirty ? "(modified)" : "");
    int rlen =
        snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
                 E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);
    if (len > E.screencols) len = E.screencols;
    abAppend(ab, status, len);
    while (len < E.screencols) {
        if (E.screencols - len == rlen) {
            abAppend(ab, rstatus, rlen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }
    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct abuf *ab) {
    abAppend(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols) msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        abAppend(ab, E.statusmsg, msglen);
}

void editorRefreshScreen() {
    editorScroll();

    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
             (E.rx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));
    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/*** init ***/

void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.startselectedx = -1;
    E.startselectedy = -1;
    E.endselectedx = -1;
    E.endselectedy = -1;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
    E.screenrows -= 2;
}

int main(int argc, char *argv[]) {
    initHLDB();
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        char *filename = handleArgv(argc, argv);
        editorOpenCallback(filename, '\r');
    }

    editorSetStatusMessage(
        "HELP: Ctrl | S = save | Q = quit | F = find | R = replace | O = open | G = goto | Z = undo");
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
