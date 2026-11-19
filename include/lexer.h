#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum e_lexer_decision
{
    DC_START_WORD,
    DC_END_WORD,

    DC_START_OPERATOR,
    DC_OPERATOR_CHANGE,

    DC_START_TOKEN_WORD,
    DC_NEXT_TOKEN,
    DC_APPEND,
    DC_SKIP,
    DC_END,
    DC_ERROR
}   input_decision;

typedef enum e_char_type
{
    CHAR_NORMAL,
    CHAR_WHITESPACE,
    CHAR_SINGLE_QUOTE,
    CHAR_DOUBLE_QUOTE,
    CHAR_OPERATOR,
    CHAR_EOF,
    CHAR_NULL
}   t_char_type;

typedef enum e_token_type
{
    TOKEN_WORD,
    TOKEN_OPERATOR,
    TOKEN_NONE
} t_token_type;

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

typedef struct s_token
{
    t_token_type    token_type;
    bool            word_active;
    union
    {
        struct
        {
            size_t      start;
            size_t      length;
            t_word_type quote_state;
        };
        t_operator_type operator_type;
    };
    struct s_token   *next;
}   t_token;

typedef struct s_decision
{
    int dec_count;
    input_decision dec[3];
} i_decision;

int lex(char *buffer, char *lexer_buffer, size_t length, t_token **first_token);
void free_tokens(t_token *first);


#endif