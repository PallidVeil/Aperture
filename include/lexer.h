#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKENS 256

typedef enum e_lexer_type
{
    LEXER_WORD,
    LEXER_OPERATOR
} t_lexer_type;

typedef enum e_word_type
{
    PART_NORMAL,
    PART_SINGLE_QUOTE,
    PART_DOUBLE_QUOTE
} t_word_type;

typedef enum e_operator_type
{
    OP_PIPE,
    OP_REDIR_IN,
    OP_REDIR_OUT,
    OP_BACKGROUND,
    OP_APPEND,
    OP_HEREDOC,
    OP_AND,
    OP_OR,
    OP_NONE
} t_operator_type;


typedef struct s_token
{
    char            *buffer;
    size_t          length;
    t_word_type     type;
}   t_token;

typedef struct s_lexer
{
    t_lexer_type    type;
    t_token         *token;
    t_operator_type operator_type;
    struct s_lexer  *next;
} t_lexer;

#endif