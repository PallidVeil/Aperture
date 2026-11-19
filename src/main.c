#include "common.h"
#include "editor.h"
#include "lexer.h"
#include "parser.h"


int ReadInput(char* buffer, Editor *editor)
{
    InputType type;

    int handle;

    EnterRawMode(&editor->orig_termios);
    
    do
    {
        read(STDIN_FILENO, &editor->input, 1);
        type = FindInputType(editor->input);
        handle = handleEditorInputTypes(buffer, editor, type);
    } while(handle != -1);
    DisableRawMode(&editor->orig_termios);
    return 0;
}

int main()
{
    
    Editor editor = {0};
    int error = 0;
    t_parser parser;
    while(1)
    {
        char *buffer = malloc(BUFFER_SIZE);
        editor.length = 0;
        editor.cursor = 0;
        char* lexer_buffer = malloc(BUFFER_SIZE);
        error = ReadInput(buffer, &editor);
        if (error == -1)
        {
            free(buffer);
            break;
        }
        t_token *first_token = NULL;
        lex(buffer ,lexer_buffer, editor.length, &first_token);
        printf("\nYou entered: %s\n", buffer);
        parser_init(&parser, first_token);
        parser_run(&parser, lexer_buffer);
        parser_free(&parser);

        free_tokens(first_token);
        free(buffer);
        free(lexer_buffer);
        break;
    }
}