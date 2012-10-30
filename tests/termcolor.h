#ifndef TERMCOLOR_H
#define TERMCOLOR_H

namespace termcolor
{

const int RESET = 0;
const int BRIGHT = 1;
const int DIM = 2;
const int UNDERLINE = 3;
const int BLINK	= 4;
const int REVERSE = 7;
const int HIDDEN = 8;

const int BLACK = 0;
const int RED = 1;
const int GREEN = 2;
const int YELLOW = 3;
const int BLUE = 4;
const int MAGENTA = 5;
const int CYAN = 6;
const int WHITE = 7;

void set(int text, int bg = BLACK, int attrib = RESET);
inline void setBright(int text, int bg = BLACK) { set(text, bg, BRIGHT); }
inline void reset() { set(WHITE); }

}

#endif

