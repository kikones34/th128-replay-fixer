#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "th128_fix.h"
#include "th128_parse.h"


enum mode_t {
    DRAGNDROP,
    FIX,
    DECODE,
    USER,
};


void display_usage(const char *filename) {
    printf("Usage:\n");
    printf("\t%s fix file1.rpy file2.rpy ...\n", filename);
    printf("\t%s decode file1.rpy file2.rpy ...\n", filename);
    printf("\t%s user file1.rpy file2.rpy ...\n", filename);
}


int main(int argc, char *argv[]) {
    // arg parsing
	if (argc < 2) {
        display_usage(argv[0]);
		return 0;
	}

    mode_t mode;
    if (!strcmp(argv[1], "fix")) {
        if (argc < 3) {
            display_usage(argv[0]);
            return 1;
        }
        mode = FIX;
    } else if (!strcmp(argv[1], "decode")) {
        if (argc < 3) {
            display_usage(argv[0]);
            return 1;
        }
        mode = DECODE;
    } else if (!strcmp(argv[1], "user")) {
        if (argc < 3) {
            display_usage(argv[0]);
            return 1;
        }
        mode = USER;
    } else {
        mode = DRAGNDROP;
    }

    // run in specified mode
    int count = 0;    
    switch (mode) {
        case DRAGNDROP:
            for (int i = 1; i < argc; i++) {
                printf("Processing %s\n", argv[i]);
                count += th128_fix_replay_file(argv[i]);
                printf("\n");
            }

            printf("All done! Fixed %d replays.\n", count);
            system("pause"); // make the terminal stay open so user can read the output

            break;
        case FIX:
            for (int i = 2; i < argc; i++) {
                printf("Processing %s\n", argv[i]);
                count += th128_fix_replay_file(argv[i]);
                printf("\n");
            }

            printf("All done! Fixed %d replays.\n", count);

            break;
        case DECODE:
            for (int i = 2; i < argc; i++) {
                printf("Processing %s\n", argv[i]);
                count += th128_decode_replay_file(argv[i]);
                printf("\n");
            }

            printf("All done! Decoded %d replays.\n", count);

            break;
        case USER:
            for (int i = 2; i < argc; i++) {
                printf("Processing %s\n", argv[i]);
                count += th128_parse_user_data(argv[i]);
                printf("\n");
            }

            printf("All done! Parsed %d replays.\n", count);

            break;
    }

	return 0;
}
