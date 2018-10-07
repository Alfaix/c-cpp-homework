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
// #define PRINT_TRACE

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
bool is_operation(char c)
{
  return (c == '+' || c == '-' || c == '*' || c == '/');
}

bool is_digit(char c)
{
  return (c >= '0' && c <= '9');
}

double compute_operator(char operation, double operand1, double operand2, bool * valid)
{
  assert(valid != NULL);
  *valid = true;
  switch (operation) {
    case '+':
      return operand1 + operand2;
    case '-':
      return operand1 - operand2;
    case '*':
      return operand1 * operand2;
    case '/':
      if (operand2 != 0) {
        return operand1 / operand2;
      } else {
        *valid = false;
        return 0;
      }
    default:
      *valid = false;
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

int pop(token_stack_t * stack) { // не возвращает удаленный элемент, просто 0/-1/-2
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

int move_token(token_stack_t * from, token_stack_t * to) {
  assert(from != NULL && to != NULL); // the rest is checked in push and pop

  int error_st = push(to, from->head[from->real_size - 1]);
  if (error_st < 0) return error_st;

  error_st = pop(from);
  return error_st;
}

  /*******************************************************************************/
 /**ACTUAL ALGORITHM: convert string to reverse polish notation, compute polish**/
/*******************************************************************************/

// -2 - неверная строка, -1 - ошибка памяти или чего-то еще
// polish_expr это на самом деле не стэк, а просто динамический массив - из него никогда не удаляются значения
// но использовать тот же struct проще
// функция не освобождает ни polish_expr, ни expr в случае ошибки
int to_reverse_polish_notation(const char * expr, token_stack_t * polish_expr)
{
  assert(expr != NULL && polish_expr != NULL && polish_expr->head == NULL);
  polish_expr->real_size = polish_expr->max_size = 0;

  token_stack_t tokens = {NULL, 0, 0};
  token_t tmp_token;
  int error_st;
  char * first_nondigit = NULL;

  for (const char *c = expr; *c != '\0'; ++c) {
    if (is_digit(*c)) {
      if (tokens.real_size && tokens.head[tokens.real_size - 1].kind == TT_NUMBER) { // two numbers separated by a space
        free(tokens.head);
        return -1;
      }
      tmp_token.kind = TT_NUMBER;
      tmp_token.number = strtod(c, &first_nondigit);
      error_st = push(polish_expr, tmp_token);
      if (error_st < 0) {
        free(tokens.head);
        return error_st;
      }
      c = first_nondigit - 1; // -1 потому что в конце цикла инкремент
    } else if (*c == '-' &&
               (c == expr || !(is_digit(*(c - 1)) || *(c - 1) == ')'))) { // здесь проверяем, унарный ли минус
      if (c != expr && *(c - 1) != '(') { // из-за этой строчки конструкции вида 3+-2 неверны (надо 3 + (-2))
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
    } else if (is_operation(*c)) { // и только потом проверяем, операция ли
      while (tokens.real_size && (tokens.head[tokens.real_size - 1].kind == TT_OPERATION ||
                                  tokens.head[tokens.real_size - 1].kind == TT_FUNCTION) &&
             (priority(*c) <= priority(tokens.head[tokens.real_size - 1].value))) {
        error_st = move_token(&tokens, polish_expr);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
      }
      tmp_token.kind = TT_OPERATION;
      tmp_token.value = *c;
      error_st = push(&tokens, tmp_token);
      if (error_st < 0) {
        free(tokens.head);
        return error_st;
      }
    } else if (*c == '(') {
      tmp_token.kind = TT_BRACE;
      tmp_token.value = '(';
      error_st = push(&tokens, tmp_token);
      if (error_st < 0) {
        free(tokens.head);
        return error_st;
      }
    } else if (*c == ')') {
      if (tokens.real_size == 0) { // проверяем на пустоту всего разок, внутри проверяется в pop()
        free(tokens.head);
        return -2;
      }
      while (tokens.head[tokens.real_size - 1].kind != TT_BRACE &&
             tokens.head[tokens.real_size - 1].value != '(') {
        error_st = move_token(&tokens, polish_expr);
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

      if (tokens.real_size != 0 && tokens.head[tokens.real_size - 1].kind == TT_FUNCTION) {
        error_st = move_token(&tokens, polish_expr);
        if (error_st < 0) {
          free(tokens.head);
          return error_st;
        }
      }
    } else if (!isspace(*c)) { // *c не входит в 0-9+-*/() - убираем за собой и выбрасываем ошибку (точка парсится в strtod)
      free(tokens.head);
      return -2;
    } // иначе c пробел, читаем дальше. Если и до и после пробелов идет число, будет проверено, когда мы его встретим
  }


  while (tokens.real_size != 0) {
    if (tokens.head[tokens.real_size - 1].kind == TT_BRACE) {
      free(tokens.head);
      return -2;
    } else {
      error_st = move_token(&tokens, polish_expr);
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
  assert(expr != NULL && result != NULL);
  if (expr->head == NULL || expr->real_size == 0 || expr->max_size == 0) {
    return -2;
  }

  token_stack_t tokens = {NULL, 0, 0};
  token_t tmp_token;
  tmp_token.kind = TT_NUMBER;
  bool is_valid = false;
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
      case TT_FUNCTION: // только унарный минус
        assert(expr->head[i].value == 'i');
        if (tokens.real_size == 0) {
          free(tokens.head); // функция без операндов (e.g. 3 + (-))
          return -2;
        }
        tokens.head[tokens.real_size - 1].number *= -1;
        break;
      case TT_OPERATION:
        if (tokens.real_size <= 1) {
          free(tokens.head); // бинарный оператор с <=1 операндом (напр., 3+ или +)
          return -2;
        }
        // отправляем в порядке добавления! (stack[-2], stack[-1])
        tmp_token.number = compute_operator(expr->head[i].value,
                                            tokens.head[tokens.real_size - 2].number,
                                            tokens.head[tokens.real_size - 1].number, &is_valid);
        if (!is_valid) {
          free(tokens.head);
          return -2;
        }
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
  int error_st = scanf(" %m[^\n]", &line); // просто читаем одну строчку
  if (error_st != 1) { // если не удалось, line освобождается в read_expr
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