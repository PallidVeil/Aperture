#include "common.h"
#include "lexer.h"
#include "editor.h"


t_lexer *lexer;
i_decision decisions;

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

void TokenAppend(char *t_buffer, int input, size_t i)
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

void end_word(size_t i)
{
    lexer->active_word->length = i - lexer->active_word->start;
    lexer->active_word = lexer->active_word->nextword;
}

void start_word(t_word_type type, size_t start)
{
    if(lexer->active_word == NULL)
        lexer->active_word = create_word(type, start);
}

void start_token()
{
    if(lexer->active_token == NULL)
        lexer->active_token = create_token();
}

t_operator_type CheckTokenMatch(int input, t_operator_type LastToken)
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

bool isInputAnOperator(int input)
{
    return (input == '|' ||
            input == '<' ||
            input == '>' ||
            input == '&' ||
            input == ';' ||
            input == '(' ||
            input == ')');
}

t_char_type check_char_type(int input)
{
    if (input == EOF)
        return CHAR_EOF;

    if (isInputAnOperator(input))
        return CHAR_OPERATOR;

    switch (input)
    {
        case ' ':
        case '\t':
            return CHAR_WHITESPACE;
        case '"':
            return CHAR_DOUBLE_QUOTE;
        case '\'':
            return CHAR_SINGLE_QUOTE;
        default:
            return CHAR_NORMAL;
    }
}

void lexer_read(char *lexer_buffer, size_t i, t_token *token, t_word *word, size_t length)
{
    // what i have in my mind is that if there is no active word the function ill allocate a new one if the word ends 
    // we do activeword = activeword-> nextword and if the token is finished we will end the token and start a new one

    t_token_type token_type = check_token_type();
    t_word_type word_type;
    if (word != NULL)
        word_type = check_word_type();
    else
        word_type = WORD_NULL;
    if(i >= length)
    {
        handleInputTypes(token_type, word_type, CHAR_EOF, input);
        return;
    }
    int input = buffer[i];
    t_char_type char_type = check_char_type(input);
    handleInputTypes(token_type, word_type, char_type, input);
    return;
}

void handleNormalWord(t_char_type char_type)
{
    switch(char_type)
    {
    case CHAR_NORMAL:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_WHITESPACE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_END_NW;
        decisions.dec[1] = DC_NEXT_TOKEN;
        break;
    case CHAR_SINGLE_QUOTE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_END_NW;
        decisions.dec[1] = DC_START_SQ;
        break;
    case CHAR_DOUBLE_QUOTE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_END_NW;
        decisions.dec[1] = DC_START_DQ;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 3;
        decisions.dec[0] = DC_END_NW;
        decisions.dec[1] = DC_NEXT_TOKEN;
        decisions.dec[2] = DC_START_OPERATOR;
        break;
    case CHAR_EOF:
        decisions.dec_count = 3;
        decisions.dec[0] = DC_END_NW;
        decisions.dec[1] = DC_END_TOKEN;
        decisions.dec[2] = DC_END;
        break;
    }
}

void handleSingleQuote(t_char_type char_type)
{
    switch(char_type)
    {
    case CHAR_NORMAL:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_WHITESPACE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_SINGLE_QUOTE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_END_SQ;
        break;
    case CHAR_DOUBLE_QUOTE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_EOF:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_ERROR;
        break;
    }
}

void handleDoubleQuote(t_char_type char_type)
{
    switch(char_type)
    {
    case CHAR_NORMAL:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_WHITESPACE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_SINGLE_QUOTE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_DOUBLE_QUOTE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_END_DQ;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_EOF:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_ERROR;
        break;
    }
}

void handleWordNull(t_char_type char_type)
{
    switch(char_type)
    {
    case CHAR_NORMAL:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_START_NW;
        decisions.dec[1] = DC_APPEND;
        break;
    case CHAR_WHITESPACE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_NEXT_TOKEN;
        break;
    case CHAR_SINGLE_QUOTE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_START_SQ;
        break;
    case CHAR_DOUBLE_QUOTE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_START_DQ;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_NEXT_TOKEN;
        decisions.dec[1] = DC_START_OPERATOR;
        break;
    case CHAR_EOF:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_TOKEN_END;
        decisions.dec[1] = DC_END;
        break;
    }
}

void handleTokenNone(t_char_type char_type)
{
    switch(char_type)
    {
        case CHAR_NORMAL:
            decisions.dec_count = 3;
            decisions.dec[0] = DC_START_TOKEN;
            decisions.dec[1] = DC_START_NW;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_WHITESPACE:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_SKIP;
            break;
        case CHAR_SINGLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_START_TOKEN;
            decisions.dec[1] = DC_START_SQ;
            break;
        case CHAR_DOUBLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_START_TOKEN;
            decisions.dec[1] = DC_START_DQ;
            break;
        case CHAR_OPERATOR:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_START_TOKEN;
            decisions.dec[1] = DC_START_OPERATOR;
            break;
        case CHAR_EOF:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_END;
            break;
    }
    return;
}

