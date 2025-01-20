#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int count_words(char *, int, int);
//add additional prototypes here
void reverse_string(char *, int);
void print_words(char *, int);
void replace_word(char *, char *, char *, int);



int setup_buff(char *buff, char *user_str, int len) {
    int i = 0;
    int j = 0;
    int last_was_space = 1;  // Start with true to handle leading spaces
    int actual_length = strlen(user_str);
    
    // Skip leading spaces
    while (user_str[j] == ' ' || user_str[j] == '\t') {
        j++;
    }
    
    // Process the string
    while (user_str[j] != '\0') {
        if (user_str[j] == ' ' || user_str[j] == '\t') {
            if (!last_was_space && user_str[j + 1] != '\0') {
                if (i >= len - 1) return -1;
                buff[i++] = ' ';
                last_was_space = 1;
            }
        } else {
            if (i >= len - 1 && user_str[j + 1] != '\0') return -1;
            buff[i++] = user_str[j];
            last_was_space = 0;
        }
        j++;
    }
    
    // Remove trailing space if exists
    if (i > 0 && buff[i - 1] == ' ') {
        i--;
    }
    
    // Fill the rest with dots
    while (i < len) {
        buff[i++] = '.';
    }
    
    return i;
}


void print_buff(char *buff, int len) {
    printf("Buffer:  [");
    fwrite(buff, 1, len, stdout);
    printf("]\n");
}

void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

void print_words(char *buff, int len) {
    printf("Word Print\n");
    printf("----------\n");
    
    int word_num = 0;
    int word_start = -1;
    int i;
    
    for (i = 0; i < len && buff[i] != '.'; i++) {
        if (buff[i] == ' ') {
            if (word_start != -1) {
                word_num++;
                printf("%d. ", word_num);
                for (int j = word_start; j < i; j++) {
                    putchar(buff[j]);
                }
                printf("(%d)\n", i - word_start);
                word_start = -1;
            }
        } else if (word_start == -1) {
            word_start = i;
        }
    }
    
    if (word_start != -1) {
        word_num++;
        printf("%d. ", word_num);
        for (int j = word_start; j < i && buff[j] != '.'; j++) {
            putchar(buff[j]);
        }
        printf("(%d)\n", i - word_start);
    }
    
    printf("\nNumber of words returned: %d\n", word_num);
}

void replace_word(char *buff, char *find, char *replace, int len) {
    char temp[BUFFER_SZ];
    char *match = strstr(buff, find);
    
    if (!match) {
        printf("Not Implemented!\n");
        exit(1);
    }
    
    size_t prefix_len = match - buff;
    size_t find_len = strlen(find);
    size_t replace_len = strlen(replace);
    size_t remaining_space = len - prefix_len - replace_len;
    char *suffix_start = match + find_len;
    size_t suffix_len = strlen(suffix_start);
    
    memcpy(temp, buff, prefix_len); // Copy prefix
    
    memcpy(temp + prefix_len, replace, replace_len);    // Copy replacement
    
    size_t copy_len = (suffix_len < remaining_space) ? suffix_len : remaining_space;    // Copy suffix, ensuring we don't overflow
    
    memcpy(temp + prefix_len + replace_len, suffix_start, copy_len);
    
    size_t total_copied = prefix_len + replace_len + copy_len;  // Fill rest with dots
    
    if (total_copied < len) {
        memset(temp + total_copied, '.', len - total_copied);
    }
    
    memcpy(buff, temp, len);    // Copy back to original buffer
}

void reverse_string(char *buff, int len) {
    int content_len = 0;
    while (content_len < len && buff[content_len] != '.') {
        content_len++;
    }
    
    for (int i = 0; i < content_len / 2; i++) {
        char temp = buff[i];
        buff[i] = buff[content_len - 1 - i];
        buff[content_len - 1 - i] = temp;
    }
}


int count_words(char *buff, int len, int str_len) {
    int count = 0;
    int in_word = 0;
    
    for (int i = 0; i < str_len && buff[i] != '.'; i++) {
        if (buff[i] == ' ') {
            in_word = 0;
        } else if (!in_word) {
            count++;
            in_word = 1;
        }
    }
    
    return count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]) {
    
    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      If argv[1] does not exist it means i set the program to run without the needed instructions it needs to start working correctly as it prevent the program from accessing out-of-bound or invalid memory.
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      The if statement below is used to make sure that there exist an input string which is provided along with the command option which stats that if argc is less than 3,the program was run without the needed information provided by the user
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    buff = (char *)malloc(BUFFER_SZ * sizeof(char));
    if (!buff) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(2);
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0) {
        free(buff);
        fprintf(stderr, "Provided input string is too large\n");
        exit(1);
    }

    switch (opt) {
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                fprintf(stderr, "Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            print_buff(buff, BUFFER_SZ);
            break;

        case 'r':
            reverse_string(buff, BUFFER_SZ);
            print_buff(buff, BUFFER_SZ);
            break;

        case 'w':
            print_words(buff, BUFFER_SZ);
            print_buff(buff, BUFFER_SZ);
            break;

        case 'x':
            if (argc < 5) {
                fprintf(stderr, "Error: '-x' requires two additional arguments\n");
                usage(argv[0]);
                free(buff);
                exit(1);
            }
            replace_word(buff, argv[3], argv[4], BUFFER_SZ);
            print_buff(buff, BUFFER_SZ);
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    free(buff); // Free allocated memory before exiting
    exit(0);
}



//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
//			It's a good practice because passing both the buffer pointer and its length to functions helps ensure you from reaching memory outside the set limits, which can cause crashes or create security risks.
