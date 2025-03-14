#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Only allowed functions used: memset, memcpy, printf, exit

#define BUFFER_SZ 50

// Function Prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);
int  count_words(char *, int, int);
void reverse_string(char *, int);
int  print_words(char *, int, int);
int  replace_word(char *, int, int *, char *, char *);

// Extra Credit helper functions
int custom_strlen(char *);         // our own strlen implementation (no array notation)
char *find_substr(char *, int, char *);  // our own strstr implementation using pointers

/*
 * custom_strlen: calculates the length of a C string using pointer arithmetic.
 */
int custom_strlen(char *s) {
    int len = 0;
    while (*s != '\0') {
        len++;
        s++;
    }
    return len;
}

/*
 * setup_buff: Copies over every non-whitespace character from user_str into buff,
 * compressing consecutive whitespace (spaces/tabs) into a single space.
 * Leading and trailing whitespace are removed.
 * After copying the effective text (which must be <= len bytes),
 * the remainder of buff is filled with '.' characters.
 *
 * Returns:
 *   -1  if the effective user string is too long to fit in buff.
 *   -2  for any other error condition (none implemented here).
 *   Otherwise, returns the effective length of the user-supplied string.
 */
int setup_buff(char *buff, char *user_str, int len) {
    char *p = user_str;
    char *q = buff;
    int effective_len = 0;

    // Skip leading whitespace (space or tab)
    while (*p != '\0' && (*p == ' ' || *p == '\t')) {
        p++;
    }
    // Process characters until end-of-string
    while (*p != '\0') {
        if (*p == ' ' || *p == '\t') {
            // If the last written character is not a space, add one (compress multiple whitespace)
            if (q != buff && *(q - 1) != ' ') {
                if (effective_len < len) {
                    *q = ' ';
                    q++;
                    effective_len++;
                } else {
                    return -1;  // too long
                }
            }
            // Skip over any additional whitespace characters
            while (*p == ' ' || *p == '\t') {
                p++;
            }
        } else {
            // Copy non-whitespace character
            if (effective_len < len) {
                *q = *p;
                q++;
                effective_len++;
            } else {
                return -1;  // too long
            }
            p++;
        }
    }
    // Remove trailing space if present
    if (effective_len > 0 && *(q - 1) == ' ') {
        q--;
        effective_len--;
    }
    // Fill the remainder of the buffer with '.' characters up to BUFFER_SZ
    while (effective_len < len) {
        *q = '.';
        q++;
        effective_len++;
    }
    // Return the length of the effective (user-supplied) string (i.e. before filling dots)
    // Note: effective text length is (q - buff) minus the dots added. We saved the count before filling dots.
    // Here, we calculate it as the distance from buff to the first dot.
    int ret = q - buff;
    // Since we overrode ret by filling dots, we can recompute the effective length by:
    // effective text length = total written (which is len) - number of dots (which is len - (q - buff_before_fill)).
    // Instead, we use a separate variable.
    // We'll modify the approach: first copy effective text then fill dots.
    // For that, we need to record the effective text length.
    // Let effective_text = (q - buff) before filling dots.
    int effective_text = effective_len - (len - (q - buff));
    // However, since we already incremented effective_len while filling dots,
    // the effective text length is actually (q - buff) before the dot fill loop.
    // To correct this, we rework the dot-fill step:
    {
        int current = q - buff; // current count after copying effective text
        int diff = len - current;
        q = buff + current;
        effective_len = current;  // effective text length
        while (diff > 0) {
            *q = '.';
            q++;
            diff--;
        }
        return effective_len;
    }
}

/*
 * count_words: Counts the number of words in buff.
 * Processes only the first str_len characters (the effective text).
 * A word is defined as a sequence of non-space characters.
 */
