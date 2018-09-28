operation_lambdas = {
    '-': lambda x, y : x - y,
    '+': lambda x, y : x + y,
    '*': lambda x, y : x * y,
    '/': lambda x, y : x / y,
    'r': lambda x : -x
    }

operators = ('-', '+', '*', '/')
functions = ('r') # all functions are unary
dual_operators_to_functions = {'-':'r'} # like.. reverse?

def priority(c):
    return 2 if c in functions else 1 if (c == '*' or c == '/') else 0

def to_polish(expr):
    tokens = []
    out = []

    reading_number = False
    first_digit = -1
    for i, c in enumerate(expr):
        print("Tokens:", tokens)
        print("Out:", out)
        if c.isdigit() or c == '.':
            if not reading_number:
                reading_number = True
                first_digit = i
        else:
            if reading_number:
                out.append(float(expr[first_digit:i]))
                reading_number = False
            if c in functions:
                tokens.append(c) 
            elif (c in dual_operators_to_functions and (i == 0 or not expr[i-1].isdigit())): # function or unary minus
                tokens.append(dual_operators_to_functions[c]) 
            elif c in operators:
                while tokens and (tokens[-1] in operators \
                             or tokens[-1] in dual_operators_to_functions.values()) \
                             and priority(c) <= priority(tokens[-1]):
                    out.append(tokens.pop())
                tokens.append(c)
            elif c == '(':
                tokens.append(c)
            elif c == ')':
                while tokens[-1] != '(': # throws out of range if tokens = [] (odd # of braces)
                    out.append(tokens.pop())
                tokens.pop()
                if tokens and tokens[-1] in functions:
                    out.append(tokens.pop())
            else:
                raise ValueError(f'Encountered something ({c}) that isn\'t 0-9+-*/()')
    if reading_number:
        out.append(float(expr[first_digit:]))
    while tokens:
        if tokens[-1] in '()':
            raise ValueError('Braces aren\'t closed')
        else:
            out.append(tokens.pop())

    return out

def compute_polish(expr):
    tokens = []
    for i, c in enumerate(expr):
        if type(c) == float:
            tokens.append(c)
        elif c in functions:
            tokens.append(operation_lambdas[c](tokens.pop()))
        elif c in operators:
            tmp = operation_lambdas[c](tokens[-2], tokens[-1])
            tokens = tokens[:-2]
            tokens.append(tmp)
        else:
            raise ValueError(f"Unexpected {c}")
        print(tokens)
    return tokens[-1]

if __name__ == '__main__':
    expr = input('Enter expression: ').replace(' ', '')
    print('Polish:', to_polish(expr))
    print("Solution:", compute_polish(to_polish(expr)))