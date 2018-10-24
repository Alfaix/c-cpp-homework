/*
  Китов Арсен, АПО-11, ДЗ N1
  Задание B. Вариант 3.
  Требуется написать программу, которая способна вычислять арифметические
  выражения. Выражения могут содержать: 1) знаки операций '+', '-', '/', '*' 2)
  Скобки '(', ')' 3) Целые и вещественные числа, в нотации '123', '123.345', все
  операции должны быть вещественны, а результаты выведены с точностю до двух
  знаков после запятой в том числе целые '2.00' 4) необходимо учитывать
  приоритеты операций, и возможность унарного минуса, пробелы ничего не значат
  5) Если в выражении встретилась ошибка требуется вывести в стандартный поток
  вывода "[error]" (без кавычек)
*/

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_STACK_SIZE 3
// #define PRINT_TRACE

/*********/
/**TYPES**/
/*********/
#define ADD_SIGN '+'
#define SUBSTRACT_SIGN '-'
#define MULTIPLY_SIGN '*'
#define DIVIDE_SIGN '/'
#define INVERSE_SIGN 'i'

typedef enum {
  TT_NUMBER = 0,
  TT_OPERATION = 1,
  TT_FUNCTION = 2,
  TT_BRACE = 3
} token_kind_t;

typedef enum {
  PS_OK = 0,
  PS_MEMORY_ERROR = -1,
  PS_LOGIC_ERROR = -2
} parsing_status_t;

// если я правильно понимаю, это тратит sizeof(double) * 2 места. В целом так
// можно делать, или стоит делать как-то иначе
typedef struct {
  double number;
  char value;
  token_kind_t kind;
} token_t;

typedef struct {
  token_t *head;
  size_t real_size;
  size_t max_size;
} token_stack_t;

bool is_operation(char c) {
  return (c == ADD_SIGN || c == SUBSTRACT_SIGN || c == MULTIPLY_SIGN ||
          c == DIVIDE_SIGN);
}

bool is_digit(char c) { return (c >= '0' && c <= '9'); }

