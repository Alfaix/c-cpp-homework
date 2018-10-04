/*
  Китов Арсен, АПО-11, ДЗ N1
  Задание А. Вариант 7.
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

#define DEFAULT_N_LINES 4

void free_pptr(void ** pptr, size_t size) {
  if (pptr != NULL) {
    for (size_t i = 0; i < size; ++i)
      free(pptr[i]);
    free(pptr);
  }
}

bool braces_closed(const char * const str) {
  assert(str != NULL);
  int braces_open = 0;
  for (const char * i = str; *i != '\0'; ++i) {
    if (*i == '(')
      ++braces_open;
    else if (*i == ')') {
      --braces_open;
      if (braces_open < 0)
        return false;
    }
  }
  return braces_open == 0;
}

char ** get_strings_with_closed_braces(const char *const *in_strings,
                                       size_t n_strings, size_t *n_valid_strings) {
  assert(in_strings != NULL && n_valid_strings != NULL);
  size_t lines_allocated = 0;
  void * realloc_buffer = NULL;
  char ** out_strings = malloc(DEFAULT_N_LINES * sizeof(char *));
  if (out_strings == NULL)
    return NULL;
  *n_valid_strings = 0;

  for (size_t i = 0; i < n_strings; ++i) {
    if (braces_closed(in_strings[i])) {
      ++(*n_valid_strings);
      if (*n_valid_strings == lines_allocated) {
        lines_allocated += DEFAULT_N_LINES;
        realloc_buffer = realloc(out_strings, lines_allocated * sizeof(char *));
        if (realloc_buffer == NULL) {
          free_pptr((void **) (*out_strings), (size_t) (*n_valid_strings));
          return NULL;
        }
        out_strings = (char **) realloc_buffer;
      }
      out_strings[*n_valid_strings - 1] = strdup(in_strings[i]);

      if (out_strings[*n_valid_strings - 1] == NULL){
        free_pptr((void**)out_strings, (size_t)(*n_valid_strings));
        return NULL;
      }
    }
  }

  if (lines_allocated != *n_valid_strings) { // то же, что и внизу
    realloc_buffer = realloc(out_strings, *n_valid_strings * sizeof(char *));
    if (realloc_buffer == NULL) {
      free_pptr((void **) out_strings, *n_valid_strings);
      return NULL;
    }
    out_strings = realloc_buffer;
  }

  return out_strings;
}

char ** read_strings(size_t * n_lines_read) {
  assert(n_lines_read != NULL);
  void * realloc_buffer = NULL;
  char ** lines = NULL;
  size_t lines_allocated = DEFAULT_N_LINES;
  *n_lines_read = 0;
  lines = malloc(DEFAULT_N_LINES * sizeof(char *));
  if (lines == NULL)
    return NULL;

  while(scanf(" %m[^\n]", &(lines[*n_lines_read])) == 1) { // пробел в начале важен!
    ++(*n_lines_read);
    if (*n_lines_read == lines_allocated) {
      lines_allocated += DEFAULT_N_LINES;
      realloc_buffer = realloc(lines, lines_allocated * sizeof(char *));
      if (realloc_buffer == NULL) {
        free_pptr((void **) lines, *n_lines_read); // n_lines_read == lines_allocated - DEFAULT_N_LINES
        return NULL;
      }
      lines = (char **)realloc_buffer;
    }
  }

  if (lines_allocated != *n_lines_read) { //realloc чтобы сократить неиспользуемую память (надо ли?)
    realloc_buffer = realloc(lines, *n_lines_read * sizeof(char *));
    if (realloc_buffer == NULL) {
      free_pptr((void **) lines, *n_lines_read);
      return NULL;
    }
    lines = realloc_buffer;
  }

  return lines;
}

int main() {
  char ** valid_lines = NULL;
  size_t n_valid_lines = 0;
  size_t n_lines = 0;

  char ** lines = read_strings(&n_lines);
  if (lines == NULL) {
    printf("[error]");
    return 0;
  }

  valid_lines = get_strings_with_closed_braces((const char *const *) lines, n_lines, &n_valid_lines);
  if (valid_lines == NULL)
  {
    printf("[error]");
    free_pptr((void**)lines, n_lines);
    return 0;
  }

  for (size_t i = 0; i < n_valid_lines; ++i) {
    printf("%s\n", valid_lines[i]);
  }
  
  free_pptr((void**)lines, n_lines);
  free_pptr((void**)valid_lines, n_valid_lines);


  return 0;
}