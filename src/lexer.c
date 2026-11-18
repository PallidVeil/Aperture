#include "common.h"
#include "lexer.h"
#include "editor.h"


t_token **active_token = NULL;
i_decision decisions;

void errorHandle()
{
    
}

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
        next = curr->next;
        free(curr);
        curr = next;
    }
}

void TokenAppend(char *t_buffer, int input, size_t i)
{
    t_buffer[i] = input;
}

void TokenEnd(void)
{
    if (active_token != NULL
        && *active_token != NULL
        && (*active_token)->token_type == TOKEN_NONE)
    {
        free(*active_token);
        *active_token = NULL;
    }
    active_token = NULL;
}

t_token *create_token()
{
    t_token *new = malloc(sizeof(t_token));
    if (!new)
        return NULL;
    new->token_type = TOKEN_NONE;
    new->operator_type = OP_NONE;
    new->word_active = false;
    new->next = NULL;
    return new;
}

void end_word()
{
    (*active_token)->quote_state = WORD_NULL;
}

void start_word(t_word_type type)
{
    (*active_token)->quote_state = type;
}

void start_token(void)
{
    if (active_token == NULL)
        return;

    if (*active_token != NULL)
        return;

    *active_token = create_token();
    if(*active_token == NULL)
        errorHandle();
}

void advance_token(void)
{
    if (active_token == NULL
        || *active_token == NULL)
        return;

    active_token = &(*active_token)->next;

    start_token();
}

void change_token_type(t_token_type type)
{
    (*active_token)->token_type = type;
    if (type == TOKEN_WORD)
    {
        (*active_token)->word_active = true;
        (*active_token)->quote_state = WORD_NULL;
    }
    else if (type == TOKEN_OPERATOR)
        (*active_token)->operator_type = OP_NONE;
    else
        errorHandle();
    return;
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

    return OP_NONE;
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

t_char_type check_char_type(char input)
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

t_token_type check_token_type(void)
{
    t_token *token;

    if (active_token == NULL)
        return TOKEN_NONE;

    token = *active_token;
    if (token == NULL)
        return TOKEN_NONE;

    return token->token_type;
}

t_word_type check_quote_state()
{
    return (*active_token)->quote_state;
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
        decisions.dec[0] = DC_END_WORD;
        decisions.dec[1] = DC_NEXT_TOKEN;
        break;
    case CHAR_SINGLE_QUOTE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_END_WORD;
        decisions.dec[1] = DC_START_WORD;
        break;
    case CHAR_DOUBLE_QUOTE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_END_WORD;
        decisions.dec[1] = DC_START_WORD;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 3;
        decisions.dec[0] = DC_END_WORD;
        decisions.dec[1] = DC_NEXT_TOKEN;
        decisions.dec[2] = DC_START_OPERATOR;
        break;
    case CHAR_EOF:
        decisions.dec_count = 3;
        decisions.dec[0] = DC_END_WORD;
        decisions.dec[1] = DC_NEXT_TOKEN;
        decisions.dec[2] = DC_END;
        break;
    default:
        errorHandle();
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
        decisions.dec_count = 2;
        decisions.dec[0] = DC_APPEND;
        decisions.dec[1] = DC_END_WORD;
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
    default:
        errorHandle();
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
        decisions.dec_count = 2;
        decisions.dec[0] = DC_APPEND;
        decisions.dec[1] = DC_END_WORD;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_APPEND;
        break;
    case CHAR_EOF:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_ERROR;
        break;
    default:
        errorHandle();
        break;
    }
}

void handleWordNull(t_char_type char_type)
{
    switch(char_type)
    {
    case CHAR_NORMAL:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_START_WORD;
        decisions.dec[1] = DC_APPEND;
        break;
    case CHAR_WHITESPACE:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_NEXT_TOKEN;
        break;
    case CHAR_SINGLE_QUOTE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_APPEND;
        decisions.dec[1] = DC_START_WORD;
        break;
    case CHAR_DOUBLE_QUOTE:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_APPEND;
        decisions.dec[1] = DC_START_WORD;
        break;
    case CHAR_OPERATOR:
        decisions.dec_count = 2;
        decisions.dec[0] = DC_NEXT_TOKEN;
        decisions.dec[1] = DC_START_OPERATOR;
        break;
    case CHAR_EOF:
        decisions.dec_count = 1;
        decisions.dec[0] = DC_END;
        break;
    default:
        errorHandle();
        break;
    }
}

void handleTokenNone(t_char_type char_type)
{
    switch(char_type)
    {
        case CHAR_NORMAL:
            decisions.dec_count = 3;
            decisions.dec[0] = DC_START_TOKEN_WORD;
            decisions.dec[1] = DC_START_WORD;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_WHITESPACE:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_SKIP;
            break;
        case CHAR_SINGLE_QUOTE:
            decisions.dec_count = 3;
            decisions.dec[0] = DC_START_TOKEN_WORD;
            decisions.dec[1] = DC_START_WORD;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_DOUBLE_QUOTE:
            decisions.dec_count = 3;
            decisions.dec[0] = DC_START_TOKEN_WORD;
            decisions.dec[1] = DC_START_WORD;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_OPERATOR:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_START_OPERATOR;
            break;
        case CHAR_EOF:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_END;
            break;
        default:
            errorHandle();
            break;
    }
    return;
}

