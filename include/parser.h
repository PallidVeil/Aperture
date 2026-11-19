#ifndef PARSER_H
#define PARSER_H
#define MAX_COMMANDS 100

#include "lexer.h"
#include "common.h"

typedef enum e_command_operator
{
    CMD_END,
    CMD_PIPE,
    CMD_AND,
    CMD_OR
}   t_command_operator;

typedef struct s_command
{
    char                    **argv;
    size_t                  argc;
    t_command_operator      next_operator;
    struct s_command        *next;
}   t_command;

typedef struct s_parser
{
    t_token     *tokens;
    t_token     *active_parser;
    size_t      *word_counts;
    size_t      command_count;
    t_command   *commands;
}   t_parser;    

typedef enum e_parser_state
{
    PARSER_START,
    PARSER_COMMAND,
    PARSER_PIPE,
    PARSER_AND,
    PARSER_OR
}   t_parser_state;

#endif