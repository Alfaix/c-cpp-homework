operation_lambdas = {
    '-': lambda x, y : x - y,
    '+': lambda x, y : x + y,
    '*': lambda x, y : x * y,
    '/': lambda x, y : x / y,
    'r': lambda x : -x
    }

operators = ('-', '+', '*', '/')
functions = ('r', ) # all functions are unary
dual_operators_to_functions = {'-':'r'} # like.. reverse?
valid_chars = operators + functions + (')', '(', '.')

def priority(c):
    return 2 if c in functions else 1 if (c == '*' or c == '/') else 0

def is_valid(expr):
    if not expr: return False
    if not expr[0].isdigit() and expr[0] not in dual_operators_to_functions \
      and expr[0] not in functions and expr[0] != '(':
        print("First symbol is not what it should be")
        return False
    braces = 0
    dot_passed = False
    for i, c in zip(range(1,len(expr)-1), expr[1:-1]):
        if c not in valid_chars and not c.isdigit():
                print(f"Invalid character at position {i}: {c}")
                return False
        elif c == '.':
            if dot_passed or not expr[i-1].isdigit() or not expr[i+1].isdigit():
                print(f"Invalid dot at position {i}: {c}")
                return False
            dot_passed = True
        else:
            if not c.isdigit():
                dot_passed = False
            if c in operators: # two operators in a row
                if not expr[i-1].isdigit() and not expr[i-1] == ')': # operator can only follow a digit
                    # if the previous symbol isn't a digit, then it has to be a function and previous symbol has to be (
                    if c not in dual_operators_to_functions or (i != 0 and expr[i-1] != '('):
                        print(f"An operator not after a non-digit at position {i}: {c}")
                        return False
                else:
                    if expr[i-1] in operators or expr[i-1] in functions:
                        print(f"An operator after a non-digit at position {i}: {c}")
                        return False
            elif c == '(':
                braces += 1
            elif c == ')':
                braces -= 1
                if braces < 0:
                    print(f"Negative number of braces: closing brace at position {i}: {c}")
                    return False

    return braces == 0 and (expr[-1].isdigit() or expr[-1] == ')')





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
            elif (c in dual_operators_to_functions and 
                (i == 0 or not (expr[i-1].isdigit() or expr[i-1] == ')'))): # function or unary minus
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
    if not expr:
        return
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
        # print(tokens)
    return tokens[-1]

if __name__ == '__main__':
    expr = input('Enter expression: ').replace(' ', '')
    #assert is_valid(expr), "Expression is invalid!"
    print('Polish:', to_polish(expr))
    print("Solution:", compute_polish(to_polish(expr)))