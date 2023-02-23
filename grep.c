#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

int main(int argc, char *argv[]) {
    // Initialize flags
    bool ignore_case = false;
    bool invert_match = false;
    bool line_numbers = false;
    bool match_entire_lines = false;
    bool silent = false;
    bool word_regexp = false;
    
    // Process command-line arguments
    int c;
    while ((c = getopt(argc, argv, "cinvxLlw")) != -1) {
        switch (c) {
            case 'c':
                // Count matching lines instead of printing them
                silent = true;
                break;
            case 'i':
                // Ignore case distinctions in both the PATTERN and the input files
                ignore_case = true;
                break;
            case 'n':
                // Prefix each line of output with the 1-based line number within its input file
                line_numbers = true;
                break;
            case 'v':
                // Select non-matching lines
                invert_match = true;
                break;
            case 'x':
                // Select only those matches that exactly match the whole line
                match_entire_lines = true;
                break;
            case 'L':
                // Only print filenames of non-matching files
                silent = true;
                invert_match = true;
                break;
            case 'l':
                // Only print filenames of matching files
                silent = true;
                break;
            case 'w':
                // Select only those lines containing matches that form whole words
                word_regexp = true;
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
    
    // Parse regular expression pattern from command line
    if (optind >= argc) {
        fprintf(stderr, "grep: missing pattern argument\n");
        return 1;
    }
    char *pattern_str = argv[optind];
    optind++;
    
    // Compile regular expression pattern
    int regex_flags = REG_EXTENDED;
    if (ignore_case) {
        regex_flags |= REG_ICASE;
    }
    regex_t pattern;
    int regex_err = regcomp(&pattern, pattern_str, regex_flags);
    if (regex_err != 0) {
        char regex_err_buf[1024];
        regerror(regex_err, &pattern, regex_err_buf, sizeof(regex_err_buf));
        fprintf(stderr, "grep: regex compilation failed: %s\n", regex_err_buf);
        return 1;
    }
    
    // Process input files
    int match_count = 0;
    for (; optind < argc; optind++) {
        char *filename = argv[optind];
        FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            fprintf(stderr, "grep: could not open file: %s\n", filename);
            continue;
        }
        char line[BUFSIZ];
        int line_number = 0;
        while (fgets(line, BUFSIZ, fp) != NULL) {
            line_number++;
            if (match_entire_lines) {
                // Match against entire line
              
               int result = regexec(&pattern, line, 0, NULL, 0);
            if (invert_match ? result == REG_NOMATCH : result == 0) {
                if (!silent) {
                    printf("%s", line);
                }
                match_count++;
            }
        } else {
            // Match against individual words
            char *token;
            token = strtok(line, " \t\n");
            while (token != NULL) {
                int result = regexec(&pattern, token, 0, NULL, 0);
                if ((word_regexp && strlen(token) == strlen(pattern_str) && 
                    (invert_match ? result == REG_NOMATCH : result == 0)) ||
                    (!word_regexp && (invert_match ? result == REG_NOMATCH : result == 0))) {
                    if (line_numbers) {
                        printf("%d:", line_number);
                    }
                    if (!silent) {
                        printf("%s ", token);
                    }
                    match_count++;
                }
                token = strtok(NULL, " \t\n");
            }
            if (!silent && match_count > 0) {
                printf("\n");
            }
        }
    }
    fclose(fp);
    if (silent && match_count == 0 && !invert_match) {
        printf("%s\n", filename);
    } else if (silent && match_count > 0 && invert_match) {
        printf("%s\n", filename);
    }
}

// Clean up
regfree(&pattern);

return match_count == 0 ? 1 : 0;
