/*
  Китов Арсен, АПО-11, ДЗ N1
  Задание B. Вариант 3.
  Требуется написать программу, которая способна вычислять арифметические выражения.
  Выражения могут содержать:
  1) знаки операций '+', '-', '/', '*'
  2) Скобки '(', ')'
  3) Целые и вещественные числа, в нотации '123', '123.345', все операции должны быть вещественны, а результаты выведены с точностю до двух знаков после запятой в том числе целые '2.00'
  4) необходимо учитывать приоритеты операций, и возможность унарного минуса, пробелы ничего не значат
  5) Если в выражении встретилась ошибка требуется вывести в стандартный поток вывода "[error]" (без кавычек)
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define MIN_STACK_SIZE 3

  /*********/
 /**TYPES**/
/*********/
typedef enum {
  TT_NUMBER = 0,
  TT_OPERATION = 1,
  TT_FUNCTION = 2,
  TT_BRACE = 3
} token_kind_t;

// если я правильно понимаю, это тратит sizeof(double) * 2 места. В целом так можно делать, или стоит делать как-то иначе
typedef struct {
  double number;
  char value;
  token_kind_t kind;
} token_t;

typedef struct {
  token_t * head;
  size_t real_size;
  size_t max_size;
} token_stack_t;

  /******************************************************************************************************/
 /**HELPER FUNCTIONS: COMPUTE OPERATOR/FUNCTION, GET PRIORITY OF AN OPERATION, DYNAMIC STACK STRUCTURE**/
/******************************************************************************************************/
double compute_operator(char operation, double operand1, double operand2)
{
  assert(operation == '+' || operation == '-' || operation == '*' || operation == '/');
  switch (operation) {
    case '+':
      return operand1 + operand2;
    case '-':
      return operand1 - operand2;
    case '*':
      return operand1 * operand2;
    case '/':
      assert(operand2 != 0); // наверное стоит проверять каким-то другим способом, но пусть пока так
      return operand1 / operand2;
    default:
      return 0;
  }
}

double compute_function(char func, double arg)
{
  assert(func == 'i');
  switch(func) {
    case 'i':
      return -arg;
    default:
      return 0;
  }
}

int priority(char op) {
  return (op == 'i') ? 2 : ((op == '*') || (op == '/') ? 1 : 0);
}

int push(token_stack_t * stack, token_t value) {
  assert(stack != NULL && (stack->head != NULL || (stack->real_size == 0 && stack->max_size == 0)));
  void * realloc_buffer;
  if (stack->real_size < stack->max_size) {
    (stack->head)[stack->real_size] = value;
    stack->real_size += 1;
  }
  else {
    stack->max_size += stack->max_size == 0 ? MIN_STACK_SIZE : (stack->max_size / 2 + 1);
    realloc_buffer = realloc(stack->head, stack->max_size * sizeof(*(stack->head)));
    if (realloc_buffer == NULL)
      return -1;
    stack->head = realloc_buffer;
    (stack->head)[stack->real_size] = value;
    stack->real_size += 1;
  }

  return 0;
}

int pop (token_stack_t * stack) { // не возвращает удаленный элемент, просто 0/-1/-2
  assert(stack != NULL && (stack->head != NULL || (stack->real_size == 0 && stack->max_size == 0)));
  if (stack->real_size == 0) {
    return -2;
  }
  void *realloc_buffer;
  if (stack->real_size > stack->max_size / 2 || (stack->max_size * 3 / 4) >= MIN_STACK_SIZE) {
    stack->real_size -= 1;
  } else {
      stack->max_size -= stack->max_size / 4;
      realloc_buffer = realloc(stack->head, stack->max_size * sizeof(*(stack->head)));
      if (realloc_buffer == NULL)
        return -1;
      stack->head = realloc_buffer;
    stack->real_size -= 1;
  }

  return 0;
}

bool is_zero(const char * ch) // нужно для проверки валидности atof, хотя если алгоритм правилен, вызов всегда валиден
{
  assert(ch != NULL);
  for (size_t i = 0; ch[i] != '\0'; ++i)
  {
    if (ch[i] != '0' && ch[i] != '.')
      return false;
  }
  return ch[0] == '0'; //проверка на пустую строчку и точку
}

bool is_operation(char c)
{
  return (c == '+' || c == '-' || c == '*' || c == '/');
}

bool is_digit(char c)
{
  return (c >= '0' && c <= '9');
}

/**************************************************************************************************************/
/**ACTUAL ALGORITHM: read one token, read a string, convert string to reverse polish notation, compute polish**/
/**************************************************************************************************************/

