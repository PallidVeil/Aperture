#include "common.h"
#include "lexer.h"
#include "editor.h"


t_lexer *lexer = NULL;
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

void free_tokens(t_token *first)
{
    t_token *curr;
    t_token *next;

    curr = first;
    while (curr)
    {
        free_words(curr->word);

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
    if (lexer->active_token != NULL
        && *lexer->active_token != NULL
        && (*lexer->active_token)->type == TOKEN_NONE)
    {
        free(*lexer->active_token);
        *lexer->active_token = NULL;
    }
    lexer->active_token = NULL;
    lexer->active_word = NULL;
}

t_token *create_token()
{
    t_token *new = malloc(sizeof(t_token));
    if (!new)
        return NULL;
    new->type = TOKEN_NONE;
    new->word = NULL;
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

t_word **getLastWord()
{
    t_word **word;

    word = &token->active_token->word;
    while (*word != NULL)
        word = &(*word)->nextword;
    return word;
}

void end_word(char *t_buffer, size_t i)
{
    t_buffer[i] = '\0';
    (*lexer->active_word)->length =
        i - (*lexer->active_word)->start;

    lexer->active_word =
        &(*lexer->active_word)->nextword;
}

void start_word(t_word_type type, size_t start)
{
    t_word **word;

    word = getLastWord();
    *word = create_word(type, start);
    lexer->active_word = *word;
}

void start_token(void)
{
    if (lexer->active_token == NULL)
        return;

    if (*lexer->active_token != NULL)
        return;

    *lexer->active_token = create_token();
    lexer->active_word = &(*lexer->active_token)->word;
}

void advance_token(void)
{
    if (lexer->active_token == NULL
        || *lexer->active_token == NULL)
        return;

    lexer->active_token = &(*lexer->active_token)->next;
    lexer->active_word = NULL;

    start_token();
}

void change_token_type(t_token_type type)
{
    (*lexer->active_token)->type = type;
    return;
}

void errorHandle()
{

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

    if (lexer->active_token == NULL)
        return TOKEN_NONE;

    token = *lexer->active_token;
    if (token == NULL)
        return TOKEN_NONE;

    return token->type;
}

t_word_type check_word_type()
{
    t_word *word = lexer->active_word;
    if (word == NULL)
        return WORD_NULL;
    else 
        return word->type;
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
            decisions.dec[1] = DC_START_NW;
            decisions.dec[2] = DC_APPEND;
            break;
        case CHAR_WHITESPACE:
            decisions.dec_count = 1;
            decisions.dec[0] = DC_SKIP;
            break;
        case CHAR_SINGLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_START_TOKEN_WORD;
            decisions.dec[1] = DC_START_SQ;
            break;
        case CHAR_DOUBLE_QUOTE:
            decisions.dec_count = 2;
            decisions.dec[0] = DC_START_TOKEN_WORD;
            decisions.dec[1] = DC_START_DQ;
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
            if (lexer->active_token == NULL
                || *lexer->active_token == NULL)
            {
                errorHandle();
                break;
            }

            t_operator_type match = CheckTokenMatch(input, (*lexer->active_token)->operator_type);
            if (match == OP_NONE)
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



void handleLexerInputTypes(t_token_type token_type, t_word_type word_type, t_char_type char_type, char *input)
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
            case DC_START_NW:
                start_word(WORD_NORMAL,*lexerindex);
                break;
            case DC_END_NW:
                end_word(lexer_buffer,*lexerindex);
                (*lexerindex)++;
                break;

            case DC_START_SQ:
                start_word(WORD_SINGLE_QUOTE,*lexerindex);
                break;
            case DC_END_SQ:
                end_word(lexer_buffer,*lexerindex);
                (*lexerindex)++;
                break;

            case DC_START_DQ:
                start_word(WORD_DOUBLE_QUOTE,*lexerindex);
                break;
            case DC_END_DQ:
                end_word(lexer_buffer,*lexerindex);
                (*lexerindex)++;
                break;

            case DC_START_OPERATOR:
                if (lexer->active_token == NULL)
                    break;
                if (*lexer->active_token == NULL)
                    start_token();
                if (*lexer->active_token != NULL)
                    (*lexer->active_token)->operator_type = find_operator(input);
                break;
            case DC_OPERATOR_CHANGE:
            if (lexer->active_token == NULL)
                    break;
                if (*lexer->active_token == NULL)
                    errorHandle();
                if (*lexer->active_token != NULL)
                    (*lexer->active_token)->operator_type = change_operator(input);
                break;

            case DC_START_TOKEN_WORD:
                change_token_type(TOKEN_WORD);
                break;
            case DC_NEXT_TOKEN:
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
    t_word_type word_type;
    if (lexer->active_word != NULL)
        word_type = check_word_type();
    else
        word_type = WORD_NULL;
    
    if(charType != CHAR_EOF)
        charType = check_char_type(*input);

    handleLexerInputTypes(token_type, word_type, charType, input);
    return;
}


void print_tokens(t_token *token, const char *lexer_buffer)
{
    
    int token_index = 0;

    while (token != NULL)
    {
        printf("\n========== TOKEN %d ==========\n", token_index);
        printf("Token type: %d\n", token->type);

        t_word *word = token->word;
        int word_index = 0;

        while (word != NULL)
        {
            printf("  Word %d: ", word_index);

            printf("%.*s\n",
                (int)word->length,
                lexer_buffer + word->start);

            word = word->nextword;
            word_index++;
        }

        token = token->next;
        token_index++;
    }
}

void debug_tokens(t_token *token)
{
    int ti = 0;

    while (token != NULL)
    {
        printf("\n===== TOKEN %d =====\n", ti);
        printf("token address: %p\n", (void *)token);
        printf("token type: %d\n", token->type);
        printf("token->word: %p\n", (void *)token->word);
        printf("token->next: %p\n", (void *)token->next);

        t_word *word = token->word;
        int wi = 0;

        while (word != NULL)
        {
            printf("  --- WORD %d ---\n", wi);
            printf("  word address: %p\n", (void *)word);
            printf("  start: %zu\n", word->start);
            printf("  length: %zu\n", word->length);
            printf("  nextword: %p\n", (void *)word->nextword);

            word = word->nextword;
            wi++;
        }

        token = token->next;
        ti++;
    }

    printf("\n===== END OF TOKEN LIST =====\n");
}

int lex(char *buffer, char *lexer_buffer, size_t length)
{
    lexer = malloc(sizeof(t_lexer));
    if (!lexer)
        return 1;

    t_token *first_token = NULL;
    lexer->active_token = &first_token;
    *lexer->active_token = create_token();

printf("AFTER CREATE:\n");
printf("token: %p\n", (void *)lexer->active_token);
printf("type: %d\n", lexer->active_token->type);
printf("word: %p\n", (void *)lexer->active_token->word);
printf("next: %p\n", (void *)lexer->active_token->next);
    
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
    printf("BEFORE PRINT:\n");
    printf("first_token: %p\n", (void *)first_token);
    printf("type: %d\n", first_token->type);
    printf("word: %p\n", (void *)first_token->word);
    printf("next: %p\n", (void *)first_token->next);
    printf("lexerindex: %ld",lexerindex );
    print_tokens(first_token, lexer_buffer);
    debug_tokens(first_token);
    free_tokens(first_token);
    return 1;
}