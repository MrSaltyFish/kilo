/*
 * Antirez's Kilo - Text Editor
 * https://viewsourcecode.org/snaptoken/kilo/
 */

/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editorConfig {
  struct termios orig_termios;
  int screenRows;
  int screenCols;
};
struct editorConfig E;

/*** terminal ***/
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {

  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
    die("tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

/*
 * Reads input keys
 */
char editorReadKey() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1) != 1)) {
    if (nread == -1 && errno != EAGAIN) {
      die("read");
    }
  }

  return c;
}

/*
 * Get the cursor's position
 */
int getCursorPosition(int *rows, int *cols) {

  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    return -1;
  }

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) {
      break;
    }
    if (buf[i] == 'R') {
      break;
    }
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') {
    return -1;
  }
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
    return -1;
  }
  editorReadKey();

  return 0;
}

/*
 * Gets The Window Size of the terminal using ioctl
 */
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  // Use IOCTL to get the window size
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // Fallback: Move cursor to corner and extract co-ordinates.
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
      return -1;
    }
    return getCursorPosition(rows, cols);
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*
 * Processes keys read by editorReadKey()
 */
void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
  case CTRL_KEY('q'):
    printf("Ctrl+q pressed, exiting\r\n");

    exit(0);
    break;

  default: {
    printf("case default: ('%c'), %d\r\n", c, c);
  } break;
  }
}

/*** append buffer ***/
struct abuf {
  char *b;
  int len;
};
#define ABUF_INIT {NULL, 0}

/*** output ***/

/*
 * Responsible for drawing Vim-style Tildes at the side
 */
void editorDrawRows() {
  int y = 0;

  for (y = 0; y < E.screenRows; y++) {
    write(STDOUT_FILENO, "~", 1);

    if (y < E.screenRows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}

/*
 * Refreshes the entire screen and sets the cursor to top left, and prints
 * Vim-style tildes
 */
void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** init ***/
void initEditor() {
  if (getWindowSize(&E.screenRows, &E.screenCols) == -1)
    die("getWindowSize");
}

int main() {
  printf("=======================\n");

  enableRawMode();
  initEditor();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }
  disableRawMode();
  return 0;
}