int read_expr(char ** line, bool ignore_newline) { // читает без пробелов до EOF или конца строки
  assert(line != NULL);
  if (*line != NULL)
    free(line);

  int chars_read = 0;
  size_t BUFFER_SIZE = 2048;
  char buffer[BUFFER_SIZE];
  int x;

  do {
    x = getchar();
    if (x == EOF || (x == '\n' && !ignore_newline)) {
      buffer[chars_read] = '\0';
      *line = malloc((chars_read + 1) * sizeof(char));
      if (*line == NULL)
        return -1;
      strcpy(*line, buffer);
    } else if (chars_read == BUFFER_SIZE - 2) // если был malloc, то из цикла уже вышли
      return -1;
    else if (!isspace(x)) // идет после проверки на \n - порядок важен
    {
      buffer[chars_read] = (char)x;
      ++chars_read;
    } // иначе (x = пробел/таб) ничего не делаем
  }  while( x != EOF && (x != '\n' || ignore_newline));
  if (ferror(stdin))
  {
    free(line);
    return -1;
  }
  return chars_read;
}

// -2 - неверная строка, -1 - ошибка памяти или чего-то еще
// polish_expr это на самом деле не стэк, а просто динамический массив - из него никогда не удаляются значения
// но использовать тот же struct проще
// функция не освобождает ни polish_expr, ни expr в случае ошибки
int to_reverse_polish_notation(const char * expr, token_stack_t * polish_expr)
{
  assert(expr != NULL && polish_expr != NULL && polish_expr->head == NULL);
  polish_expr->real_size = polish_expr->max_size = 0;
  const size_t BUFFER_SIZE = 32;
  token_stack_t tokens = {NULL, 0, 0};
  token_t tmp_token;
  size_t first_digit = 0;
  bool reading_number = false;
  bool dot_passed = false;
  char buffer[BUFFER_SIZE];
  int error_st;
  size_t i = 0;
  size_t size = strlen(expr);
  for (; i < size; ++i) {
    if (expr[i] >= 0x30 && expr[i] <= 0x39) {
      if (!reading_number) {
        reading_number = true;
        first_digit = i;
        dot_passed = false;
      }
    } else if (expr[i] == '.') {
      // не уверен, можно ли так делать (i==0 || expr[i-1]), но в целом работает
      // помню, в c++ точно порядок задан, по сям не смог нормальный ответ нагуглить
      if (dot_passed || !reading_number || i == 0 || !is_digit(expr[i-1])) {
        free(tokens.head);
        return -2;
      } else {
        dot_passed = true;
      }
    } else {
      if (reading_number) {
        reading_number = false;
        if (i - first_digit >= BUFFER_SIZE) {  // 30 знаков в double все равно не влезет
          free(tokens.head);
          return -1;
        }
        strncpy(buffer, expr + first_digit, i - first_digit);
        buffer[i - first_digit] = '\0';
        tmp_token.kind = TT_NUMBER;
        tmp_token.number = atof(buffer);
        assert(tmp_token.number != 0 || is_zero(buffer)); // всегда верно, если алгоритм работает правильно
        error_st = push(polish_expr, tmp_token);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
      }
      //same i==0 || expr[i-1] as above
      if (expr[i] == '-' && (i == 0 || !(is_digit(expr[i-1]) || expr[i-1] == ')'))) {
        if (i != 0 && expr[i-1] != '(') { // из-за этой строчки конструкции вида 3+-2 неверны (надо 3 + (-2))
          free(tokens.head);
          return -1;
        }
        tmp_token.kind = TT_FUNCTION;
        tmp_token.value = 'i';
        error_st = push(&tokens, tmp_token);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
      } else if (is_operation(expr[i])) {
        while (tokens.real_size && (tokens.head[tokens.real_size - 1].kind == TT_OPERATION ||
          tokens.head[tokens.real_size - 1].kind == TT_FUNCTION) &&
          (priority(expr[i]) <= priority(tokens.head[tokens.real_size - 1].value))) {
            error_st = push(polish_expr, tokens.head[tokens.real_size - 1]);
            if (error_st < 0) {
              free(tokens.head);
              return error_st;
            }
            error_st = pop(&tokens);
            if (error_st < 0) {
              free(tokens.head);
              return error_st;
            }
        }
        tmp_token.kind = TT_OPERATION;
        tmp_token.value = expr[i];
        error_st = push(&tokens, tmp_token);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
      } else if (expr[i] == '(') {
        tmp_token.kind = TT_BRACE;
        tmp_token.value = '(';
        error_st = push(&tokens, tmp_token);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
      } else if (expr[i] == ')') {
        if (tokens.real_size == 0) { // проверяем на пустоту всего разок, внутри проверяется в pop()
          free(tokens.head);
          return -2;
        }
        while (tokens.head[tokens.real_size - 1].kind != TT_BRACE &&
          tokens.head[tokens.real_size - 1].value != '(') {
            error_st = push(polish_expr, tokens.head[tokens.real_size - 1]);
            if (error_st < 0) {
              free(tokens.head);
              return error_st;
            }
            error_st = pop(&tokens);
            if (error_st < 0) {
              free(tokens.head);
              return error_st;
            }
        }
        // tokens, не может быть пустым, но все равно проверяем, если realloc в pop вернет nullptr
        error_st = pop(&tokens);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }

        if (tokens.real_size != 0 && tokens.head[tokens.real_size-1].kind == TT_FUNCTION) {
          error_st = push(polish_expr, tokens.head[tokens.real_size - 1]);
          if (error_st < 0) {
            free(tokens.head);
            return error_st;
          }
          error_st = pop(&tokens);
          if (error_st < 0) {
            free(tokens.head);
            return error_st;
          }
        }
      } else { // expr[i] не входит в 0-9+-*/() - убираем за собой и выбрасываем ошибку
        free(tokens.head);
        return -2;
      }
    }

  }
  if (reading_number) { // если выражение заканчивается на число, мы его еще не прочитали
    if (i - first_digit >= BUFFER_SIZE) {  // снова выбрасываем ошибку, если не помещается
      free(tokens.head);
      return -1;
    }
    strncpy(buffer, expr + first_digit, i - first_digit);
    buffer[i-first_digit] = '\0';
    tmp_token.kind = TT_NUMBER;
    tmp_token.number = atof(buffer);
    assert(tmp_token.number != 0 || is_zero(buffer)); // и все та же проверка на правильность алгоритма
    error_st = push(polish_expr, tmp_token);
    if (error_st < 0) {
      free(tokens.head);
      return error_st;
    }
  }

  while (tokens.real_size != 0) {
    if (tokens.head[tokens.real_size - 1].kind == TT_BRACE) {
      free(tokens.head);
      return -2;
    } else {
      error_st = push(polish_expr, tokens.head[tokens.real_size - 1]);
      if (error_st < 0) {
        free(tokens.head);
        return error_st;
      }
      error_st = pop(&tokens);
      if (error_st < 0) {
        free(tokens.head);
        return error_st;
      }
    }
  }
  free(tokens.head);
  return 0;
}

