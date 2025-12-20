#include "wordle.h"

// Global dictionary
char dictionary[MAX_WORDS][WORD_LENGTH + 1];
int dict_size = 0;

// Helper: Convert to Uppercase
void to_upper_str(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Helper: Load Dictionary
void load_dictionary() {
    FILE *file = fopen(DICT_FILE, "r");
    if (!file) {
        printf(RED "Error: Could not open %s.\n" RESET, DICT_FILE);
        exit(1);
    }
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), file) && dict_size < MAX_WORDS) {
        buffer[strcspn(buffer, "\n")] = 0;
        buffer[strcspn(buffer, "\r")] = 0;
        if (strlen(buffer) == WORD_LENGTH) {
            strcpy(dictionary[dict_size], buffer);
            to_upper_str(dictionary[dict_size]);
            dict_size++;
        }
    }
    fclose(file);
}

// Helper: Check if word exists in dictionary
int is_valid_word(const char *word) {
    for (int i = 0; i < dict_size; i++) {
        if (strcmp(dictionary[i], word) == 0) {
            return 1;
        }
    }
    return 0;
}

// Helper: Check colors (used by Game and Solver)
// Returns an array: 2=Green, 1=Yellow, 0=Gray
void calculate_feedback(const char *guess, const char *target, int *result_colors) {
    int target_freq[26] = {0};
    for (int i = 0; i < WORD_LENGTH; i++) target_freq[target[i] - 'A']++;

    // Reset colors
    for(int i=0; i<WORD_LENGTH; i++) result_colors[i] = 0;

    // Pass 1: Green
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i]) {
            result_colors[i] = 2;
            target_freq[guess[i] - 'A']--;
        }
    }
    // Pass 2: Yellow
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (result_colors[i] != 2 && target_freq[guess[i] - 'A'] > 0) {
            result_colors[i] = 1;
            target_freq[guess[i] - 'A']--;
        }
    }
}

// --- OPTION 1: PLAYER MODE ---
void play_game() {
    int random_index = rand() % dict_size;
    char target[WORD_LENGTH + 1];
    strcpy(target, dictionary[random_index]);

    int attempts = 0;
    int won = 0;
    char guess[100];
    int colors[WORD_LENGTH];

    printf("\n" BOLD "--- PLAYER MODE ---" RESET "\n");
    printf("Guess the 5-letter word! (Must be in dictionary)\n");

    while (attempts < MAX_GUESSES && !won) {
        printf("\nAttempt %d/%d > ", attempts + 1, MAX_GUESSES);
        if (!fgets(guess, sizeof(guess), stdin)) break;
        
        guess[strcspn(guess, "\n")] = 0;
        guess[strcspn(guess, "\r")] = 0;
        to_upper_str(guess);

        // VALIDATION 1: Length
        if (strlen(guess) != WORD_LENGTH) {
            printf(RED "Error: Must be exactly %d letters.\n" RESET, WORD_LENGTH);
            continue; 
        }

        // VALIDATION 2: Dictionary Check (Restored)
        if (!is_valid_word(guess)) {
            printf(RED "Error: Word not found in dictionary. Try again.\n" RESET);
            continue;
        }

        calculate_feedback(guess, target, colors);

        printf("Result: ");
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (colors[i] == 2) printf(GREEN "%c" RESET, guess[i]);
            else if (colors[i] == 1) printf(YELLOW "%c" RESET, guess[i]);
            else printf(GRAY "%c" RESET, guess[i]);
        }
        printf("\n");

        if (strcmp(guess, target) == 0) won = 1;
        else attempts++;
    }

    if (won) printf("\n" GREEN BOLD "VICTORY! Word: %s" RESET "\n", target);
    else printf("\n" RED BOLD "DEFEAT. Word: %s" RESET "\n", target);
}

