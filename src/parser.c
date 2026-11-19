#include "parser.h"

void parser_init(t_parser *parser, t_token *tokens)
{
    parser->tokens = tokens;
    parser->active_parser = tokens;
    parser->word_counts = malloc(sizeof(size_t) * MAX_COMMANDS);
    if (parser->word_counts == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for word counts\n");
        exit(EXIT_FAILURE);
    }
    parser->command_count = 0;
    parser->commands = NULL;

}

void parser_next(t_parser *parser)
{
    if (parser->active_parser != NULL)
        parser->active_parser = parser->active_parser->next;
}

void parser_free(t_parser *parser)
{
    free(parser->word_counts);
    parser->word_counts = NULL;
    parser->tokens = NULL;
    parser->active_parser = NULL;
    parser->command_count = 0;
}

void init_command(t_command *command)
{
    command->argc = 0;
    command->argv = NULL;
    command->next = NULL;
}

bool parser_validate_word(t_parser *parser, t_parser_state last_state)
{
    return true;
}

bool parser_validate_pipe(t_parser *parser, t_parser_state last_state)
{
    if (last_state == PARSER_START || last_state == PARSER_PIPE || last_state == PARSER_AND || last_state == PARSER_OR)
        return false;
    return true;
}

bool parser_validate_and(t_parser *parser, t_parser_state last_state)
{
    if (last_state == PARSER_START || last_state == PARSER_PIPE || last_state == PARSER_AND || last_state == PARSER_OR)
        return false;
    return true;
}

bool parser_validate_or(t_parser *parser, t_parser_state last_state)
{
    if (last_state == PARSER_START || last_state == PARSER_PIPE || last_state == PARSER_AND || last_state == PARSER_OR)
        return false;
    return true;
}

void parser_validate(t_parser *parser)
{
    t_parser_state last_state = PARSER_START;
    while(parser->active_parser != NULL)
    {
        bool parser_error = false;
        switch (parser->active_parser->token_type)
        {
            case TOKEN_WORD:
                parser_error = parser_validate_word(parser, last_state);
                last_state = PARSER_COMMAND;
            break;
            case TOKEN_OPERATOR:
                switch (parser->active_parser->operator_type)
                {
                    case OP_PIPE:
                        parser_error =  parser_validate_pipe(parser, last_state);
                        last_state = PARSER_PIPE;
                    break;
                    case OP_AND:
                        parser_error = parser_validate_and(parser, last_state);
                        last_state = PARSER_AND;
                    break;
                    case OP_OR:
                        parser_error = parser_validate_or(parser, last_state);
                        last_state = PARSER_OR;
                    break;
                    default:
                        fprintf(stderr, "Invalid operator type\n");
                        exit(EXIT_FAILURE);
                }
            break;
            default:
                fprintf(stderr, "Invalid token type\n");
                exit(EXIT_FAILURE);
        }
        if (!parser_error)
        {
            fprintf(stderr, "Syntax error: invalid token sequence\n");
            exit(EXIT_FAILURE);
        }
        parser_next(parser);
    }
    if (last_state == PARSER_PIPE
    || last_state == PARSER_AND
    || last_state == PARSER_OR)
    {
    // Syntax error
        fprintf(stderr, "Syntax error: unexpected end of input\n");
        exit(EXIT_FAILURE);
    }
}

void parser_command_count(t_parser *parser)
{
    parser->command_count = 1;

    while (parser->active_parser != NULL)
    {
        if (parser->active_parser->token_type == TOKEN_WORD)
        {
            parser->word_counts[parser->command_count - 1]++;
        }
        else if (parser->active_parser->token_type == TOKEN_OPERATOR)
        {
            if (parser->active_parser->operator_type == OP_PIPE
                || parser->active_parser->operator_type == OP_AND
                || parser->active_parser->operator_type == OP_OR)
            {
                parser->command_count++;
            }
        }
        parser_next(parser);
    }
}

void parser_allocate_commands(t_parser *parser)
{
    t_command *last_command = NULL;

    for (int i = 0; i < parser->command_count; i++)
    {
        t_command *new_command = malloc(sizeof(t_command));
        if (new_command == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for command\n");
            exit(EXIT_FAILURE);
        }

        init_command(new_command);

        new_command->argc = parser->word_counts[i];
        new_command->argv = malloc(sizeof(char *) * (new_command->argc + 1));

        if (new_command->argv == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for command arguments\n");
            exit(EXIT_FAILURE);
        }

        new_command->argv[new_command->argc] = NULL;

        if (parser->commands == NULL)
            parser->commands = new_command;
        else
            last_command->next = new_command;

        last_command = new_command;
    }
}

void parser_populate_commands(t_parser *parser)
{
    int argc_index = 0;
    t_command *active_command = parser->commands;

    while (parser->active_parser != NULL)
    {
        if (parser->active_parser->token_type == TOKEN_WORD)
        {
            if (argc_index >= active_command->argc)
            {
                active_command = active_command->next;
                argc_index = 0;
            }

            active_command->argv[argc_index] =
                strdup(parser->active_parser->value);
            argc_index++;
        }
        parser_next(parser);
    }
}

void parser_run(t_parser *parser)
{
    t_command *first_command = parser->commands = malloc(sizeof(t_command));
    parser->commands = first_command;
    if (first_command == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for first command\n");
        exit(EXIT_FAILURE);
    }
    init_command(first_command);

    parser_validate(parser);
    parser_active_parser = parser->tokens;
    parser_command_count(parser);
    parser->active_parser = parser->tokens;
    parser_allocate_commands(parser);
    parser_populate_commands(parser);
}