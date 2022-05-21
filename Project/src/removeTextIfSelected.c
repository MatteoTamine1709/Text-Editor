#include "MyVim.h"

void removeTextIfSelected() {
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
        startselectedx -=
            E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
        endselectedx -= E.options.shownumberline * SHOW_LINE_NUMBER_SPACE;
        E.cx = startselectedx;
        E.cy = startselectedy;
        printf("%d %d | ", startselectedx, endselectedx);
        for (int y = startselectedy; y <= endselectedy; ++y) {
            erow *row = &E.row[y];
            if (y == startselectedy) {
                for (int x = y == endselectedy ? endselectedx : row->rsize;
                     x > startselectedx; --x) {
                    E.cx = x;
                    editorDelChar();
                }
            } else if (y == endselectedy) {
                editorDelRow(startselectedy);
                E.cy = startselectedy;
                for (int x = endselectedx; x >= 0; --x) {
                    E.cx = x;
                    editorDelChar();
                }
            } else {
                editorDelRow(startselectedy + 1);
            }
        }
        E.cx = startselectedx;
        E.cy = startselectedy;
    }
}