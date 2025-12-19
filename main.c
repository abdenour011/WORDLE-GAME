#include "wordle.h"

WordList load_dictionary(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Error loading dictionary"); exit(1); }

    WordList wl;
    wl.words = malloc(sizeof(char*) * 20000); 
    wl.count = 0;

    char buffer[10];
    while (fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\n")] = 0; 
        if (strlen(buffer) == WORD_LEN) {
            wl.words[wl.count] = strdup(buffer);
            wl.count++;
        }
    }
    fclose(f);
    return wl;
}

void get_feedback(const char *target, const char *guess, int *feedback) {
    int target_counts[26] = {0};
    int guess_counts[26] = {0};

  
    for (int i = 0; i < WORD_LEN; i++) {
        target_counts[target[i] - 'a']++;
        feedback[i] = GRAY; 
    }
    
    for (int i = 0; i < WORD_LEN; i++) {
        if (guess[i] == target[i]) {
            feedback[i] = GREEN;
            target_counts[target[i] - 'a']--; 
            guess_counts[i] = 1; 
        }
    }
   
    for (int i = 0; i < WORD_LEN; i++) {
        if (feedback[i] == GREEN) continue; 
        
        int char_idx = guess[i] - 'a';
        if (target_counts[char_idx] > 0) {
            feedback[i] = YELLOW;
            target_counts[char_idx]--;
        }
    }
}


WordList filter_candidates(WordList *candidates, const char *last_guess, const int *actual_feedback) {
    WordList new_list;
    new_list.words = malloc(sizeof(char*) * candidates->count);
    new_list.count = 0;

    int sim_feedback[WORD_LEN];

    for (int i = 0; i < candidates->count; i++) {
        
        get_feedback(candidates->words[i], last_guess, sim_feedback);

        bool match = true;
        for (int k = 0; k < WORD_LEN; k++) {
            if (sim_feedback[k] != actual_feedback[k]) {
                match = false;
                break;
            }
        }

        if (match) {
            new_list.words[new_list.count++] = candidates->words[i]; 
        }
    }
    return new_list;
}

void solve_wordle(WordList *full_list, const char *target, bool debug_mode) {
    
    WordList candidates;
    candidates.words = malloc(sizeof(char*) * full_list->count);
    memcpy(candidates.words, full_list->words, sizeof(char*) * full_list->count);
    candidates.count = full_list->count;

    char guess[WORD_LEN + 1];
    int feedback[WORD_LEN];
    int attempts = 0;
    
    strcpy(guess, "slate"); 

    if (debug_mode) printf("\n[SOLVER] Target: %s\n", target);

    while (attempts < MAX_GUESSES) {
        attempts++;
        
        get_feedback(target, guess, feedback);

        if (debug_mode) {
            printf("Guess %d: %s | Remaining Candidates: %d\n", attempts, guess, candidates.count);
        }
       
        bool win = true;
        for(int i=0; i<WORD_LEN; i++) if(feedback[i] != GREEN) win = false;
        
        if (win) {
            printf("Solver WON in %d attempts! Word: %s\n", attempts, guess);
            free(candidates.words);
            return;
        }
       
        WordList next_candidates = filter_candidates(&candidates, guess, feedback);
        free(candidates.words);
        candidates = next_candidates;
       
        if (candidates.count > 0) {
            strcpy(guess, candidates.words[0]); 
            
        } else {
            printf("Error: No words match the feedback.\n");
            break;
        }
    }
    printf("Solver LOST. Target was: %s\n", target);
    free(candidates.words);
}


int main() {
    srand(time(NULL));
    WordList dictionary = load_dictionary(DICT_FILE);
    printf("Loaded %d words.\n", dictionary.count);

    if (dictionary.count == 0) return 1;
  
    char *target = dictionary.words[rand() % dictionary.count];

    printf("1. Play Human vs CPU\n2. Run AI Solver\nChoice: ");
    int choice;
    scanf("%d", &choice);

    if (choice == 1) {
        
        char guess[10];
        int feedback[WORD_LEN];
        for (int i = 0; i < MAX_GUESSES; i++) {
            printf("Guess %d/%d: ", i+1, MAX_GUESSES);
            scanf("%s", guess);            
            
            get_feedback(target, guess, feedback);
            
            printf("Result: ");
            int green_count = 0;
            for(int k=0; k<WORD_LEN; k++) {
                if(feedback[k] == GREEN) { printf("G "); green_count++; }
                else if(feedback[k] == YELLOW) printf("Y ");
                else printf(". ");
            }
            printf("\n");

            if (green_count == 5) {
                printf("You Win!\n");
                return 0;
            }
        }
        printf("You Lose. Word was: %s\n", target);
    } 
    else {
        solve_wordle(&dictionary, target, true);
    }

    for(int i=0; i<dictionary.count; i++) free(dictionary.words[i]);
    free(dictionary.words);
    
    return 0;
}
