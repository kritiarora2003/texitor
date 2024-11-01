/**************** includes ***************/

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>



/**************** data ****************/

// to save the original termios structure
struct termios orig_termios;


/**************** termimnal ****************/

void die(const char *s) {
    perror(s);
    exit(1);
}


// after entering the raw mode, it remains like that on the terminal
// so we want ot exit it and go back to canonical mode

void disableRawMode() {
    // after making changes in the enable raw mode 
    // set the attributes back to original
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios)== -1) die("tcsetattr");
}


// void enableRawMode() {
//     // make a new struct for each instance of the function calling
//     struct termios raw;

//     // send the parameters of standard input to the struct
//     tcgetattr(STDIN_FILENO, &raw);

//     // unset the echo bit in the local flag attribute of struct
//     // which is the fourth from the last 
       // the echo flag is what prints whatever you are writing on the screen 
//     raw.c_lflag &= ~(ECHO);

//     // send the changes done in raw to standard input
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
// }

void enableRawMode() {

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die ("tcgetattr");

    atexit(disableRawMode);

    // make a copy of original termios struct and change it for raw mode
    struct termios raw = orig_termios;

    // turn off ctrl+s and ctrl+q
    // turn off carriage return and new file (ctrl+m)
    // carriage return is returning the cursor to the start 


    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // ouput post processing turn off 
    // \n translates to \r\n
    // turning it off will just send \n
    raw.c_oflag &= ~(OPOST);


    raw.c_cflag |= (CS8);

    // the ICANON flag disables the cannonical mode
    // canonical mode reads input line by line
    // when you press enter, the input line is sent in a batch to the shell
    // in raw mode, the input is sent byte-by-byte

    // turn off sending ctrl+c and ctrl+z signals
    // using the ISIG flag 

    // IEXTEN for disabling ctrl+v
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // putting a timer on read
    // vmin: min bytes needed for read to return
    raw.c_cc[VMIN] = 0;

    // vtime: time to wait for read
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die ("tcsetattr");
}



/**************** init ****************/

int main() {
    enableRawMode();

    char c;
    while (1){
        char c = '\0';

        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die ("read");

        // check if a character is a control character
        // they are ASCII 0-32(+127)
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } 
        else {
            printf("%d ('%c')\r\n", c, c);
        }

        if (c=='q') break;
    }
    return 0;
}
