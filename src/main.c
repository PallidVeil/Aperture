#include "common.h"
#include "editor.h"

int ReadInput(char* buffer, Editor *editor)
{
    InputType type;

    int handle;

    EnterRawMode(&editor->orig_termios);
    
    do
    {
        read(STDIN_FILENO, &editor->input, 1);
        type = FindInputType(editor->input);
        handle = handleInputTypes(buffer, editor, type);
    } while(handle != -1);
    DisableRawMode(&editor->orig_termios);
    return 0;
}

int main()
{
    
    Editor editor = {0};
    int error = 0;

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
        lexer(buffer ,lexer_buffer, &editor);
        printf("\nYou entered: %s\n", buffer);


        free(buffer);
        break;
    }
}