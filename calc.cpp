#include "list.h"
#include "stack.h"
#include <cmath> // pow
#include <cstdlib>
#include <cstring> // strchr
#include <iomanip>
#include <iostream> // cout
#include <string>

#define EPSILON (float)1e-5

struct Token {
    enum Type { NUMBER, OPERATOR, UNARY_MINUS, LPAR, RPAR } type;
    float value;
    char op;

    Token(float p_value) : type(NUMBER), value(p_value), op(0) {}
    Token(char p_op) : type(OPERATOR), value(0), op(p_op) {}
    Token(Type p_type) : type(p_type), value(0), op(0) {}
    Token() : type(NUMBER), value(0), op(0) {}

    // for printing
    friend std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        switch (token.type) {
        case NUMBER: os << token.value; break;
        case OPERATOR:
        case UNARY_MINUS: os << token.op; break;
        case LPAR: os << "("; break;
        case RPAR: os << ")"; break;
        }
        return os;
    }
};

bool equals(float a, float b) { return std::abs(a - b) < EPSILON; }

// Parses a string into a list of tokens
// Throws an exception if the string is invalid
// Edge cases Handled:
//  - multiple decimal points in a number
//  - two numbers in a row
//  - two operators in a row
//  - operator at the beginning of the expression
//  - implicit multiplication
//  - unary operators
List<Token> tokenize(std::string str)
{
    List<Token> tokens; // output list

    for (size_t i = 0; i < str.length(); i++) {
        char ch = str[i];

        // unary operators (negative and positive numbers)
        if (i == 0 || tokens.last().type == Token::OPERATOR ||
            tokens.last().type == Token::LPAR) {
            if (ch == '+')
                continue;
            else if (ch == '-') {
                tokens.push_back(Token{Token::UNARY_MINUS});
                continue;
            }
        }

        switch (ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.': {
            // two numbers in a row or multiple decimal points in a number
            if (tokens.get_size() > 0 && tokens.last().type == Token::NUMBER)
                throw "Unexpected token at position " + std::to_string(i) +
                    ", expected OPERATOR or PARENTESES but found '" + ch + "'";

            size_t offset = 0;
            float val     = std::stof(str.substr(i), &offset);
            tokens.push_back(Token{val});
            // skip the rest of the number
            i += offset - 1;
            break;
        }
        case ' ': break;
        case '-':
        case '+':
        case '*':
        case '/':
        case '%':
        case '^': {
            // two operators in a row, or operator at the beginning of the
            // expression
            if (tokens.last().type == Token::OPERATOR ||
                tokens.last().type == Token::UNARY_MINUS || i == 0)
                throw "Unexpected token at position " + std::to_string(i) +
                    ", expected NUMBER but found '" + ch + "'";
            tokens.push_back(Token{ch});
            break;
        }
        case '(': {
            // implicit multiplication
            // 2(3+4) ==> 2*(3+4)
            // (3+4)(2+1) ==>(3+4)*(2+1)
            if (tokens.get_size() > 0 && (tokens.last().type == Token::RPAR ||
                                          tokens.last().type == Token::NUMBER))
                tokens.push_back(Token{'*'});

            tokens.push_back(Token{Token::LPAR});
            break;
        }
        case ')': {
            // close parenthesis after operator or open parenthesis.
            if (tokens.last().type == Token::OPERATOR ||
                tokens.last().type == Token::LPAR)
                throw "Unexpected token at position " + std::to_string(i) +
                    ", expected NUMBER but found ')'";
            tokens.push_back(Token{Token::RPAR});
            break;
        }
        default: {
            // invalid character
            throw "Invalid character at position " + std::to_string(i) + ": '" +
                ch + '\'';
        }
        }
    }

    return tokens;
}

// Converts an infix expression to postfix
// Throws an exception if parenthesis are mismatched
// other edge cases are handled by the tokenizer
List<Token> infix_to_postfix(List<Token> tokens_infix)
{
    List<Token> tokens_postfix; // output list
    Stack<Token> stack;         // operator stack

    for (size_t i = 0; i < tokens_infix.get_size(); i++) {
        Token token = tokens_infix[i];
        switch (token.type) {
        case Token::NUMBER: {
            tokens_postfix.push_back(token);
            break;
        }
        case Token::OPERATOR: {
            // operator precedence
            const char* precedence = "^%*/+-";
            while (!stack.is_empty() && stack.peek().type != Token::LPAR) {
                /*
                 * if the precedence of the operator on the stack is greater
                 * than or equal to the precedence of the operator in the
                 * input, pop the operator from the stack and add it to the
                 * output
                 */
                if (strchr(precedence, stack.peek().op) <=
                    strchr(precedence, token.op)) {
                    tokens_postfix.push_back(stack.pop());
                }
                else {
                    break;
                }
            }
            stack.push(token);
            break;
        }
        case Token::UNARY_MINUS: {
            // unary minus is the same as a negative number
            tokens_postfix.push_back(Token{(float)-1.0});
            stack.push(Token{'*'});
            break;
        }
        case Token::LPAR: {
            stack.push(token);
            break;
        }
        case Token::RPAR: {
            while (!stack.is_empty() && stack.peek().type != Token::LPAR)
                tokens_postfix.push_back(stack.pop());

            // mismatched parenthesis [close without open]
            if (stack.is_empty())
                throw std::string("Mismatched parenthesis");

            stack.pop();
        }
        }
    }
    while (!stack.is_empty()) {
        // mismatched parenthesis [open without close]
        if (stack.peek().type == Token::LPAR)
            throw std::string("Mismatched parenthesis");

        tokens_postfix.push_back(stack.pop());
    }
    return tokens_postfix;
}

