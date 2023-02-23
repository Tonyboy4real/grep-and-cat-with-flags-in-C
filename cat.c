#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Initialize flags
    bool show_line_number = false;
    bool number_nonempty_lines = false;
    bool show_dollar_sign = false;
    bool squeeze_blank_lines = false;
    bool show_tabs = false;
    bool show_ends = false;
    
    // Process command-line arguments
    int c;
    while ((c = getopt(argc, argv, "bEnstT")) != -1) {
        switch (c) {
            case 'b':
                number_nonempty_lines = true;
                break;
            case 'E':
                show_ends = true;
                break;
            case 'n':
                show_line_number = true;
                break;
            case 's':
                squeeze_blank_lines = true;
                break;
            case 't':
                show_tabs = true;
                break;
            case 'T':
                show_tabs = true;
                show_ends = true;
                break;
            case '?':
                if (optopt == 'c') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                return 1;
            default:
                abort();
        }
    }
    
    // Read lines from standard input and print them out with appropriate options
    char line[BUFSIZ];
    int line_number = 0;
    bool last_line_blank = false;
    while (fgets(line, BUFSIZ, stdin) != NULL) {
        bool is_blank_line = (line[0] == '\n');
        line_number += !squeeze_blank_lines || (!is_blank_line && !last_line_blank);
        last_line_blank = is_blank_line;
        
        // Print line number, if requested
        if (show_line_number) {
            printf("%6d  ", line_number);
        }
        
        // Number non-empty lines, if requested
        if (number_nonempty_lines && !is_blank_line) {
            printf("%6d  ", line_number);
        }
        
        // Print line with appropriate options
        if (show_ends) {
            int len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '$';
                printf("%s", line);
            } else {
                printf("%s$", line);
            }
        } else {
            char *formatted_line = line;
            if (show_tabs) {
                formatted_line = (char *) malloc(strlen(line) * 3 + 1);
                int j = 0;
                for (int i = 0; line[i] != '\0'; i++) {
                    if (line[i] == '\t') {
                        formatted_line[j++] = '^';
                        formatted_line[j++] = 'I';
                    } else {
                        formatted_line[j++] = line[i];
                    }
                }
                formatted_line[j] = '\0';
            }
            printf("%s", formatted_line);
            if (formatted_line != line) {
                free(formatted_line);
            }
        }
    }
    
    return 0;
}
