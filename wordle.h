#ifndef WORDLE_H
#define WORDLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


#define WORD_LENGTH 5
#define MAX_GUESSES 6
#define MAX_WORDS 5000
#define DICT_FILE "words.txt"


#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define GRAY    "\033[1;30m"
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[1;31m"
#define CYAN    "\033[1;36m"  // <--- This was missing in your old file


void load_dictionary();
int is_valid_word(const char *word);
void to_upper_str(char *str);
void play_game();   
void solve_game();  

#endif
