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

void TokenEnd(char *t_buffer, size_t i)
{
    t_buffer[i] = '\0';
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

void end_word(t_word *word, size_t i)
{
    word->length = i - word->start;
    lexer->active_word = lexer->active_word->next;
}

void start_word(t_word *word, t_word_type type, size_t start)
{
    if(lexer->active_word == NULL)
        lexer->active_word = create_word(type, start);
}

void create_new_word(t_word *word, t_word_type type, size_t start)
{
    word->nextword = create_word(type, start);
    if (!word->nextword)
        return;
    word = word->nextword;
}
/*
void TokenStartDoubleQ(size_t i, t_word *word)
{
    word->type = PART_DOUBLE_QUOTE;
    word->start = i;
}

void TokenStartNormalWord(size_t i,t_word *word)
{
    word->type = TOKEN_WORD;
    word->start = i;
}

void TokenStartSingleQ(size_t i,t_word *word)
{
    word->type = PART_SINGLE_QUOTE;
    word->start = i;
}

void TokenStartWord(size_t i, t_token *token)
{
    token->type = TOKEN_WORD;   
}

void TokenStartOperator(char input, t_token *token)
{
    token->type = TOKEN_OPERATOR;

    if (input == '|')
        token->operator_type = OP_PIPE;
    else if (input == '&')
        token->operator_type = OP_BACKGROUND;
    else if (input == '<')
        token->operator_type = OP_REDIR_IN;
    else if (input == '>')
        token->operator_type = OP_REDIR_OUT;

    return;
}

void TokenEnd(t_token *token, char *t_buffer, size_t i)
{
    t_buffer[i] = '\0';
    token->next = create_token();
    if (!token->next)
        return;
    token = token->next;
}

void TokenChangeOPType(t_operator_type TokenTypeToChange, size_t i, t_token *token)
{
    token->operator_type = TokenTypeToChange;
}

void handleDoubleQuote(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == NULL)
        TokenStartDoubleQ(i, word);
    else if (tokens[i].type == TOKEN_WORD)
        TokenChangeOPType(PART_DOUBLE_QUOTE, i, token);
    else if (tokens[i].type == PART_DOUBLE_QUOTE)
        TokenEnd(t_buffer, tokenCount, i);
    else
        TokenAppend(t_buffer, input, i);
}

void handleSpace(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == PART_SINGLE_QUOTE || tokens[i].type == PART_DOUBLE_QUOTE)
        TokenAppend(t_buffer, input, i);
    else if (tokens[i].type == TOKEN_WORD)
        TokenEnd(t_buffer, tokenCount, i);
}

void handleSingleQuote(char *t_buffer, size_t *tokenCount, char input, size_t i)
{
    if (tokens[i].type == NULL)
        TokenStartDoubleQ(i);
    else if (tokens[i].type == TOKEN_WORD)
        TokenChangeType(PART_SINGLE_QUOTE, i);
    else if (tokens[i].type == PART_DOUBLE_QUOTE)
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

t_operator_type CheckTokenMatch(char input, t_operator_type LastToken)
{
    if (input == '|' && LastToken == OP_PIPE)
        return OP_OR;
    else if (input == '&' && LastToken == OP_BACKGROUND)
        return OP_AND;
    else if (input == '>' && LastToken == OP_REDIR_OUT)
        return OP_APPEND;
    else if (input == '<' && LastToken == OP_REDIR_IN)
        return OP_HEREDOC;

    return -1;
}

input_decision handleOperator(t_token *token, char *t_buffer, char input, size_t i)
{
    // im completely lost
    
    if ()
    switch (token->operator_type)
    {
    case OP_NONE:
        return TOKEN
        break;
    case PART_DOUBLE_QUOTE:
    case PART_SINGLE_QUOTE:
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

input_decision Decide_Token(char *t_buffer, size_t *tokenCount, char input, size_t i)
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
    */

input_decision lexer_read(char *buffer, char *lexer_buffer, size_t *i, t_token *token, t_word *word)
{
    // what i have in my mind is that if there is no active word the function ill allocate a new one if the word ends 
    // we do activeword = activeword-> nextword and if the token is finished we will end the token and start a new one
    char input = buffer[*i];

    t_token_type token_type = check_token_type(token);
    t_word_type word_type;
    if (token_type == TOKEN_WORD)
        word_type = check_word_type(word);

    if(token_type == TOKEN_NONE)
    {
        if (isInputAnOperator(input))
            return DE_OP_START;
        else
            return handleInputTypes(token_type, word_type, input); // it needs to check if its space or not then it needs to start a new word in this token
    }
    else if(token_type == TOKEN_OPERATOR)
    {
        if (isInputAnOperator(input))
            return DE_OP_CHANGEHANDLER;
        else
            return handleInputTypes(token_type, word_type, input); // it needs to end the token and start a new token and if it needs it needs to create
        // a new word too
    }
    else if(token_type == TOKEN_WORD)
    {
        return handleInputTypes(token_type, word_type, input); // it needs to check if the active word situation whether the word is finished or not then 
    }

    return DE_ERROR;
}

input_decision handleInputTypes(t_token_type token_type, t_word_type word_type, char input)
{
    switch (token_type)
    {
    case TOKEN_NONE:
        if (input == ' ')
            return DE_SKIP;
        else
            return DE_TOKEN_WORD_START;
        break;
    case TOKEN_OPERATOR:
        if(input == ' ')
            return DE_TOKEN_ENDSTART;
        else
            return DE_TOKEN_WORD_ENDSTART;
        break;
    case TOKEN_WORD:
        switch (word_type)
        {
        case WORD_NONE:
            if (input == ' ')
                return DE_SKIP;
            else
                return DE_WORD_START;
            break;
        case WORD_NORMAL:
            if (input == ' ')
                return DE_WORD_END;
            else if (input == '"' || input == '\'')
                return DE_WORD_ENDSTART;
            else
                return DE_APPEND;
            break;
        case WORD_SINGLE_QUOTE:
            if (input == '\'')
                return DE_WORD_ENDSTART;
            else
                return DE_APPEND;
            break;
        case WORD_DOUBLE_QUOTE:
            if (input == '"')
                return DE_WORD_ENDSTART;
            else
                return DE_APPEND;
            break;
        case WORD_FINISHED:
            if (input == ' ')
                return DE_TOKEN_ENDSTART;
            else if (input == '"' || input == '\'')
                return DE_WORD_ENDSTART;
            else
                return DE_APPEND;
            break;
        }
    }       
}


t_token_type check_token_type()
{
    t_token *token = lexer->active_token;
    if (token == NULL)
        return TOKEN_NONE;
    if (token->type == TOKEN_NONE)
        return TOKEN_NONE;
    else if (token->type == TOKEN_WORD)
        return TOKEN_WORD;
    else if (token->type == TOKEN_OPERATOR)
        return TOKEN_OPERATOR;
    return TOKEN_NONE;
}

t_word_state check_word_type()
{
    t_word *word = lexer->active_word;
    if (word == NULL)
        return WORD_NONE;
    else if (word->length != 0)
        return WORD_FINISHED;
    else if (word->type == WORD_NORMAL)
        return WORD_NORMAL;
    else if (word->type == WORD_SINGLE_QUOTE)
        return WORD_SINGLE_QUOTE;
    else if (word->type == WORD_DOUBLE_QUOTE)
        return WORD_DOUBLE_QUOTE;
    return WORD_NONE;
}

int lexer(char *buffer, char *lexer_buffer, Editor *editor)
{
    lexer->active_token = create_token();
    t_token *first_token;
    first_token = lexer->active_token;
    size_t i = 0;
    size_t tokenCount = 0;

    while (i < editor->length)
    {   

        i++;

        // decide token function will return the decision and then i will put a switch case and handle it 
        // its starting to get complicated so much

        /* 
          the program probably will take a active token so we need to decide what to do later that 
          for example we can check if the tokens word are full or if we encounter the type of tokens word is then later dexide what to do
          
          we need to write a checking funciton that returns if token is empty operator or a word 
          then we'll decide what to do after that 

          we need an end and start function it will first end the old token then it will start a new one by looking into the type of input
          we need end word and start a new one funciton too
          we need a checking funciton for us to check the last word of token
          we need an append function
          we need a op change function
        
        */
    }
    if (token-> != NULL)
        TokenEnd(lexer_buffer, &tokenCount, i);

    free_tokens(first);
    return 1;
}