void handleTokenWord(t_word_type quote_state, t_char_type char_type)
{
    switch (quote_state)
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
            return;
        default:
            errorHandle();
            break;
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
            decisions.dec[1] = DC_START_WORD;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_WHITESPACE:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_NEXT_TOKEN;
            break;
        case CHAR_SINGLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_NEXT_TOKEN;
            decisions.dec[1] = DC_START_WORD;
            break;
        case CHAR_DOUBLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_NEXT_TOKEN;
            decisions.dec[1] = DC_START_WORD;
            break;
        case CHAR_OPERATOR:
        {
            if (active_token == NULL
                || *active_token == NULL)
            {
                errorHandle();
                break;
            }

            t_operator_type match = CheckTokenMatch(input, (*active_token)->operator_type);
            if (match != OP_NONE)
            {
                decisions.dec_count = 1;
                decisions.dec[0] = DC_OPERATOR_CHANGE;
            }
            else
            {
                decisions.dec_count = 2;
                decisions.dec[0] = DC_NEXT_TOKEN;
                decisions.dec[1] = DC_START_OPERATOR;
            }
            break;
        }
        case CHAR_EOF:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_END;
            break;
        default:
            errorHandle();
            break;
    }
    return;
}



void handleLexerInputTypes(t_token_type token_type, t_word_type quote_state, t_char_type char_type, char *input)
{
    decisions.dec_count = 0;
    switch (token_type)
    {
        case TOKEN_NONE:
            handleTokenNone(char_type);
            return;
        case TOKEN_WORD:
            handleTokenWord(quote_state, char_type);
            return;
        case TOKEN_OPERATOR:
            handleTokenOperator(char_type, *input);
            return;
    }
    return;
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

void exec_decision(char input, char *lexer_buffer, size_t *lexerindex)
{
    for(int j = 0;j < decisions.dec_count; j++)
    {
        switch (decisions.dec[j])
        {
            case DC_START_WORD:
            t_word_type quote_state;
            if (input == '\'')
                quote_state = WORD_SINGLE_QUOTE;
            if (input == '"')
                quote_state = WORD_DOUBLE_QUOTE;
            else
                quote_state = WORD_NORMAL;
            start_word(quote_state);
            break;
            case DC_END_WORD:
                end_word();
                break;

            case DC_START_OPERATOR:
                if (active_token == NULL)
                    break;
                if (*active_token == NULL)
                    start_token();
                if (*active_token != NULL)
                {
                    (*active_token)->operator_type = find_operator(input);
                    (*active_token)->token_type = TOKEN_OPERATOR;
                }
                break;
            case DC_OPERATOR_CHANGE:
            if (active_token == NULL)
                    break;
                if (*active_token == NULL)
                    errorHandle();
                if (*active_token != NULL)
                    (*active_token)->operator_type = change_operator(input);
                break;

            case DC_START_TOKEN_WORD:
                change_token_type(TOKEN_WORD);
                (*active_token)->start = *lexerindex;
                break;
            case DC_NEXT_TOKEN:
                if ((*active_token)->token_type == TOKEN_WORD)
                    (*active_token)->length = *lexerindex - (*active_token)->start;
                advance_token();
                break;
            case DC_APPEND:
                TokenAppend(lexer_buffer, input, *lexerindex);
                (*lexerindex)++;
                break;
            case DC_SKIP:
                break;
            case DC_END:
                TokenEnd();
                break;
            case DC_ERROR:
                errorHandle();
                break;
        }
    }
    
}

void lexer_read(char *input , t_char_type charType)
{
    // what i have in my mind is that if there is no active word the function ill allocate a new one if the word ends 
    // we do activeword = activeword-> nextword and if the token is finished we will end the token and start a new one
    t_token_type token_type = check_token_type();
    t_word_type quote_state;
    if ((*(active_token))->word_active)
        quote_state = check_quote_state();
    else
        quote_state = WORD_NULL;
    
    if(charType != CHAR_EOF)
        charType = check_char_type(*input);

    handleLexerInputTypes(token_type, quote_state, charType, input);
    return;
}

void debug_tokens(t_token *token)
{
    int ti = 0;

    while (token != NULL)
    {
        printf("\n===== TOKEN %d =====\n", ti);
        printf("token address: %p\n", (void *)token);
        printf("token type: %d\n", token->token_type);
        printf("token->next: %p\n", (void *)token->next);

        token = token->next;
        ti++;
    }

    printf("\n===== END OF TOKEN LIST =====\n");
}

int lex(char *buffer, char *lexer_buffer, size_t length)
{
    t_token *first_token = NULL;
    active_token = &first_token;
    *active_token = create_token();
    if(*active_token == NULL)
    {
        errorHandle();
        return 0;
    }

    size_t bufferindex = 0;
    size_t lexerindex = 0;
    t_char_type charType;
    char input;
    while (bufferindex <= length)
    {   
        if(bufferindex >= length)
            charType = CHAR_EOF;
        else
        {
           charType = CHAR_NULL;
           input = buffer[bufferindex];
        }
        lexer_read(&input, charType); // im sending pointer because i could be empty and i dont want to get an error
        exec_decision(input ,lexer_buffer ,&lexerindex);
        bufferindex++;
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
    debug_tokens(first_token);
    free_tokens(first_token);
    return 1;
}