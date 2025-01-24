#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {

  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON );

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main() {
  printf("=======================\n");

  enableRawMode();


  char c;
  while(read(STDIN_FILENO, &c, 1) == 1) {
    
    printf("Character input: %c | %d\n", c, c);
    
    // if (iscntrl(c)) {
    //   printf("%d\n", c);
    // } else {
    //   printf("%d ('%c')\n", c, c);
    // }

    if (c == 'q') {
      exit(0);
    }
  }
  disableRawMode();
  return 0;
}