#include "MyVim.h"

void swapPointerChange(struct change **a, struct change **b) {
    struct change *tmp = *a;
    *a = *b;
    *b = tmp;
}

void printStackChange() {
    struct change *head = E.changeStack;
    printf("=> ");
    while (head != NULL) {
        if (head->chars != NULL)
            printf(
                "[%p, '%s', NL: %d, Rep: %d, BS: %d, Y: %d, S: %d, RS: %d], ",
                head, head->chars, head->isNewLine, head->isRepeat,
                head->isBackspace, head->yRow, head->size, head->rsize);
        head = head->prev;
    }
}

void pushChangeStack(struct change *newChange) {
    if (!E.changeStack) {
        E.changeStack = newChange;
        return;
    }
    E.changeStack->next = newChange;
    newChange->prev = E.changeStack;
    swapPointerChange(&E.changeStack, &newChange);
}

void freeChange(struct change *ch) {
    if (ch) {
        free(ch->chars);
        free(ch);
    }
}

void initChange(struct change *newChange, int y) {
    newChange->prev = NULL;
    if (E.changeStack) newChange->prev = E.changeStack->prev;
    newChange->next = NULL;
    newChange->yRow = y;
    newChange->isNewLine = 0;
    newChange->isRepeat = 0;
    newChange->isBackspace = 0;
    newChange->chars = strdup((&E.row[y])->chars);
    newChange->size = (&E.row[y])->size;
    newChange->rsize = (&E.row[y])->rsize;
    newChange->cx = E.cx;
}

void editChangeStack(int key, int y) {
    static int lastType = -1;
    static int currType = -1;
    switch (key) {
        case CTRL_KEY('z'):
            currType = CTRL_KEY('z');
            break;
        case '\r':
        case '\t':
            currType = '\r';
            break;
        case ' ':
        case BACKSPACE:
        case CTRL_KEY('h'):
            currType = BACKSPACE;
            break;
        case DEL_KEY:
            currType = DEL_KEY;
            break;
        default:
            currType = 0;
            break;
    }

    if (currType == BACKSPACE && E.cx == 0) {
        lastType = currType;
        struct change *newChange = malloc(sizeof(struct change));
        initChange(newChange, y);
        newChange->isRepeat = 1;
        newChange->isBackspace = 1;
        pushChangeStack(newChange);
        newChange = malloc(sizeof(struct change));
        initChange(newChange, y - 1);
        newChange->isRepeat = 0;
        pushChangeStack(newChange);
    }

    if (currType == DEL_KEY && E.cx == (&E.row[y])->size) {
        lastType = currType;
        struct change *newChange = malloc(sizeof(struct change));
        initChange(newChange, y + 1);
        newChange->isRepeat = 1;
        pushChangeStack(newChange);
        newChange = malloc(sizeof(struct change));
        initChange(newChange, y);
        newChange->isRepeat = 0;
        newChange->isBackspace = 1;
        pushChangeStack(newChange);
    }
    if ((currType != lastType || currType == '\r')) {
        lastType = currType;
        if (currType == CTRL_KEY('z')) return;
        struct change *newChange = malloc(sizeof(struct change));
        initChange(newChange, y);
        newChange->isNewLine = key == '\r';
        newChange->isRepeat = key == ' ';
        pushChangeStack(newChange);
    }
}