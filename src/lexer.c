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
    lexer->active_token = lexer->active_token->next;
    lexer->active_word = NULL;
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

void start_token(t_token *token, size_t start)
{
    if(lexer->active_token == NULL)
        lexer->active_token = create_token();
}

void create_new_word(t_word *word, t_word_type type, size_t start)
{
    word->nextword = create_word(type, start);
    if (!word->nextword)
        return;
    word = word->nextword;
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

input_decision lexer_read(char *buffer, char *lexer_buffer, size_t *i, t_token *token, t_word *word)
{
    // what i have in my mind is that if there is no active word the function ill allocate a new one if the word ends 
    // we do activeword = activeword-> nextword and if the token is finished we will end the token and start a new one
    char input = buffer[*i];

    t_token_type token_type = check_token_type(token);
    t_word_type word_type;
    if (word != NULL)
        word_type = check_word_type(word);
    else
        word_type = WORD_NULL;

    
    return handleInputTypes(token_type, word_type, input);
}

input_decision handleInputTypes(t_token_type token_type, t_word_type word_type, char input)
{
    switch (token_type)
    {
        case TOKEN_NONE:
            if (isInputAnOperator(input))
                return DE_OP_START;
            else
                switch (input)
                {
                    case ' ':
                        return DE_SKIP;
                    case '"':
                        return DE_WORD_START;
                    case '\'':
                        return DE_WORD_START;
                    default:
                        return DE_WORD_START;
                }
        case TOKEN_WORD:
            switch (word_type)
                {
                    case WORD_NULL:
                        if (isInputAnOperator(input))
                            return DE_WORD_END_OP_START;
                        else
                            switch (input)
                            {
                                case ' ':
                                    return DE_WORD_END;
                                case '"':
                                    return DE_WORD_START;
                                case '\'':
                                    return DE_WORD_START;
                                default:
                                    return DE_APPEND;
                            }
                    case WORD_NORMAL:
                        if (isInputAnOperator(input))
                            return DE_WORD_END_OP_START;
                        else
                            switch (input)
                            {
                                case ' ':
                                    return DE_WORD_END;
                                case '"':
                                    return DE_WORD_END_WORD_START;
                                case '\'':
                                    return DE_WORD_END_WORD_START;
                                default:
                                    return DE_APPEND;
                            }
                    case WORD_SINGLE_QUOTE:
                            switch (input)
                            {
                                case '\'':
                                    return DE_WORD_END;
                                default:
                                    return DE_APPEND;
                            }
                    case WORD_DOUBLE_QUOTE:
                            switch (input)
                            {
                                case '"':
                                    return DE_WORD_END;
                                default:
                                    return DE_APPEND;
                            }
                }
        case TOKEN_OPERATOR:
            if (isInputAnOperator(input))
                return DE_OP_CHANGEHANDLE;
            else
                switch (input)
                {
                    case ' ':
                        return DE_OP_END;
                    case '"':
                        return DE_OP_END_WORD_START;
                    case '\'':
                        return DE_OP_END_WORD_START;
                    default:
                        return DE_OP_END_WORD_START;
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

void exec_decision(input_decision decision, char input, char *lexer_buffer, size_t *i)
{
    switch (decision)
    {
        case DE_SKIP:
            break;
        case DE_APPEND:
            TokenAppend(lexer_buffer, input, *i);
            break;
        case DE_TOKEN_START:
            TokenStartWord(*i, lexer->active_token);
            break;
        case DE_TOKEN_END:
            TokenEnd(lexer_buffer, *i);
            break;
        case DE_WORD_START:
            start_word(lexer->active_word, WORD_NORMAL, *i);
            switch (input)
            {
                case '"':
                    lexer->active_word->type = WORD_DOUBLE_QUOTE;
                    break;
                case '\'':
                    lexer->active_word->type = WORD_SINGLE_QUOTE;
                    break;
                default:
                    lexer->active_word->type = WORD_NORMAL;
                    TokenAppend(lexer_buffer, input, *i);
                    break;
            }
            break;
        case DE_WORD_END:
            end_word(lexer->active_word, *i);
            break;
        case DE_WORD_END_WORD_START:
            end_word(lexer->active_word, *i);
            start_word(lexer->active_word, WORD_NORMAL, *i);
            switch (input)
            {
                case '"':
                    lexer->active_word->type = WORD_DOUBLE_QUOTE;
                    break;
                case '\'':
                    lexer->active_word->type = WORD_SINGLE_QUOTE;
                    break;
                default:
                    lexer->active_word->type = WORD_NORMAL;
                    TokenAppend(lexer_buffer, input, *i);
                    break;
            }
            break;
        case DE_WORD_END_TOKEN_START:
            end_word(lexer->active_word, *i);
            lexer->active_token = create_token();
            lexer->active_word = NULL;
            break;
        case DE_TOKEN_END_OP_START:
            if(lexer->active_word != NULL)
                end_word(lexer->active_word, *i);
            TokenEnd(lexer_buffer, *i);
            TokenStartOperator(input, lexer->active_token);
            break;
        case DE_OP_START:
            TokenStartOperator(input, lexer->active_token);
            break;
        case DE_OP_END:
            TokenEnd(lexer_buffer, *i);
            break;
        case DE_OP_CHANGEHANDLE:
            t_operator_type new_op_type = CheckTokenMatch(input, lexer->active_token->operator_type);
            if (new_op_type != -1)
                lexer->active_token->operator_type = new_op_type;
            else
            {
                TokenEnd(lexer_buffer, *i);
                lexer->active_token = create_token();
                switch (input)
                {
                    case '|':
                        lexer->active_token->operator_type = OP_PIPE;
                        break;
                    case '&':
                        lexer->active_token->operator_type = OP_BACKGROUND;
                        break;
                    case '<':
                        lexer->active_token->operator_type = OP_REDIR_IN;
                        break;
                    case '>':
                        lexer->active_token->operator_type = OP_REDIR_OUT;
                        break;
                }
            }
            break;
        case DE_OP_END_WORD_START:
            TokenEnd(lexer_buffer, *i);
            lexer->active_token = create_token();
            start_word(lexer->active_word, WORD_NORMAL, *i);
            switch (input)
            {
                case '"':
                    lexer->active_word->type = WORD_DOUBLE_QUOTE;
                    break;
                case '\'':  
                    lexer->active_word->type = WORD_SINGLE_QUOTE;
                    break;
                default:
                    lexer->active_word->type = WORD_NORMAL;
                    TokenAppend(lexer_buffer, input, *i);
                    break;
            }
        default:
            // error handling
            break;
    }
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
        input_decision decision = lexer_read(buffer, lexer_buffer, &i, lexer->active_token, lexer->active_word);

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
    if (lexer->active_token != NULL)
        TokenEnd(lexer_buffer, &tokenCount, i);

    free_tokens(first);
    return 1;
}