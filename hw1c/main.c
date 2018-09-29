/* Задание А. Вариант 7.
  Составить программу построчной обработки текста. Суть обработки - отбор строк, 
  содержащих одинаковое количество открывающих и закрывающих круглых скобок. 

  Программа считывает входные данные со стандартного ввода, и печатает результат в стандартный вывод. 

  Процедура отбора нужных строк должна быть оформлена в виде отдельной функции,
  которой на вход подается массив строк (который необходимо обработать),
  количество переданных строк, а также указатель на переменную,
  в которой необходимо разместить результат - массив отобранных строк. 
  В качестве возвращаемого значения функция должна возвращать количество строк,
  содержащихся в результирующем массиве. 

  Программа должна уметь обрабатывать ошибки - такие как неверные
  входные данные(отсутствие входных строк) или ошибки выделения памяти и т.п.
  В случае возникновения ошибки нужно выводить об этом сообщение "[error]"
  и завершать выполнение программы. 
*/


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

void free_pptr(void ** pptr, size_t size) {
  if (pptr != NULL) {
    for (int i = 0; i < size; ++i)
      free(pptr[i]);
    free(pptr);
  }
}

bool braces_closed(const char * const str) {
  int braces_open = 0;
  size_t size = strlen(str);
  for (size_t i = 0; i < size; ++i) {
    if (str[i] == '(')
      ++braces_open;
    else if (str[i] == ')') {
      --braces_open;
      if (braces_open < 0) // this invalidates )( , although I'm not sure that's how it should be
        return false;
    }
  }
  return (bool) (braces_open == 0);
}

int check_closed_braces(const char *const *const in_strings,
                        char *** out_strings, size_t n_strings) {
  // pout_strings ---> out_strings outside of the function ---> array of pointers ---> actual string
  assert(in_strings != NULL && out_strings != NULL);
  int n_valid_strings = 0;
  void * realloc_buffer = NULL;
  for (size_t i = 0; i < n_strings; ++i) {
    if (braces_closed(in_strings[i])) {
      n_valid_strings += 1;

      realloc_buffer = realloc(*out_strings, n_valid_strings * sizeof(char *));
      if (realloc_buffer == NULL) {
        free_pptr((void**)(*out_strings), (size_t)n_valid_strings);
        return -1;
      }
      *out_strings = (char**)realloc_buffer;
      (*out_strings)[n_valid_strings - 1] = malloc((strlen(in_strings[i]) + 1) * sizeof(char));

      if ((*out_strings)[n_valid_strings - 1] == NULL){
        free_pptr((void**)(*out_strings), (size_t)n_valid_strings);
        return -1;
      }

      strcpy((*out_strings)[n_valid_strings - 1], in_strings[i]);
    }
  }

  return n_valid_strings;
}

int read_strings(char *** lines) {
  assert(lines != NULL);
  const int BUFFER_SIZE = 3000;
  int x = 0;
  char buffer[BUFFER_SIZE];
  int n_lines = 0;
  size_t symbols_read = 0;
  void * realloc_buffer = NULL;


  do {
    x = getchar();
    if (x == '\n' || x == EOF) {
      ++n_lines;
      realloc_buffer = realloc((*lines), n_lines * sizeof(char *));
      if (realloc_buffer == NULL) {
        free_pptr((void**)(*lines), (size_t)n_lines);
        return -1;
      }
      *lines = realloc_buffer;
      (*lines)[n_lines - 1] = malloc((symbols_read+1) * sizeof(char));
      if ((*lines)[n_lines-1] == NULL){
        free_pptr((void**)(*lines), (size_t)n_lines);
        return -1;
      }
      buffer[symbols_read] = '\0';
      strcpy((*lines)[n_lines - 1], buffer);
      symbols_read = 0;
    } else {
      if (symbols_read == BUFFER_SIZE - 1) // buffer can't fit all of this
      {
        free_pptr((void**)(*lines), (size_t)n_lines);
        return -1;
      }
      buffer[symbols_read] = (char)x; // should always be positive.. i think
      ++symbols_read;
    }
  } while (x != EOF);

  if (!feof(stdin)) // then the loop ended with an error
  {
    free_pptr((void**)(*lines), (size_t)n_lines);
    return -1;
  }

  return n_lines;
}

int main() {
/*
  // beware: quality debug code follows
  char ** lines = malloc(4*sizeof(char*));
  lines[0] = malloc(6*sizeof(char));
  lines[1] = malloc(10*sizeof(char));
  lines[2] = malloc(9*sizeof(char));
  lines[3] = malloc(9*sizeof(char));
  strcpy(lines[0], "Hello()\0");
  strcpy(lines[1], "((World))\0");
  strcpy(lines[2], "(World))\0");
  strcpy(lines[3], "((World)\0");
  char ** valid_lines = NULL;
  int k = check_closed_braces(lines, &valid_lines, 2);
  printf("%d\n", k);
  for (int i = 0; i < k; ++i)
    printf("%s\n", valid_lines[i]);

*/
  char ** lines = NULL;
  char ** valid_lines = NULL;
  int n_valid_lines = 0;
  int n_lines = read_strings(&lines);
  if (n_lines == -1) {
    printf("[error]");
    return 0;
  }

  // actually even after some googling no idea what to do about this: this seems like a really dumb way to avoid
  // the warning, making the function take non-const char** seems even dumber.. What's the right way?..
  // I understand why it's wrong to do so (afaik, it allows to indirectly create a pointer with non-const
  // value type, thus modifying a const object), but not sure of the workaround/right way to do this.
  // send help.
  n_valid_lines = check_closed_braces((const char *const *const)lines, &valid_lines, n_lines);

  if (n_valid_lines == -1) //valid_lines is freed in check_closed_braces
  {
    printf("[error]");
    free_pptr((void**)lines, (size_t)n_lines);
    return 0;
  }
  for (int i = 0; i < n_valid_lines; ++i) {
    printf("%s\n", valid_lines[i]);
  }
  free_pptr((void**)lines, (size_t)n_lines);
  free_pptr((void**)valid_lines, (size_t)n_valid_lines);


  return 0;
}