#ifndef LEXER_H
#define LEXER_H

typedef enum e_token_type
{
    TOKEN_WORD,
    TOKEN_OPERATOR,
    TOKEN_NONE
} t_token_type;

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


typedef struct s_word
{
    size_t          start;
    size_t          length;
    t_word_type     type;
    struct s_word   *nextword;
}   t_word;

typedef struct s_token
{
    t_token_type    type;
    t_word          *token;
    t_operator_type operator_type;
    struct s_token  *next;
} t_token;

#endif