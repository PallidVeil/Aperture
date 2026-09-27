#include "common.h"
#include "lexer.h"
#include "editor.h"

Token tokens[MAX_TOKENS];

void TokenAppend(char* t_buffer, char input,size_t i)
{
    t_buffer[i] = input;
}

void TokenEnd(char* t_buffer,size_t* tokenCount,size_t i)
{
    t_buffer[i] = "\0"
    (*tokenCount)++;
}

void TokenStartDoubleQ(size_t i)
{
    tokens->type = TOKEN_DOUBLE_QUOTE;
    tokens->start = i;
}

void TokenStartWord(size_t i)
{
    tokens->type = TOKEN_WORD;
    tokens->start = i;
}

void TokenStartSingleQ(size_t i)
{
    tokens->type = TOKEN_SINGLE_QUOTE;
    tokens->start = i;
}

void TokenChangeType(TokenType TokenTypeToChange)
{
    // it onyl changes between word to single or double quote so i will only take the type im going to change it to
    tokens->type = TokenTypeToChange;
}

void handleDoubleQuote(char* t_buffer, size_t* tokenCount, char input, size_t i)
{
    if(tokens->type == NULL)
        TokenStartDoubleQ(i);
    else if (tokens->type == TOKEN_WORD)
        TokenChangeType(TOKEN_DOUBLE_QUOTE);
    else if(tokens->type == TOKEN_DOUBLE_QUOTE)
        TokenEnd(t_buffer, tokenCount,i);
    else
        TokenAppend(t_buffer, input,i);
}

void handleSpace(char* t_buffer, size_t* tokenCount, char input, size_t i)
{
    if(tokens->type == TOKEN_SINGLE_QUOTE || tokens->type == TOKEN_DOUBLE_QUOTE)
        TokenAppend(t_buffer, input,i);
    else if(tokens->type == TOKEN_WORD)
        TokenEnd(t_buffer, tokenCount,i);
}

void handleSingleQuote(char* t_buffer, size_t* tokenCount, char input, size_t i)
{
    if(tokens->type == NULL)
        TokenStartDoubleQ(i);
    else if (tokens->type == TOKEN_WORD)
        TokenChangeType(TOKEN_SINGLE_QUOTE);
    else if(tokens->type == TOKEN_DOUBLE_QUOTE)
        TokenEnd(t_buffer, tokenCount,i);
    else
        TokenAppend(t_buffer, input,i);
}

void handleNormalInput(char* t_buffer, size_t* tokenCount, char input, size_t i)
{
    if(tokens->type == NULL)
    {
        TokenStartWord(i);
        TokenAppend(t_buffer, input,i);
    } else
        TokenAppend(t_buffer, input,i)
}

void handleOperator(char* t_buffer, size_t* tokenCount, char input, size_t i)
{
    //it is empty for now
}

bool isInputAnOperator(char input)
{
    // will change it on the next commit
    return 0;
}

void handleToken(char* t_buffer, size_t* tokenCount, char input, size_t i)
{
    // operators will start and end within the same time so i dont handle it on the handle functions
    if(input == "\"")
        handleDoubleQuote(t_buffer,tokenCount,input, i);
    else if(input == "'")
        handleSingleQuote(t_buffer,tokenCount,input, i);
    else if(input == " ")
        handleSpace(t_buffer,tokenCount,input, i);
    else if(isInputAnOperator(input))
        handleOperator(t_buffer,tokenCount,input, i);
    else 
        handleNormalInput(t_buffer,tokenCount,input, i);
}

int lexer(char* buffer,char* lexer_buffer ,Editor* editor)
{
    size_t i = 0;
    size_t tokenCount = 0;

    while (i < editor->length)
    {
        handleToken(lexer_buffer,&tokenCount, buffer[i],i);
        i++;
    }
}