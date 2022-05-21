#include "MyVim.h"

const struct option lineNumber = {.name = "linenumber", .has_arg = no_argument};

char *handleArgv(int argc, char **argv) {
    const struct option options[] = {lineNumber, {0, 0, 0, 0}};
    int thisoptionoptind = optind ? optind : 1;
    int optionindex = 0;
    int c;
    while (1) {
        c = getopt_long(argc, argv, "l", options, &optionindex);
        if (c == -1) break;
        switch (c) {
            case 'l':
            case 0:
                if (strcmp(options[optionindex].name, "linenumber") == 0)
                    E.options.shownumberline = 1;
                break;

            case '?':
                break;
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
                break;
        }
    }
    if (optind < argc) return argv[optind];
}