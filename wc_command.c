#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

struct FileInfo {
  size_t strings_number;
  size_t words_number;
  size_t symbols_number;
};

typedef struct FileInfo FileInfo;

typedef enum {
  false, true
} bool;

struct FileInfo WC(FILE *file) {
  int symbol;
  bool word_end = false;
  FileInfo Info = {0, 0, 0};

  while ((symbol = fgetc(file)) != EOF) {
    ++Info.symbols_number;

    if (symbol == '\n') {
      ++Info.strings_number;
    }

    if (isspace(symbol)) {
      if (word_end) {
        ++Info.words_number;
        word_end = false;
      }
    } else {
      word_end = true;
    }
  }

  if (word_end) {
    ++Info.words_number;
  }

  return Info;
}

int main(int argc, char **argv) {

  if (argc == 1) {
    FILE *file = stdin;
    FileInfo Info = WC(file);
    printf("%zu %zu %zu\n", Info.strings_number, Info.words_number, Info.symbols_number);
    fclose(file);
  } else {
    for (int i = 1; i < argc; ++i) {
      FILE *file = fopen(argv[i], "r");
      if (!file) {
        perror(argv[i]);
        continue;
      }
      FileInfo Info = WC(file);
      printf("%zu %zu %zu %s\n", Info.strings_number, Info.words_number, Info.symbols_number, argv[i]);
      fclose(file);
    }
  }

  return 0;
}