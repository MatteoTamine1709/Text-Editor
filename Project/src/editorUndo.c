#include "MyVim.h"

void editorUndo() {
    if (E.changeStack) {
        do {
            struct change *head = E.changeStack;
            while (E.changeStack->isNewLine) {
                E.cy = E.changeStack->yRow + 1;
                E.cx = 0;
                editorDelChar();
                if (E.changeStack->prev) {
                    swapPointerChange(&E.changeStack, &E.changeStack->prev);
                    freeChange(E.changeStack->next);
                } else {
                    freeChange(E.changeStack);
                    E.changeStack = NULL;
                }
                if (E.changeStack == NULL) return;
                if (!E.changeStack->isRepeat) return;
                if (!E.changeStack->isNewLine) break;
            }
            if (E.changeStack->isBackspace)
                editorInsertRow(E.changeStack->yRow, E.changeStack->chars, E.changeStack->size);
            else {
                free((&E.row[E.changeStack->yRow])->chars);
                (&E.row[E.changeStack->yRow])->chars = strdup(E.changeStack->chars);
                (&E.row[E.changeStack->yRow])->size = E.changeStack->size;
                (&E.row[E.changeStack->yRow])->rsize = E.changeStack->rsize;
                editorUpdateRow(&E.row[E.changeStack->yRow], E.changeStack->yRow);
            }
            E.cx = E.changeStack->cx;
            E.cy = E.changeStack->yRow;
            if (E.changeStack->prev) {
                E.changeStack = E.changeStack->prev;
                freeChange(E.changeStack->next);
            } else {
                freeChange(E.changeStack);
                E.changeStack = NULL;
            }
        } while (E.changeStack && E.changeStack->isRepeat);
    }
}