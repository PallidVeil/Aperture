#include "common.h"
#include "lexer.h"
#include "editor.h"

/*
 * there is so many things i need to change in lexer
 * first i need to make only two typse for token word and pipe
 * second i need to add dq sq and normal word in word category
 * the lexer should have many types of word in one word and 
 * if the word is gonna end it should end with either pipe or quotes
 * 
 * im planning to remove lexer buffer and move it to ane struct or a type of struct
 * whats pecial about this struct is that it can crow on command so that 
 * shell doesnt have to mess with token start or token end or any kinds of things
 * 
 * and i want to change the reader part too 
 * right now it does one single thing in one time
 * looks at the char checks what it is and then it falls it into 5 categories 
 * in my next "attempt" i want to make it the same but change the one char  thing to
 *  one word and look for end in only main searching func 
 * 
 * i will try to make the struct on this commit and will change the lexer in the other commits 
*/

void free_tokens(t_token *first)
{
    t_token *curr;
    t_token *next;

    curr = first;
    while (curr)
    {
        free_words(curr->token);

        next = curr->next;
        free(curr);
        curr = next;
    }
}

void free_words(t_word *first)
{
    t_word *curr;
    t_word *next;

    curr = first;
    while (curr)
    {
        next = curr->nextword;
        free(curr);
        curr = next;
    }
}

void TokenAppend(char *t_buffer, char input, size_t i)
{
    t_buffer[i] = input;
}

void TokenEnd(char *t_buffer, size_t *tokenCount, size_t i)
{
    t_buffer[i] = '\0';
    (*tokenCount)++;
}

t_token *create_token()
{
    t_token *new = malloc(sizeof(t_token));
    if (!new)
        return NULL;
    new->type = TOKEN_NONE;
    new->token = NULL;
    new->operator_type = OP_NONE;
    new->next = NULL;
    return new;
}

t_word *create_word(t_word_type type,size_t start)
{
    t_word *new = malloc(sizeof(t_word));
    if (!new)
        return NULL;
    new->start = start;
    new->length = 0;
    new->type = type;
    new->nextword = NULL;
    return new;
}

void TokenStartDoubleQ(size_t i)
{
    tokens[i].type = TOKEN_DOUBLE_QUOTE;
    tokens[i].start = i;
}

void TokenStartWord(size_t i)
{
    tokens[i].type = TOKEN_WORD;
    tokens[i].start = i;
}

void TokenStartSingleQ(size_t i)
{
    tokens[i].type = TOKEN_SINGLE_QUOTE;
    tokens[i].start = i;
}

void TokenStartOperator(char input, size_t i)
{
    tokens[i].type = TOKEN_OPERATOR;

    if (input == '|')
        tokens[i].operator = TOKEN_PIPE;
    else if (input == '&')
        tokens[i].operator = TOKEN_BACKGROUND;
    else if (input == '<')
        tokens[i].operator = TOKEN_REDIR_IN;
    else if (input == '>')
        tokens[i].operator = TOKEN_REDIR_OUT;

    return;
}

void TokenChangeType(TokenType TokenTypeToChange, size_t i)
{
    tokens[i].type = TokenTypeToChange;
}

void handleDoubleQuote(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == NULL)
        TokenStartDoubleQ(i);
    else if (tokens[i].type == TOKEN_WORD)
        TokenChangeType(TOKEN_DOUBLE_QUOTE, i);
    else if (tokens[i].type == TOKEN_DOUBLE_QUOTE)
        TokenEnd(t_buffer, tokenCount, i);
    else
        TokenAppend(t_buffer, input, i);
}

void handleSpace(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == TOKEN_SINGLE_QUOTE || tokens[i].type == TOKEN_DOUBLE_QUOTE)
        TokenAppend(t_buffer, input, i);
    else if (tokens[i].type == TOKEN_WORD)
        TokenEnd(t_buffer, tokenCount, i);
}

void handleSingleQuote(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == NULL)
        TokenStartDoubleQ(i);
    else if (tokens[i].type == TOKEN_WORD)
        TokenChangeType(TOKEN_SINGLE_QUOTE, i);
    else if (tokens[i].type == TOKEN_DOUBLE_QUOTE)
        TokenEnd(t_buffer, tokenCount, i);
    else
        TokenAppend(t_buffer, input, i);
}

void handleNormalInput(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == NULL)
    {
        TokenStartWord(i);
        TokenAppend(t_buffer, input, i);
    }
    else
        TokenAppend(t_buffer, input, i);
}

TokenOperator CheckTokenMatch(char input, TokenOperator LastToken)
{
    if (input == '|' && LastToken == TOKEN_PIPE)
        return TOKEN_OR;
    else if (input == '&' && LastToken == TOKEN_BACKGROUND)
        return TOKEN_AND;
    else if (input == '>' && LastToken == TOKEN_REDIR_OUT)
        return TOKEN_APPEND;
    else if (input == '<' && LastToken == TOKEN_REDIR_IN)
        return TOKEN_HEREDOC;

    return -1;
}

void handleOperator(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    switch (tokens[i].type)
    {
    case TOKEN_NONE:
        TokenStartOperator(input, i);
        break;
    case TOKEN_DOUBLE_QUOTE:
    case TOKEN_SINGLE_QUOTE:
        TokenAppend(t_buffer, input, i);
        break;
    case TOKEN_WORD:
        TokenEnd(t_buffer, tokenCount, i);
        TokenStartOperator(input, i);
        break;
    case TOKEN_OPERATOR:
        TokenOperator match = CheckTokenMatch(input, tokens[i].operator);
        if (match == -1)
            TokenEnd(t_buffer, tokenCount, i);
        else
            TokenChangeType(match, i);
        break;
    }
    return;
}

bool isInputAnOperator(char input)
{
    return (input == '|' ||
            input == '<' ||
            input == '>' ||
            input == '&' ||
            input == ';' ||
            input == '(' ||
            input == ')');
}

void handleToken(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    // operators will start and end within the same time so i dont handle it on the handle functions
    if (isInputAnOperator(input))
    {
        handleOperator(t_buffer, tokenCount, input, i);
        return;
    }
    switch (input)
    {
    case ' ':
        handleSpace(t_buffer, tokenCount, input, i);
        break;
    case '"':
        handleDoubleQuote(t_buffer, tokenCount, input, i);
        break;
    case '\'':
        handleSingleQuote(t_buffer, tokenCount, input, i);
        break;
    default:
        handleNormalInput(t_buffer, tokenCount, input, i);
    }
}

int lexer(char *buffer, char *lexer_buffer, Editor *editor)
{
    t_token *first = create_token();

    size_t i = 0;
    size_t tokenCount = 0;

    while (i < editor->length)
    {
        handleToken(lexer_buffer, &tokenCount, buffer[i], i);
        i++;
    }
    if (tokens[i].type != NULL)
        TokenEnd(lexer_buffer, &tokenCount, i);

    free_tokens(first);
    return 1;
}