int count_words(char *buff, int len, int str_len) {
    int count = 0;
    int in_word = 0;
    char *p = buff;
    int i = 0;
    while (i < str_len) {
        if (*p != ' ') {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
        p++;
        i++;
    }
    return count;
}

/*
 * reverse_string: Reverses the effective text (first str_len characters in buff) in place.
 */
void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff + str_len - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

/*
 * print_words: Prints each word (and its length) from the effective text in buff.
 * Words are assumed to be separated by a single space.
 * Returns the count of words printed.
 */
int print_words(char *buff, int buff_len, int str_len) {
    int word_count = 0;
    int char_count = 0;
    int at_start = 1;
    char *p = buff;
    int word_number = 1;
    
    printf("Word Print\n----------\n");
    
    while (p < buff + str_len) {
        if (*p != ' ') {
            if (at_start) {
                // Start of a new word: print word number and prepare to print characters
                printf("%d. ", word_number);
                word_count++;
                at_start = 0;
                char_count = 0;
            }
            putchar(*p);
            char_count++;
        } else {
            if (!at_start) {
                // End of a word: print the word's character count
                printf(" (%d)\n", char_count);
                word_number++;
                at_start = 1;
            }
        }
        p++;
    }
    // If the last character was not a space, finish the last word's line.
    if (!at_start) {
        printf(" (%d)\n", char_count);
    }
    return word_count;
}

/*
 * find_substr: Searches for the first occurrence of needle in haystack
 * (only within the first haystack_len characters) and returns a pointer to it.
 * Returns NULL if not found.
 */
char *find_substr(char *haystack, int haystack_len, char *needle) {
    int needle_len = custom_strlen(needle);
    if (needle_len == 0 || haystack_len < needle_len)
        return NULL;
    char *h_end = haystack + haystack_len - needle_len + 1;
    char *p = haystack;
    while (p < h_end) {
        char *q = p;
        char *r = needle;
        while (*r != '\0' && *q == *r) {
            q++;
            r++;
        }
        if (*r == '\0')
            return p;  // Found match
        p++;
    }
    return NULL;
}

/*
 * replace_word: Searches for the first occurrence of 'search' in the effective text of buff,
 * and replaces it with 'replace'. If the replacement changes the effective length,
 * the tail of the string is shifted accordingly.
 *
 * Parameters:
 *   buff      - pointer to the internal buffer (size = buff_len)
 *   buff_len  - total size of the buffer
 *   str_len_ptr - pointer to the effective text length (will be updated if changed)
 *   search    - string to search for (case sensitive)
 *   replace   - replacement string
 *
 * Returns 0 on success, negative value on error.
 */
int replace_word(char *buff, int buff_len, int *str_len_ptr, char *search, char *replace) {
    int str_len = *str_len_ptr;
    int search_len = custom_strlen(search);
    int replace_len = custom_strlen(replace);
    char *pos = find_substr(buff, str_len, search);
    if (!pos) {
        printf("Search string not found.\n");
        return 1;  // No replacement done
    }
    int offset = pos - buff;
    int new_len = str_len - search_len + replace_len;
    if (new_len > buff_len) {
        printf("Error: replacement causes buffer overflow.\n");
        return -1;
    }
    if (replace_len != search_len) {
        // Shift the tail of the effective text to make room (or remove extra space)
        char *tail_start = buff + offset + search_len;
        int tail_len = str_len - (offset + search_len);
        if (replace_len > search_len) {
            // Shift right: start from the end and move backwards
            char *tail_end = buff + str_len;
            char *dest = tail_end + (replace_len - search_len);
            while (tail_len > 0) {
                *(dest - 1) = *(tail_end - 1);
                tail_end--;
                dest--;
                tail_len--;
            }
        } else {
            // Shift left: move tail to the left by (search_len - replace_len) positions
            char *src = tail_start;
            char *dest = src - (search_len - replace_len);
            int i = 0;
            while (i < tail_len) {
                *(dest) = *(src);
                dest++;
                src++;
                i++;
            }
        }
    }
    // Copy the replacement string into the position of the found substring
    char *r = replace;
    char *p_dest = buff + offset;
    while (*r != '\0') {
        *p_dest = *r;
        p_dest++;
        r++;
    }
    // Update effective length and fill remainder with '.' characters
    *str_len_ptr = new_len;
    char *p_fill = buff + new_len;
    while ((p_fill - buff) < buff_len) {
        *p_fill = '.';
        p_fill++;
    }
    // Print the modified effective string
    printf("Modified String: ");
    char *p_print = buff;
    int count = 0;
    while (count < new_len) {
        putchar(*p_print);
        p_print++;
        count++;
    }
    putchar('\n');
    return 0;
}

/*
 * usage: Prints the correct command line usage.
 */
void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

/*
 * print_buff: Prints the entire buffer (all BUFFER_SZ characters).
 */
void print_buff(char *buff, int len) {
    printf("Buffer:  ");
    for (int i = 0; i < len; i++) {
        putchar(*(buff + i));
    }
    putchar('\n');
}

/*
 * main: Processes command-line arguments and calls the appropriate function.
 */
int main(int argc, char *argv[]) {

    char *buff;             // placeholder for the internal buffer
    char *input_string;     // holds the string provided by the user on cmd line
    char opt;               // used to capture user option from cmd line
    int rc;                 // used for return codes
    int user_str_len;       // effective length of user supplied string

    /*
     * TODO #1: WHY IS THIS SAFE?
     * We first check that argc is at least 2 and that argv[1] begins with '-'.
     * This ensures that argv[1] exists before we dereference it.
     */
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = *(argv[1] + 1);   // get the option flag

    // handle the help flag and then exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    /*
     * TODO #2: PURPOSE OF THE FOLLOWING IF STATEMENT
     * This check ensures that a user-supplied input string (argv[2]) is provided.
     * Without it, the program would not have any string to process.
     */
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; // capture the user input string

    /*
     * TODO #3: ALLOCATE SPACE FOR THE BUFFER
     * We allocate BUFFER_SZ bytes using malloc. If malloc fails, we exit with code 99.
     */
    buff = (char *)malloc(BUFFER_SZ);
    if (buff == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(99);
    }

    // Set up the internal buffer by copying and processing the user input.
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0) {
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(2);
    }

    // Process the option flag provided by the user
    switch (opt) {
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        case 'r':
            // Reverse string option: reverse the effective text in place.
            reverse_string(buff, user_str_len);
            printf("Reversed String: ");
            {
                int count = 0;
                char *p = buff;
                while (count < user_str_len) {
                    putchar(*p);
                    p++;
                    count++;
                }
                putchar('\n');
            }
            break;

        case 'w':
            // Word print option: print each word and its length.
            print_words(buff, BUFFER_SZ, user_str_len);
            break;

        case 'x':
            // String search and replace option: requires exactly 5 arguments.
            if (argc != 5) {
                usage(argv[0]);
                free(buff);
                exit(1);
            }
            rc = replace_word(buff, BUFFER_SZ, &user_str_len, argv[3], argv[4]);
            if (rc < 0) {
                printf("Error in string replacement, rc = %d\n", rc);
                free(buff);
                exit(3);
            }
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    /*
     * TODO #7: WHY PASS BOTH THE BUFFER POINTER AND ITS LENGTH?
     * Providing both the pointer and the length to helper functions is good practice because:
     *  - The internal buffer is not null-terminated and may contain extra characters (like the '.' fillers),
     *    so knowing its total size prevents reading or writing out-of-bounds.
     *  - It makes the functions more robust and safer against buffer overflows,
     *    especially when processing strings that are not standard C strings.
     */
    
    print_buff(buff, BUFFER_SZ);
    
    // TODO #6: Free the allocated buffer before exiting.
    free(buff);
    exit(0);
}
