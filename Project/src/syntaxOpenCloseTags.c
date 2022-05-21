#include "MyVim.h"

/*** syntax open close tags ***/

int isCharIn(char c, char *str, int size) {
    for (int i = 0; i < size; ++i)
        if (c == str[i]) return i;
    return -1;
}

void syntaxOpenCloseTags() {
    char openingtags[] = "{([";
    char closingtags[] = "})]";

    // Look for start
    int opencount[7] = {0};
    char openingtagidx = -1;
    int offsetOpen = 0;
    if (isCharIn(E.hoverchar, closingtags, sizeof(closingtags)) != -1)
        offsetOpen = -1;
    int startx = -1;
    int starty = -1;
    for (int i = E.cy; i >= 0 && openingtagidx == -1; --i) {
        erow *row = &E.row[i];
        for (int j = (i == E.cy ? E.cx + offsetOpen : row->size); j >= 0; --j) {
            int idxOpenChar =
                isCharIn(row->chars[j], openingtags, sizeof(openingtags));
            int idxCloseChar =
                isCharIn(row->chars[j], closingtags, sizeof(closingtags));
            if (idxOpenChar != -1) opencount[idxOpenChar]++;
            if (idxCloseChar != -1) opencount[idxCloseChar]--;
            if (idxOpenChar != -1) {
                if (opencount[idxOpenChar] == 1) {
                    openingtagidx = isCharIn(row->chars[j], openingtags,
                                             sizeof(openingtags));
                    startx = j;
                    starty = i;
                    break;
                }
            }
        }
    }

    // Loof for end
    int closecount[sizeof(closingtags)] = {0};
    int offsetClose = 0;
    if (isCharIn(E.hoverchar, openingtags, sizeof(openingtags)) != -1)
        offsetClose = 1;
    char closingtagidx = -1;
    int endx = -1;
    int endy = -1;
    for (int i = E.cy; i < E.numrows && closingtagidx == -1; ++i) {
        erow *row = &E.row[i];
        for (int j = (i == E.cy ? E.cx + offsetClose : 0); j < row->size; ++j) {
            int idxOpenChar =
                isCharIn(row->chars[j], openingtags, sizeof(openingtags));
            int idxCloseChar =
                isCharIn(row->chars[j], closingtags, sizeof(closingtags));
            if (idxOpenChar != -1) closecount[idxOpenChar]++;
            if (idxCloseChar != -1) closecount[idxCloseChar]--;
            if (idxCloseChar != -1) {
                if (closecount[idxCloseChar] == -1) {
                    closingtagidx = isCharIn(row->chars[j], closingtags,
                                             sizeof(closingtags));
                    endx = j;
                    endy = i;
                    break;
                }
            }
        }
    }

    if (startx != -1 && starty != -1 && endx != -1 && endy != -1) {
        erow *startrow = &E.row[starty];
        erow *endrow = &E.row[endy];
        if (isCharIn(startrow->chars[startx], openingtags,
                     sizeof(openingtags)) ==
            isCharIn(endrow->chars[endx], closingtags, sizeof(closingtags))) {
            startrow->hl[startx] |= HL_BG_OPEN_CLOSE_TAG;
            endrow->hl[endx] |= HL_BG_OPEN_CLOSE_TAG;
        }
    }
}