// -2 for incorrect sequence, -1 for other errors
int compute_polish(const token_stack_t * expr, double * result) {
  assert(expr != NULL);
  if (expr->head == NULL || expr->real_size == 0 || expr->max_size == 0) {
    return -2;
  }

  token_stack_t tokens = {NULL, 0, 0};
  token_t tmp_token;
  tmp_token.kind = TT_NUMBER;
  int error_st;
  for (size_t i = 0; i < expr->real_size; ++ i) {
    switch (expr->head[i].kind) {

      case TT_NUMBER:
        error_st = push(&tokens, expr->head[i]);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
        break;
      case TT_FUNCTION:
        if (tokens.real_size == 0) {
          free(tokens.head); // функция без операндов (e.g. 3 + (-))
          return -2;
        }
        tokens.head[tokens.real_size - 1].number =
          compute_function(expr->head[i].value, tokens.head[tokens.real_size - 1].number);
        break;
      case TT_OPERATION:
        if (tokens.real_size <= 1) {
          free(tokens.head); // бинарный оператор с <=1 операндом (напр., 3+ или +)
          return -2;
        }
        // отправляем в порядке добавления! (stack[-2], stack[-1])
        tmp_token.number = compute_operator(expr->head[i].value,
          tokens.head[tokens.real_size - 2].number, tokens.head[tokens.real_size - 1].number);
        // pop 1 раз, затем запись в последний элемент - то же, что и pop pop push
        error_st = pop(&tokens);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
        tokens.head[tokens.real_size - 1].number = tmp_token.number;
        break;
      case TT_BRACE: // в польской нотации нет скобок
        free(tokens.head);
        return -2;
        break;
    }

  }

  if (tokens.real_size != 1) {
    free(tokens.head);
    return -2;
  }
  *result = tokens.head[0].number;
  free(tokens.head);
  return 0;
}

int main() {
  char * line = NULL;
  token_stack_t polish_expr = {NULL, 0, 0};
  double res;
  int error_st = read_expr(&line, false); // прочитать до \n (true = до EOF)
  if (error_st < 0) { // если не удалось, line освобождается в read_expr
    printf("[error]");
    return 0;
  }
#ifdef PRINT_TRACE
  printf("Expression we've read (no spaces): %s\n", line);
#endif
  error_st = to_reverse_polish_notation(line, &polish_expr);
  if (error_st < 0) {
    free(line);
    free(polish_expr.head);
    printf("[error]");
    return 0;
  }
#ifdef PRINT_TRACE
  printf("Converted to polish notation:\n");
  for (size_t i = 0; i < polish_expr.real_size; ++i) {
    if (polish_expr.head[i].kind == TT_NUMBER) {
      printf("%f,", polish_expr.head[i].number);
    } else {
      printf("%c,", polish_expr.head[i].value);
    }
  }
#endif
  error_st = compute_polish(&polish_expr, &res);
  if (error_st < 0) {
    free(line);
    free(polish_expr.head);
    printf("[error]");
    return 0;
  }
#ifdef PRINT_TRACE
  printf("\n Solution: %f", res);
#endif
  printf("%.2f", res);
  free(line); free(polish_expr.head);
  return 0;
}