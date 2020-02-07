#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_BUFFER_INCREASE 100
#define WORDS_BUFFER 1000

typedef enum {
    false, true
} bool;

int CompareWords(const void *a, const void *b) {
    return (strcmp(*(char **) a, *(char **) b));
}

char **AddWordToList(char **words, char **word, size_t *words_counter, size_t *word_index, size_t *word_buffer,
                     size_t *words_buffer) {
    if (*word != NULL) {
        (*word)[*word_index] = '\0';
        words[*words_counter] = *word;
        *word_index = 0;
        *word_buffer = 100 * sizeof(char);
        *word = NULL;
        ++(*words_counter);

        if ((*words_counter + 1) * sizeof(char *) > *words_buffer) {
            *words_buffer += WORDS_BUFFER;
            words = realloc(words, *words_buffer);
            if (words == NULL) {
                perror("ERROR\n");
            }
        }
    }

    return words;
}

char *ReadText() {

    size_t buffer = 100, index = 0;
    char *input_text = (char *) malloc(sizeof(char) * buffer);
    int symbol;

    while ((symbol = getchar()) != EOF) {
        input_text[index] = (char) symbol;
        ++index;

        if (buffer <= index) {
            buffer += WORD_BUFFER_INCREASE;
            input_text = realloc(input_text, buffer);

            if (input_text == NULL) {
                exit(1);
            }
        }
    }

    input_text[index] = '\0';

    return input_text;
}

char **MakeAndSortWords(char *input_text) {

    size_t text_index = 0, words_counter = 0, word_index = 0, amp_counter = 0, stick_counter = 0;
    size_t word_buffer = 100 * sizeof(char), words_buffer = 1000 * sizeof(char *);
    bool space_sequence = true;
    char quotes = '\0';
    char **words = (char **) malloc(words_buffer);
    char *word = NULL;

    while (input_text[text_index]) {

        int symbol = input_text[text_index];
        ++text_index;

        if (quotes != '\0') {
            if (symbol == quotes) {
                quotes = '\0';
            } else {
                if (!word) {
                    word_buffer = 100 * sizeof(char);
                    word_index = 0;
                    word = realloc(word, word_buffer);
                }
                word[word_index] = (char) symbol;
                ++word_index;
                if (word_index * sizeof(char) >= word_buffer) {
                    word_buffer += WORD_BUFFER_INCREASE;
                    word = realloc(word, word_buffer);
                }
            }
            continue;
        } else {

            switch (symbol) {
                case ';':
                    space_sequence = true;
                    words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                    char *string = (char *) malloc(2);
                    string[0] = ';';
                    string[1] = '\0';
                    words[words_counter] = string;
                    ++words_counter;
                    break;
                case '&':
                    space_sequence = true;
                    stick_counter = 0;
                    if (!amp_counter) {
                        ++amp_counter;
                        words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                        word = realloc(word, 3);
                        word[0] = '&';
                        word[1] = word[2] = '\0';
                        word_index = 1;
                    } else {
                        word[1] = '&';
                        word_index = 2;
                        words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                        amp_counter = 0;
                    }
                    break;
                case '|':
                    space_sequence = true;
                    amp_counter = 0;
                    if (!stick_counter) {
                        ++stick_counter;
                        words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                        word = realloc(word, 3);
                        word[0] = '|';
                        word[1] = word[2] = '\0';
                        word_index = 1;
                    } else {
                        word[1] = '|';
                        word_index = 2;
                        words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                        stick_counter = 0;
                    }
                    break;
                case '\"':
                    quotes = '\"';
                    break;
                case '\'':
                    quotes = '\'';
                    break;
                default:
                    if (isspace(symbol)) {
                        space_sequence = true;
                        words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                    } else {
                        if (space_sequence) {
                            words = AddWordToList(words, &word, &words_counter, &word_index, &word_buffer, &words_buffer);
                            word = realloc(word, word_buffer);
                            space_sequence = false;
                        }

                        word[word_index] = (char) symbol;
                        ++word_index;

                        if (word_index * sizeof(char) >= word_buffer) {
                            word_buffer += WORD_BUFFER_INCREASE;
                            word = realloc(word, word_buffer);
                        }
                    }
            }
        }
    }

    if (quotes != '\0') {
        perror("ERROR\n");
    }

    if (word) {
        words[words_counter] = word;
        ++words_counter;
    }

    words[words_counter] = NULL;

    qsort(words, words_counter, sizeof(char **), CompareWords);

    return words;
}

void PrintSortedWords(char **words) {

    for (size_t i = 0; words[i] != NULL; ++i) {
        printf("\"%s\"\n", words[i]);
    }

}

void FreeMemory(char *input_text, char **words) {
    free(input_text);
    for (size_t i = 0; words[i] != NULL; ++i) {
        free(words[i]);
    }
}

int main() {
    char *input_text = ReadText();
    char **words = MakeAndSortWords(input_text);
    PrintSortedWords(words);
    FreeMemory(input_text, words);
    return 0;
}