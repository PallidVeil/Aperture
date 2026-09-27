#include "common.h"
#include "editor.h"

bool isInsertOn = true;

void writeBufferToTerminal(char *buffer, Editor *editor)
{
    printf("\033[2K\r");
    printf("%s", buffer);
    printf("\033[%zuG", editor->cursor + 1);

    fflush(stdout);
}

void moveCursorLeft(Editor *editor)
{
    if (editor->cursor <= 0)
        return;
    editor->cursor--;
    printf("\033[D");
    fflush(stdout);
}

void moveCursorRight(Editor *editor)
{
    if (editor->cursor >= editor->length)
        return;
    editor->cursor++;
    printf("\033[C");
    fflush(stdout);
}

void moveCursorHome(Editor *editor)
{
    editor->cursor = 0;
    printf("\033[%zuD", editor->cursor);
    fflush(stdout);
}

void moveCursorEnd(Editor *editor)
{
    editor->cursor = editor->length;
    printf("\033[%zuC", editor->length - editor->cursor);
    fflush(stdout);
}

void EnterRawMode(struct termios *orig_termios)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, orig_termios);
    raw = *orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void DisableRawMode(struct termios *orig_termios)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

int executeEscapeSequence(InputType type, char *buffer, Editor *editor)
{
    switch (type)
    {
    case INPUT_UP:
        return 0;
        break;
    case INPUT_DOWN:
        return 0;
        break;
    case INPUT_LEFT:
        moveCursorLeft(editor);
        return 0;
        break;
    case INPUT_RIGHT:
        moveCursorRight(editor);
        return 0;
        break;
    case INPUT_HOME:
        moveCursorHome(editor);
        return 0;
        break;
    case INPUT_END:
        moveCursorEnd(editor);
        return 0;
        break;
    case INPUT_INSERT:
        isInsertOn = !isInsertOn;
        setInsertOrOverwriteCursor();
        return 0;
        break;
    case INPUT_DELETE:
        editor->delete = true;
        HandleInputMovement(buffer, editor);
        editor->delete = false;
        return 0;
        break;
    case INPUT_PAGE_UP:
        return 0;
        break;
    case INPUT_PAGE_DOWN:
        return 0;
        break;
    case INPUT_CONTROL_UP:
        return 0;
        break;
    case INPUT_CONTROL_DOWN:
        return 0;
        break;
    case INPUT_CONTROL_LEFT:
        return 0;
        break;
    case INPUT_CONTROL_RIGHT:
        return 0;
        break;
    case INPUT_UNKNOWN:
        return 0;
        break;
    default:
        return 0;
        break;
    }
}

InputType FindEscapeSequenceType(char *sequence)
{
    if (strcmp(sequence, "[A") == 0)
        return INPUT_UP;
    else if (strcmp(sequence, "[B") == 0)
        return INPUT_DOWN;
    else if (strcmp(sequence, "[C") == 0)
        return INPUT_RIGHT;
    else if (strcmp(sequence, "[D") == 0)
        return INPUT_LEFT;
    else if (strcmp(sequence, "[H") == 0)
        return INPUT_HOME;
    else if (strcmp(sequence, "[F") == 0)
        return INPUT_END;
    else if (strcmp(sequence, "[2~") == 0)
        return INPUT_INSERT;
    else if (strcmp(sequence, "[3~") == 0)
        return INPUT_DELETE;
    else if (strcmp(sequence, "[5~") == 0)
        return INPUT_PAGE_UP;
    else if (strcmp(sequence, "[6~") == 0)
        return INPUT_PAGE_DOWN;
    else if (strcmp(sequence, "[1;5A") == 0)
        return INPUT_CONTROL_UP;
    else if (strcmp(sequence, "[1;5B") == 0)
        return INPUT_CONTROL_DOWN;
    else if (strcmp(sequence, "[1;5C") == 0)
        return INPUT_CONTROL_RIGHT;
    else if (strcmp(sequence, "[1;5D") == 0)
        return INPUT_CONTROL_LEFT;
    else
        return INPUT_UNKNOWN;
}