double compute_operator(char operation, double operand1, double operand2,
                        bool *valid) {
  assert(valid != NULL);
  *valid = true;
  switch (operation) {
  case ADD_SIGN:
    return operand1 + operand2;
  case SUBSTRACT_SIGN:
    return operand1 - operand2;
  case MULTIPLY_SIGN:
    return operand1 * operand2;
  case DIVIDE_SIGN:
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
  return (op == INVERSE_SIGN)
             ? 2
             : ((op == MULTIPLY_SIGN) || (op == DIVIDE_SIGN) ? 1 : 0);
}

parsing_status_t push(token_stack_t *stack, token_t value) {
  assert(stack != NULL && (stack->head != NULL ||
                           (stack->real_size == 0 && stack->max_size == 0)));
  void *realloc_buffer = NULL;
  if (stack->real_size < stack->max_size) {
    (stack->head)[stack->real_size] = value;
    ++(stack->real_size);
  } else {
    stack->max_size +=
        stack->max_size == 0 ? MIN_STACK_SIZE : (stack->max_size / 2 + 1);
    realloc_buffer =
        realloc(stack->head, stack->max_size * sizeof(*(stack->head)));
    if (realloc_buffer == NULL)
      return PS_MEMORY_ERROR;
    stack->head = realloc_buffer;
    (stack->head)[stack->real_size] = value;
    ++(stack->real_size);
  }

  return PS_OK;
}

parsing_status_t
pop(token_stack_t *stack) { // не возвращает удаленный элемент, просто 0/-1/-2
  assert(stack != NULL && (stack->head != NULL ||
                           (stack->real_size == 0 && stack->max_size == 0)));
  if (stack->real_size == 0) {
    return PS_LOGIC_ERROR;
  }
  void *realloc_buffer = NULL;
  if (stack->real_size > stack->max_size / 2 ||
      (stack->max_size * 3 / 4) >= MIN_STACK_SIZE) {
    --(stack->real_size);
  } else {
    stack->max_size -= stack->max_size / 4;
    realloc_buffer =
        realloc(stack->head, stack->max_size * sizeof(*(stack->head)));
    if (realloc_buffer == NULL)
      return PS_MEMORY_ERROR;
    stack->head = realloc_buffer;
    --(stack->real_size);
  }

  return PS_OK;
}

token_t *top(token_stack_t *stack) {
  assert(stack != NULL && stack->head != NULL && stack->real_size != 0 &&
         stack->max_size != 0);
  return &(stack->head[stack->real_size - 1]);
}

int move_token(token_stack_t *from, token_stack_t *to) {
  assert(from != NULL && to != NULL); // the rest is checked in push and pop

  int error_st = push(to, from->head[from->real_size - 1]);
  if (error_st < 0)
    return error_st;

  error_st = pop(from);
  return error_st;
}

// -2 - неверная строка, -1 - ошибка памяти или чего-то еще
// polish_expr это на самом деле не стэк, а просто динамический массив - из него
// никогда не удаляются значения но использовать тот же struct проще функция не
// освобождает ни polish_expr, ни expr в случае ошибки
parsing_status_t parse_number(token_stack_t *tokens, token_stack_t *polish_expr,
                              const char *first_digit, char **first_nondigit) {
  token_t tmp_token = {0, 0, 0};
  if (tokens->real_size && top(tokens)->kind == TT_NUMBER) {
    return PS_LOGIC_ERROR;
  }
  tmp_token.number = strtod(first_digit, first_nondigit);
  tmp_token.kind = TT_NUMBER;

  return push(polish_expr, tmp_token);
}

parsing_status_t parse_function(token_stack_t *tokens, const char *current_char,
                                const char *full_expr) {
  parsing_status_t error_st = 0;
  token_t tmp_token = {0, 0, 0};
  if (current_char != full_expr && *(current_char - 1) != '(') {
    return PS_LOGIC_ERROR;
  }
  tmp_token.kind = TT_FUNCTION;
  tmp_token.value = INVERSE_SIGN;
  error_st = push(tokens, tmp_token);
  if (error_st != PS_OK) {
    return error_st;
  }

  return error_st;
}

parsing_status_t parse_operation(token_stack_t *tokens,
                                 token_stack_t *polish_expr,
                                 const char *current_char) {
  parsing_status_t error_st = 0;
  token_t tmp_token = {0, 0, 0};
  while (
      tokens->real_size &&
      (top(tokens)->kind == TT_OPERATION || top(tokens)->kind == TT_FUNCTION) &&
      (priority(*current_char) <= priority(top(tokens)->value))) {
    error_st = move_token(tokens, polish_expr);
    if (error_st != PS_OK) {
      return error_st;
    }
  }
  tmp_token.kind = TT_OPERATION;
  tmp_token.value = *current_char;
  return push(tokens, tmp_token);
}

parsing_status_t parse_closing_brace(token_stack_t *tokens,
                                     token_stack_t *polish_expr) {
  parsing_status_t error_st = 0;
  if (tokens->real_size == 0) {
    return PS_LOGIC_ERROR;
  }
  while (top(tokens)->kind != TT_BRACE && top(tokens)->value != '(') {
    error_st = move_token(tokens, polish_expr);
    if (error_st != PS_OK) {
      return error_st;
    }
  }
  error_st = pop(tokens);
  if (error_st != PS_OK) {
    return error_st;
  }

  if (tokens->real_size != 0 && top(tokens)->kind == TT_FUNCTION) {
    error_st = move_token(tokens, polish_expr);
    if (error_st != PS_OK) {
      return error_st;
    }
  }
  return error_st;
}

parsing_status_t parse_opening_brace(token_stack_t *tokens) {
  token_t tmp_token = {0, 0, 0};
  tmp_token.kind = TT_BRACE;
  tmp_token.value = '(';
  return push(tokens, tmp_token);
}

parsing_status_t parse_token(const char *expr, const char *current_char,
                             token_stack_t *tokens, token_stack_t *polish_expr,
                             const char **next_token_position) {

  ++(*next_token_position);
  if (is_digit(*current_char))
    return parse_number(tokens, polish_expr, current_char,
                        (char **)next_token_position);
  else if (*current_char == '-' &&
           (current_char == expr ||
            !(is_digit(*(current_char - 1)) || *(current_char - 1) == ')')))
    return parse_function(tokens, current_char, expr);
  else if (is_operation(*current_char))
    return parse_operation(tokens, polish_expr, current_char);
  else if (*current_char == '(')
    return parse_opening_brace(tokens);
  else if (*current_char == ')')
    return parse_closing_brace(tokens, polish_expr);
  else if (!isspace(*current_char))
    return PS_LOGIC_ERROR;
  else
    return PS_OK;
}

parsing_status_t to_reverse_polish_notation(const char *expr,
                                            token_stack_t *polish_expr) {
  assert(expr != NULL && polish_expr != NULL && polish_expr->head == NULL);
  polish_expr->real_size = polish_expr->max_size = 0;

  token_stack_t tokens = {NULL, 0, 0};
  parsing_status_t error_st = 0;
  const char *next_token_position = expr;
  const char *c = expr;
  while (*c != '\0') {
    error_st = parse_token(expr, c, &tokens, polish_expr, &next_token_position);
    if (error_st != PS_OK) {
      free(tokens.head);
      return error_st;
    }
    c = next_token_position;
  }

  while (tokens.real_size != 0) {
    if (top(&tokens)->kind == TT_BRACE) {
      free(tokens.head);
      return PS_LOGIC_ERROR;
    } else {
      error_st = move_token(&tokens, polish_expr);
      if (error_st != PS_OK) {
        free(tokens.head);
        return error_st;
      }
    }
  }
  free(tokens.head);
  return PS_OK;
}

// -2 for incorrect sequence, -1 for other errors
int compute_polish(const token_stack_t *expr, double *result) {
  assert(expr != NULL && result != NULL);
  if (expr->head == NULL || expr->real_size == 0 || expr->max_size == 0) {
    return PS_LOGIC_ERROR;
  }

  token_stack_t tokens = {NULL, 0, 0};
  token_t tmp_token;
  tmp_token.kind = TT_NUMBER;
  bool is_valid = false;
  int error_st;
  for (size_t i = 0; i < expr->real_size; ++i) {
    switch (expr->head[i].kind) {

    case TT_NUMBER:
      error_st = push(&tokens, expr->head[i]);
      if (error_st != PS_OK) {
        free(tokens.head);
        return error_st;
      }
      break;
    case TT_FUNCTION: // только унарный минус
      assert(expr->head[i].value == INVERSE_SIGN);
      if (tokens.real_size == 0) {
        free(tokens.head); // функция без операндов (e.g. 3 + (-))
        return PS_LOGIC_ERROR;
      }
      top(&tokens)->number *= -1;
      break;
    case TT_OPERATION:
      if (tokens.real_size <= 1) {
        free(tokens.head);
        return PS_LOGIC_ERROR;
      }
      // отправляем в порядке добавления! (stack[-2], stack[-1])
      tmp_token.number = compute_operator(
          expr->head[i].value, tokens.head[tokens.real_size - 2].number,
          top(&tokens)->number, &is_valid);
      if (!is_valid) {
        free(tokens.head);
        return PS_LOGIC_ERROR;
      }
      // pop 1 раз, затем запись в последний элемент - то же, что и pop pop push
      error_st = pop(&tokens);
      if (error_st != PS_OK) {
        free(tokens.head);
        return error_st;
      }
      top(&tokens)->number = tmp_token.number;
      break;
    case TT_BRACE: // в польской нотации нет скобок
      free(tokens.head);
      return PS_LOGIC_ERROR;
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
  char *line = NULL;
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
  if (error_st != PS_OK) {
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
  if (error_st != PS_OK) {
    free(line);
    free(polish_expr.head);
    printf("[error]");
    return 0;
  }
#ifdef PRINT_TRACE
  printf("\n Solution: %f", res);
#endif
  printf("%.2f", res);
  free(line);
  free(polish_expr.head);
  return 0;
}