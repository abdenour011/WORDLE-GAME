#ifndef WORDLE_H
#define WORDLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define WORD_LEN 5
#define MAX_GUESSES 6
#define DICT_FILE "words.txt"

// Feedback constants
#define GRAY 0
#define YELLOW 1
#define GREEN 2

typedef struct {
    char **words;
    int count;
} WordList;

// Function Prototypes
WordList load_dictionary(const char *filename);
void free_dictionary(WordList *wl);
void get_feedback(const char *target, const char *guess, int *feedback);
void print_feedback(const char *guess, const int *feedback);
bool is_valid_guess(WordList *wl, const char *guess);
void solve_wordle(WordList *full_list, const char *target, bool debug_mode);

#endif
