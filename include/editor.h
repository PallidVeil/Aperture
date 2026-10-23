#ifndef EDITOR_H
#define EDITOR_H

typedef enum {
    INPUT_NORMAL,
    INPUT_ENTER,
    INPUT_BACKSPACE,
    INPUT_TAB,
    INPUT_ESCAPE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_HOME,
    INPUT_END,
    INPUT_INSERT,
    INPUT_DELETE,
    INPUT_PAGE_UP,
    INPUT_PAGE_DOWN,
    INPUT_CONTROL_UP,
    INPUT_CONTROL_DOWN,
    INPUT_CONTROL_LEFT,
    INPUT_CONTROL_RIGHT,
    INPUT_UNKNOWN
} InputType;

typedef struct {
    size_t length;
    size_t cursor;
    char input;
    bool delete;
    struct termios orig_termios;
} Editor;

#define POLLTIMEOUT 50 // milliseconds

void moveCursorLeft(Editor *editor);
void moveCursorRight(Editor *editor);
void moveCursorHome(Editor *editor);
void moveCursorEnd(Editor *editor);

InputType FindInputType(char input);
InputType parseEscapeSequence(const char *sequence);
int executeEscapeSequence(InputType type, char *buffer, Editor *editor);
int handleEditorInputTypes(char *buffer, Editor *editor, InputType type);

void setInsertOrOverwriteCursor(void);
void HandleInputMovement(char *buffer, Editor *editor);
int executeEscapeSequence(InputType type,char* buffer, Editor *editor);
void HandleEscSequence(char* buffer, Editor *editor);
void EnterRawMode(struct termios *orig_termios);
void DisableRawMode(struct termios *orig_termios);


extern bool isInsertOn;

#endif