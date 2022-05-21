#ifndef MYVIM_HPP_
#define MYVIM_HPP_

/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*** defines ***/

#define VERSION "0.0.1"
#define TAB_STOP 4
#define QUIT_TIMES 2
#define USE_SPACE_AS_TABS 0
#define SHOW_LINE_NUMBER_SPACE 4

#define CTRL_KEY(k) ((k)&0x1f)

enum editorKey {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    CTRL_ARROW_LEFT,
    CTRL_ARROW_RIGHT,
    CTRL_ARROW_UP,
    CTRL_ARROW_DOWN,
    CTRL_HOME_KEY,
    CTRL_END_KEY,
    ALT_ARROW_LEFT,
    ALT_ARROW_RIGHT,
    ALT_ARROW_UP,
    ALT_ARROW_DOWN,
    ALT_HOME_KEY,
    ALT_END_KEY,
    SHIFT_ARROW_LEFT,
    SHIFT_ARROW_RIGHT,
    SHIFT_ARROW_UP,
    SHIFT_ARROW_DOWN,
    CTRL_SHIFT_ARROW_LEFT,
    CTRL_SHIFT_ARROW_RIGHT,
    CTRL_SHIFT_ARROW_UP,
    CTRL_SHIFT_ARROW_DOWN,
    ALT_SHIFT_ARROW_LEFT,
    ALT_SHIFT_ARROW_RIGHT,
    ALT_SHIFT_ARROW_UP,
    ALT_SHIFT_ARROW_DOWN,
};

enum editorHighlight {
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_PREPROC,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH,
    HL_BG_OPEN_CLOSE_TAG = 1 << 4,
    HL_BG_SELECTED_TEXT,
};

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)
#define HL_HIGHLIGHT_PREPROC (1 << 2)

/*** data ***/

struct editorSyntax {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    char *preproc_start;
    int flags;
};

typedef struct erow {
    int idx;
    int size;
    int rsize;
    char *chars;
    char *render;
    unsigned char *hl;
    int hl_open_comment;
} erow;

struct optionList {
    int shownumberline;
};

struct change {
    int yRow;
    int isNewLine;
    int isRepeat;
    int isBackspace;
    int size;
    int rsize;
    int cx;
    char *chars;
    struct change *prev;
    struct change *next;
};

typedef struct editorConfig_s {
    int cx, cy;
    int rx;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    int numrows;
    char hoverchar;
    int startselectedx;
    int startselectedy;
    int endselectedx;
    int endselectedy;
    char **copybuffer;
    int *copylens;
    int copynbline;
    int openx;
    int openy;
    int closex;
    int closey;
    erow *row;
    int dirty;
    char *filename;
    char statusmsg[100];
    time_t statusmsg_time;
    struct editorSyntax *syntax;
    struct termios orig_termios;
    struct optionList options;
    struct change *changeStack;
} editorConfig;

editorConfig E;

/*** syntax database ***/

extern struct editorSyntax *HLDB;
void initHLDB();
#define HLDB_ENTRIES 1  //(sizeof(HLDB) / sizeof(HLDB[0]))

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT \
    { NULL, 0 }

/*** prototypes ***/

void editorProcessKeypress();
void editorMoveCursor(int key);
char *editorPrompt(char *prompt, void (*callback)(char *, int));

void die(const char *s);
void disableRawMode();
void enableRawMode();
int editorReadKey();
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
int is_separator(int c);
void editorUpdateSyntax(erow *row, int y);
int editorSyntaxToColor(int hl);
int editorBGSyntaxToColor(int hl);
void editorSelectSyntaxHighlight();
int editorRowCxToRx(erow *row, int cx);
int editorRowRxToCx(erow *row, int rx);
void editorUpdateRow(erow *row, int y);
void editorInsertRow(int at, char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorRowInsertChar(erow *row, int at, int c, int y);
void editorRowAppendString(erow *row, char *s, size_t len, int y);
void editorRowDelChar(erow *row, int at, int y);
void editorInsertChar(int c);
void editorInsertNewline();
void editorDelChar();
char *editorRowsToString(int *buflen);
void editorSave();
void editorFindCallback(char *query, int key);
void editorFind();
void editorReplaceCallback(char *query, int key);
void editorReplace();
void editorOpenCallback(char *filename, int key);
void editorOpen();
void editorGoToLineCallback(char *query, int key);
void editorGoToLine();
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void editorScroll();
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);
void initEditor();
void syntaxOpenCloseTags();
int isCharIn(char c, char *str, int size);
void refreshAllSyntax();
void syntaxSelectedText();
void editorCopy();
void editorPaste();
void editorUndo();
void removeTextIfSelected();
char *handleArgv(int argc, char **argv);
void editChangeStack(int key, int y);
void printStackChange();
void swapPointerChange(struct change **a, struct change **b);
void freeChange(struct change *ch);

#endif /* MYVIM_HPP_ */