// Evaluates a postfix expression
// Throws an exception if the expression is invalid or division by zero
float evaluate_postfix(List<Token> tokens)
{
    Stack<float> stack;

    for (size_t i = 0; i < tokens.get_size(); i++) {
        Token token = tokens[i];
        if (token.type == Token::OPERATOR) {
            // check if there are at least two values on the stack
            if (stack.get_size() < 2)
                throw std::string("Invalid expression");

            float val_r = stack.pop();
            float val_l = stack.pop();
            float out   = 0.0;

            switch (token.op) {
            case '-': out = val_l - val_r; break;
            case '+': out = val_l + val_r; break;
            case '*': out = val_l * val_r; break;
            case '%': {
                if ((int)val_r == 0)
                    throw std::string("Division by zero");
                out = (float)((int)val_l % (int)val_r);
                break;
            }
            case '/': {
                // check for division by zero
                if (equals(val_r, 0.0))
                    throw std::string("Division by zero");
                out = val_l / val_r;
                break;
            }
            case '^': {
                // 0^x is undefined for x <= 0
                if (equals(val_l, 0.0) && val_r <= EPSILON)
                    throw std::string("Division by zero");
                out = (float)pow(val_l, val_r);
                break;
            }
            default: throw std::string("Invalid operator");
            }
            stack.push(out);
        }
        else if (token.type == Token::NUMBER) {
            stack.push(token.value);
        }
    }

    if (stack.get_size() != 1)
        throw std::string("Invalid expression");
    return stack.pop();
}

void tests()
{
    struct Test {
        std::string expression;
        float result;
    };

    // tests
    Test tests[] = {
        {"2 + 3", (float)5},
        {"4 - 5", (float)-1},
        {"6 * 7", (float)42},
        {"8 / 9", (float)0.88888888888888884},
        {"(10 + 11) * 12", (float)252},
        {"(13 - 14) / 15", (float)-0.066666666666666666},
        {"(16 * 17) + 18", (float)290},
        {"(19 / 20) - 21", (float)-20.05},
        {"(22 + 23) * (24 - 25)", (float)-45},
        {"(26 / 27) + (28 * 29)", (float)812.962962963},
        {"(30 + 31) / (32 - 33)", (float)-61},
        {"(34 * 35) / (36 + 37)", (float)16.301369863013697},
        {"2.5 + 3.5", (float)6},
        {"4.5 - 5.5", (float)-1},
        {"6.5 * 7.5", (float)48.75},
        {"8.5 / 9.5", (float)0.89473684210526316},
        {"(10.5 + 11) * -12", (float)-258},
        {"(13 - -14) / 15", (float)1.8},
        {"(16 * -17) + 18", (float)-254},
        {"(-19 / 20) - -21", (float)20.05},
        {"(22 + -23) * (24 - -25)", (float)-49},
        {"(22 + -23) (24 - -25)", (float)-49},
        {"-(5)(-3)(2)", (float)30},
        {"(-26 / 27) + (28 * -29)", (float)-812.962962963},
        {"(-30 + 31) / (-32 - 33)", (float)-0.015384615384615385},
        {"(-34 * 35) / (-36 + 37)", (float)-1190},
        {"(38 + -39) * (40 - -41)", (float)-81},
        {"(-42 / -43) + 44", (float)44.9767441860465},
        {"-(5)*-(3)", (float)15.0},

    };

    bool err_flag = false;

    // test cases for infix to postfix
    for (Test i : tests) {
        try {
            std::cout << i.expression << " = ";
            List<Token> test = tokenize(i.expression);
            // std::cout << "Infix: ";
            // print_tokens(test);
            test = infix_to_postfix(test);
            // std::cout << "Postfix: ";
            // print_tokens(test);
            float result = evaluate_postfix(test);
            std::cout << std::left << std::setw(10) << result;
            if (result != i.result) {
                err_flag = true;
                std::cout << "ERROR: Expected " << i.result << std::endl;
            }
            else
                std::cout << std::right << std::setw(10) << "OK\n";
        }
        catch (std::string msg) {
            err_flag = true;
            std::cout << "ERROR: " << msg << std::endl;
        }
    }

    // invalid expressions

    std::string invalid_expressions[] = {
        "*1 + 2 + 3",   "2 +* 3",         "4 - 5 /",
        "6 * 7 +",      "8 / 9 -",        "(38 + 39) * (40 - )",
        "(41 / ) - 42", "(43 a 44) + 45", "(46 +* 47) * (48 - 49)",
        "5.3.3",        "((55",           "56))",
        "0 / 0",        "0 ^ 0",          "0 ^ -1",
        "1/0",
    };

    for (std::string i : invalid_expressions) {
        try {
            std::cout << i << " = ";
            List<Token> test = tokenize(i);
            // std::cout << "Infix: ";
            // print_tokens(test);
            test = infix_to_postfix(test);
            // std::cout << "Postfix: ";
            // print_tokens(test);
            float result = evaluate_postfix(test);
            // should not reach here
            std::cout << "Result: " << result << "\t";
            err_flag = true;
            std::cout << "ERROR: Expected exception" << std::endl;
        }
        catch (std::string msg) {
            std::cout << "Exception: " << msg << std::endl;
        }
    }

    if (err_flag)
        std::cout << "Some tests failed" << std::endl;
    else
        std::cout << "All tests passed" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: calc [<expression> | run_tests]" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "run_tests") {
        tests();
        return 0;
    }

    try {
        List<Token> tokens_infix   = tokenize(argv[1]);
        List<Token> tokens_postfix = infix_to_postfix(tokens_infix);
        float result               = evaluate_postfix(tokens_postfix);
        std::cout << argv[1] << " = " << result << std::endl;

        std::cout << "postfix: " << tokens_postfix;
    }
    catch (std::string e) {
        std::cout << e << std::endl;
        return 1;
    }

    return 0;
}