void HandleEscSequence(char *buffer, Editor *editor)
{
    char sequenceBuffer[MAX_ESCAPE_SEQUENCE_LENGTH] = {0};
    int i = 0;
    int timeoutcheck;
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    while (1)
    {
        timeoutcheck = poll(&pfd, 1, POLLTIMEOUT);
        if (i >= MAX_ESCAPE_SEQUENCE_LENGTH - 1)
            break;
        if (timeoutcheck == 0)
            break;
        if (timeoutcheck == -1) // error add a check later
            break;
        read(STDIN_FILENO, &sequenceBuffer[i], 1);
        i++;
    }
    InputType type = FindEscapeSequenceType(sequenceBuffer);
    executeEscapeSequence(type, buffer, editor);
}

void HandleBackspace(char *buffer, Editor *editor)
{
    if (editor->cursor <= 0)
        return;
        
    for (size_t i = editor->cursor - 1; i < editor->length; i++)
    {
        buffer[i] = buffer[i + 1];
    }
    moveCursorLeft(editor);
    editor->length--;
    buffer[editor->length] = '\0';
    writeBufferToTerminal(buffer, editor);
}

void HandleDelete(char *buffer, Editor *editor)
{
    if (editor->cursor < editor->length)
    {
        for (size_t i = editor->cursor; i < editor->length; i++)
        {
            buffer[i] = buffer[i + 1];
        }
        editor->length--;
        buffer[editor->length] = '\0';
        writeBufferToTerminal(buffer, editor);
    }
}

void HandleInsertInput(char *buffer, Editor *editor)
{
    editor->length++;
    for (size_t i = editor->length; i > editor->cursor; i--)
    {
        buffer[i] = buffer[i - 1];
    }
    buffer[(editor->cursor)] = editor->input;
    moveCursorRight(editor);
    buffer[editor->length] = '\0';
    writeBufferToTerminal(buffer, editor);
}

void HandleOverwriteInput(char *buffer, Editor *editor)
{
    if (editor->cursor <= editor->length)
    editor->length++;
    buffer[(editor->cursor)] = editor->input;
    writeBufferToTerminal(buffer, editor);
    moveCursorRight(editor);
}

void HandleInputMovement(char *buffer, Editor *editor)
{
    if (editor->input == 127)
        HandleBackspace(buffer, editor);
    else if (editor->delete)
        HandleDelete(buffer, editor);
    else if (isInsertOn)
        HandleInsertInput(buffer, editor);
    else
        HandleOverwriteInput(buffer, editor);
    return;
}

InputType FindInputType(char input)
{
    switch (input)
    {
    case 9:
        return INPUT_TAB;

    case 10:
    case 13:
        return INPUT_ENTER;
    case 27:
        return INPUT_ESCAPE;
    case 127:
        return INPUT_BACKSPACE;
    default:
        return INPUT_NORMAL;
    }
}

void setInsertOrOverwriteCursor(void)
{
    if (isInsertOn)
        printf("\033[6 q");
    else
        printf("\033[2 q");
}

int handleInputTypes(char *buffer, Editor *editor, InputType type)
{
    switch (type)
    {
    case INPUT_NORMAL:
        HandleInputMovement(buffer, editor);
        break;
    case INPUT_ENTER:
        return -1;
        break;
    case INPUT_INSERT:
        isInsertOn = !isInsertOn;
        setInsertOrOverwriteCursor();
        return 0;
        break;
    case INPUT_BACKSPACE:
        HandleInputMovement(buffer, editor);
        break;
    case INPUT_TAB:
        return 0;
        break;
    case INPUT_ESCAPE:
        HandleEscSequence(buffer, editor);
        return 0;
        break;
    }
    return 0;
}