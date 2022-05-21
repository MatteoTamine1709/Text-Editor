#include "MyVim.h"

void syntaxSelectedText() {
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

        for (int y = startselectedy; y <= endselectedy; ++y) {
            erow *row = &E.row[y];
            for (int x = y == startselectedy ? startselectedx : 0;
                 x < row->rsize &&
                 x < (y == startselectedy ? startselectedx : 0) +
                         (y == endselectedy ? endselectedx : row->rsize) -
                         (y == startselectedy ? startselectedx : 0) +
                         (startselectedy == endselectedy);
                 ++x) {
                row->hl[x] |= HL_BG_OPEN_CLOSE_TAG;
            }
        }
    }
}