// --- OPTION 2: SOLVER MODE ---
int is_possible(const char *candidate, const char *guess, const int *feedback) {
    int temp_feedback[WORD_LENGTH];
    calculate_feedback(guess, candidate, temp_feedback);
    for(int i=0; i<WORD_LENGTH; i++) {
        if(temp_feedback[i] != feedback[i]) return 0;
    }
    return 1;
}

void solve_game() {
    int random_index = rand() % dict_size;
    char target[WORD_LENGTH + 1];
    strcpy(target, dictionary[random_index]);
    
    printf("\n" BOLD "--- SOLVER MODE ---" RESET "\n");
    printf(CYAN "Target Word (Hidden): %s" RESET "\n", target);

    int *valid_candidates = malloc(dict_size * sizeof(int));
    for(int i=0; i<dict_size; i++) valid_candidates[i] = 1;
    
    int attempts = 0;
    int won = 0;
    char current_guess[WORD_LENGTH + 1];
    
    if(dict_size > 0) strcpy(current_guess, "CRANE"); 
    if (!is_valid_word(current_guess) && dict_size > 0) strcpy(current_guess, dictionary[0]);

    while(attempts < MAX_GUESSES && !won) {
        printf("\nSolver Guess %d: " BOLD "%s" RESET, attempts + 1, current_guess);
        
        int feedback[WORD_LENGTH];
        calculate_feedback(current_guess, target, feedback);

        printf(" -> ");
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (feedback[i] == 2) printf(GREEN "G" RESET);
            else if (feedback[i] == 1) printf(YELLOW "Y" RESET);
            else printf(GRAY "X" RESET);
        }
        printf("\n");

        if (strcmp(current_guess, target) == 0) {
            won = 1;
            break;
        }

        int next_guess_index = -1;
        for(int i=0; i<dict_size; i++) {
            if(valid_candidates[i]) {
                if(is_possible(dictionary[i], current_guess, feedback)) {
                    valid_candidates[i] = 1;
                    if(next_guess_index == -1) next_guess_index = i;
                } else {
                    valid_candidates[i] = 0;
                }
            }
        }

        if (next_guess_index != -1) {
            strcpy(current_guess, dictionary[next_guess_index]);
        } else {
            printf("Error: Solver stuck! No words match feedback.\n");
            break;
        }
        attempts++;
    }
    
    if (won) printf("\n" GREEN "Solver found the word in %d attempts!" RESET "\n", attempts + 1);
    else printf("\n" RED "Solver failed." RESET "\n");

    free(valid_candidates);
}

// --- MAIN CONTROLLER ---
int main() {
    srand(time(NULL));
    load_dictionary();

    int current_mode = 0; 
    int keep_running = 1;

    while (keep_running) {
        if (current_mode == 0) {
            printf("\n" BOLD "=== WORDLE PROJECT ===" RESET "\n");
            printf("1. Play Game (You guess)\n");
            printf("2. Run Solver (Computer guesses)\n");
            printf("3. Exit\n");
            printf("Choice: ");
            scanf("%d", &current_mode);
            while (getchar() != '\n'); 

            if (current_mode == 3) {
                printf("Goodbye!\n");
                return 0;
            }
            if (current_mode != 1 && current_mode != 2) {
                printf("Invalid choice.\n");
                current_mode = 0;
                continue;
            }
        }

        if (current_mode == 1) play_game();
        else if (current_mode == 2) solve_game();

        printf("\n" BOLD "--- WHAT NEXT? ---" RESET "\n");
        printf("1. Play Again (Same Mode)\n");
        printf("2. Change Mode (Main Menu)\n");
        printf("3. Exit Game\n");
        printf("Choice: ");
        
        int post_choice;
        scanf("%d", &post_choice);
        while (getchar() != '\n'); 

        if (post_choice == 1) {
            continue; 
        } else if (post_choice == 2) {
            current_mode = 0; 
        } else {
            keep_running = 0; 
            printf("Goodbye!\n");
        }
    }

    return 0;
}
