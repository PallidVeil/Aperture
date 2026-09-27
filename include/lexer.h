#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKENS 256

typedef enum
{
    TOKEN_WORD,
    TOKEN_SINGLE_QUOTE,
    TOKEN_DOUBLE_QUOTE,
    TOKEN_OPERATOR
} TokenType;

typedef struct
{
    TokenType type;
    int start;
    TokenOperator operator;
} Token;

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIR_IN,
    TOKEN_REDIR_OUT,
    TOKEN_APPEND,
    TOKEN_HEREDOC,
    TOKEN_AND,
    TOKEN_OR
} TokenOperator;

#endif