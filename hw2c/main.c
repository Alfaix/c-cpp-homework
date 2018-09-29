#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <bits/types/stack_t.h>

#define MIN_STACK_SIZE 3

/****************************************************************/
/**TYPES: OPERATION, FUNCTION, TYPE OF A TOKEN, TOKEN**/
/****************************************************************/
typedef enum {
  OPK_PLUS = '+',
  OPK_MINUS = '-',
  OPK_MULT = '*',
  OPK_DIV = '/'
} operation_t;

typedef enum {
  FK_INVERSE = 'i'
} unary_func_t; // all functions are unary

typedef enum {
  TT_NUMBER = 0,
  TT_OPERATION = 1,
  TT_FUNCTION = 2,
  TT_BRACE = 3
} token_kind_t;

typedef struct {
  double number;
  char brace;
  bool valid;
  operation_t operation;
  unary_func_t ufunc;
  token_kind_t kind;
} token_t; //wasting lots of space (3 bytes out of 16 (?)) but idk how to do it better

/******************************************************************************************************/
/**HELPER FUNCTIONS: COMPUTE OPERATOR/FUNCTION, GET PRIORITY OF AN OPERATION, DYNAMIC STACK STRUCTURE**/
/******************************************************************************************************/
double compute_operator(operation_t operation, double operand1, double operand2)
{
  switch (operation) {
    case OPK_PLUS:
      return operand1 + operand2;
    case OPK_MINUS:
      return operand1 - operand2;
    case OPK_MULT:
      return operand1 * operand2;
    case OPK_DIV:
      assert(operand2 != 0); // should think of some other way to report this
      return operand1 / operand2;
  }
}

double compute_function(unary_func_t func, double arg) //just in case i'll add more
{
  switch(func) {
    case FK_INVERSE:
      return -arg;
  }
}

int priority(operation_t op) {
  return (op == OPK_MULT) || (op == OPK_DIV) ? 1 : 0;
}

int push(token_t ** stack, token_t value, size_t * real_size, size_t * max_size) {
  assert(stack != NULL && real_size != NULL && max_size != NULL && (*stack != NULL || *max_size == 0));
  void * realloc_buffer;
  if (*real_size < *max_size) {
    (*stack)[*real_size] = value;
    *real_size += 1;
  }
  else {
    *max_size += *max_size == 0 ? MIN_STACK_SIZE : (*max_size / 2 + 1);
    realloc_buffer = realloc(*stack, *max_size * sizeof(token_t));
    if (realloc_buffer == NULL)
      return -1;
    *stack = realloc_buffer;
    (*stack)[*real_size] = value;
    *real_size += 1;
  }

  return 0;
}

int pop (token_t ** stack, size_t * real_size, size_t * max_size) {
  assert(stack != NULL && real_size != NULL && max_size != NULL && *stack != NULL);
  assert(*real_size != 0);
  void *realloc_buffer;
  if (*real_size > *max_size / 2) {
    *real_size -= 1;
  } else {
    if ((*max_size * 3 / 4) >= MIN_STACK_SIZE) {
      *max_size -= (*max_size / 4);
      realloc_buffer = realloc(*stack, *max_size * sizeof(token_t));
      if (realloc_buffer == NULL)
        return -1;
      *stack = realloc_buffer;
    }
    *real_size -= 1;
  }

  return 0;
}

/**************************************************************************************************************/
/**ACTUAL ALGORITHM: read one token, read a string, convert string to reverse polish notation, compute polish**/
/**************************************************************************************************************/

token_t read_token(const char * str, size_t pos, size_t size, size_t * token_size) {
  token_t res;
  res.valid = true;
  if (!isdigit(str[pos])){
    *token_size = 1;
    switch(str[pos]) {
      case '(':
        res.kind = TT_BRACE;
        res.brace = '(';
        break;
      case ')':
        res.kind = TT_BRACE;
        res.brace = ')';
        break;
      case '+':
        res.kind = TT_OPERATION;
        res.brace = '+';
        break;
      case '-': // just parsing as a special case now
        if (pos == 0 || pos == 1)
        res.kind = TT_OPERATION;
        res.brace = '-';
        break;
      case '*':
        res.kind = TT_OPERATION;
        res.brace = '*';
        break;
      case '/':
        res.kind = TT_OPERATION;
        res.brace = '/';
        break;
      default:
        res.valid = false;
        break;
    }
    return res;
  }
  size_t BUFFER_SIZE = 100;
  char buffer[BUFFER_SIZE];
  size_t start = pos;
  for (; pos < size && isdigit(str[pos]); ++pos);
  if (pos - start > BUFFER_SIZE - 1) {
    res.valid = false;
    return res;
  }
  strncpy(buffer, &str[start], pos-start);


}

bool is_operation(char c)
{
  return (c == OPK_PLUS || c == OPK_MINUS || c == OPK_MULT || c == OPK_DIV);
}

bool is_function(char c)
{
  return (c == OPK_PLUS || c == OPK_MINUS || c == OPK_MULT || c == OPK_DIV);
}

int read_line(char ** line, bool ignore_newline) {
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
      *line = malloc(chars_read * sizeof(char));
      if (*line == NULL)
        return -1;
      strcpy(*line, buffer);
    } else if (chars_read == BUFFER_SIZE - 2) // if line was allocated, the loop ends, so no need to free
      return -1;
    else
    {
      buffer[chars_read] = (char)x;
      ++chars_read;
    }
  }  while( x != EOF && (x != '\n' || ignore_newline));

  return chars_read;
}

int to_reverse_polish_notation(const char * expression, token_t * polish_expression, size_t size)
{
  int n_tokens;
  for (int i = 0; i < size; ++i) {

  }
}

int main() {
/*  double * stack = NULL;
  size_t max_size = 0, real_size = 0;
  push(&stack, 10.0, &real_size, &max_size);
  push(&stack, 11.0, &real_size, &max_size);
  push(&stack, 12.0, &real_size, &max_size);
  printf("%f %f %f %zd %zd\n", stack[0], stack[1], stack[2], real_size, max_size); // 10 11 12 3 3
  pop(&stack, &real_size, &max_size);
  printf("%f %f %zd %zd\n", stack[0], stack[1], real_size, max_size); // 10 11 2 3
  push(&stack, 13.0, &real_size, &max_size);
  push(&stack, 14.0, &real_size, &max_size);
  push(&stack, 15.0, &real_size, &max_size);
  push(&stack, 16.0, &real_size, &max_size);
  push(&stack, 17.0, &real_size, &max_size);
  push(&stack, 18.0, &real_size, &max_size);
  printf("%f %f %f %f  %f %f  %f %f %zd %zd\n", stack[0], stack[1], stack[2], stack[3],stack[4], stack[5], stack[6], stack[7], real_size, max_size); // 10 11 13 14 15 16 17 18 8 9
  pop(&stack, &real_size, &max_size);
  pop(&stack, &real_size, &max_size);
  pop(&stack, &real_size, &max_size);
  pop(&stack, &real_size, &max_size);
  pop(&stack, &real_size, &max_size);
  pop(&stack, &real_size, &max_size);
  printf("%f %f %zd %zd\n", stack[0], stack[1], real_size, max_size); // 10 11 2 3
  pop(&stack, &real_size, &max_size);
  pop(&stack, &real_size, &max_size);
  printf("%zd %zd\n", real_size, max_size); // 0 3
*/


  return 0;
}