#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    DE_SKIP,
    DE_APPEND,
    DE_TOKEN_START,
    DE_TOKEN_END,
    DE_WORD_START,
    DE_WORD_END,

    DE_WORD_END_WORD_START,
    DE_WORD_END_TOKEN_START,

    DE_TOKEN_END_OP_START,
    DE_OP_START,
    DE_OP_END,
    DE_OP_CHANGEHANDLE,
    DE_OP_END_WORD_START,

    DE_ERROR
}input_decision;

typedef enum e_token_type
{
    TOKEN_WORD,
    TOKEN_OPERATOR,
    TOKEN_NONE
} t_token_type;

typedef struct s_lexer
{
    t_token *active_token;
    t_word  *active_word;
}   t_lexer;

typedef enum e_word_type
{
    WORD_NORMAL,
    WORD_SINGLE_QUOTE,
    WORD_DOUBLE_QUOTE,
    WORD_NULL,
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

t_lexer *lexer;
#endif