void handleTokenWord(t_word_type word_type, t_char_type char_type)
{
    switch (word_type)
    {
        case WORD_NORMAL:
            handleNormalWord(char_type);
            return;
        case WORD_SINGLE_QUOTE:
            handleSingleQuote(char_type);
            return;
        case WORD_DOUBLE_QUOTE:
            handleDoubleQuote(char_type);
            return;
        case WORD_NULL:
            handleWordNull(char_type);
    }
    return;
}

void handleTokenOperator(t_char_type char_type, int input)
{
    switch(char_type)
    {
        case CHAR_NORMAL:
            decisions.dec_count = 3;
            decisions.dec[0] = DC_NEXT_TOKEN;
            decisions.dec[1] = DC_START_NW;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_WHITESPACE:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_NEXT_TOKEN;
            break;
        case CHAR_SINGLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_NEXT_TOKEN;
            decisions.dec[1] = DC_START_SQ;
            break;
        case CHAR_DOUBLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_NEXT_TOKEN;
            decisions.dec[1] = DC_START_DQ;
            break;
        case CHAR_OPERATOR:
        {
            t_operator_type match = CheckTokenMatch(input, lexer->active_token->operator_type);
            if(match != -1)
            {
                decisions.dec_count = 1;
                decisions.dec[0] = DC_OPERATOR_CHANGE;
            } else {
                decisions.dec_count = 2;
                decisions.dec[0] = DC_NEXT_TOKEN;
                decisions.dec[1] = DC_START_OPERATOR;
            }
        }
            break;
        case CHAR_EOF:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_END;
            break;
    }
    return;
}



void handleInputTypes(t_token_type token_type, t_word_type word_type, t_char_type char_type, int input)
{
    decisions.dec_count = 0;
    switch (token_type)
    {
        case TOKEN_NONE:
            handleTokenNone(char_type);
            return;
        case TOKEN_WORD:
            handleTokenWord(word_type, char_type);
            return;
        case TOKEN_OPERATOR:
            handleTokenOperator(char_type, input);
            return;
    }
    return;
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
        return WORD_NULL;
    else 
        return word->type;
}

t_operator_type change_operator(char input)
{
    switch (input)
    {
        case '|':
            return OP_OR;
        case '&':
            return OP_AND;
        case '<':
            return OP_REDIR_IN;
        case '>':
            return OP_REDIR_OUT;
        default:
            return OP_NONE;
    }
}

t_operator_type find_operator(char input)
{
    switch (input)
    {
        case '|':
            return OP_PIPE;
        case '<':
            return OP_REDIR_IN;
        case '>':
            return OP_REDIR_OUT;
        default:
            return OP_NONE;
    }
}

void exec_decision(char input, char *lexer_buffer, size_t i)
{
    for(int j = 0;j < decisions.dec_count; j++)
    {
        switch (decisions.dec[j])
        {
            case DC_START_NW:
                start_word(WORD_NORMAL,i);
                break;
            case DC_END_NW:
                end_word(i);
                break;

            case DC_START_SQ:
                start_word(WORD_SINGLE_QUOTE,i);
                break;
            case DC_END_SQ:
                end_word(i);
                break;

            case DC_START_DQ:
                start_word(WORD_DOUBLE_QUOTE,i);
                break;
            case DC_END_DQ:
                end_word(i);
                break;

            case DC_START_OPERATOR:
                start_token();
                lexer->active_token->operator_type = find_operator(input);
                break;
            case DC_OPERATOR_CHANGE:
                lexer->active_token->operator_type = change_operator(input);
                break;

            case DC_START_TOKEN:
                start_token();
                break;
            case DC_NEXT_TOKEN:
                lexer->active_token = lexer->active_token->next;
                break;
            case DC_APPEND:
                TokenAppend(lexer_buffer, input, i);
                break;
            case DC_SKIP:
                break;
            case DC_END:
                tokenEnd(lexer_buffer, i);
                break;
            case DC_ERROR:
                errorHandle();
                break;
        }
    }
    
}

int lex(char *buffer, char *lexer_buffer, Editor *editor)
{
    lexer->active_token = create_token();
    t_token *first_token;
    first_token = lexer->active_token;
    size_t i = 0;
    size_t tokenCount = 0;

    while (i < editor->length)
    {   
        lexer_read(buffer, lexer_buffer, i, lexer->active_token, lexer->active_word, editor.length);
        exec_decision(input ,lexer_buffer ,i);
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

    free_tokens(first);
    return 1;
}"