#ifndef ANSI_CODE_H
#define ANSI_CODE_H

#define ANSI_INDEX 17

// this is for non-hex code
// and hex code are created dynamically
// for that see function parse_hex_colors
const char* color[] =
{
    // 3 color-mode
    "normal",
    "light",
    "dark",

    // 6 text-mode
    "italic",
    "underline",
    "blink",
    "cross",
    "reverse",
    "hide",

    // 8 foreground and background
    // NOTE that sixth name (= purple) is usually said to be
    // magenta: reddish-purple in color. But I preferred to
    // use purple since it is easy to pronounce and write and remember
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "purple",
    "cyan",
    "white"
};

const char* ansi[] =
{

    // 3 color-mode
    "0;",
    "1;",
    "2;",

    // 6 text-mode
    "3;",
    "4;",
    "5;",
    "9;",
    "7;",
    "8;",

    // 8 foreground and background
    // NOTE that sixth name (= purple) is usually said to be
    // magenta: reddish-purple in color. But I preferred to
    // use purple since it is easy to pronounce and write and remember
    "30;",
    "31;",
    "32;",
    "33;",
    "34;",
    "35;",
    "36;",
    "37;",
};

#endif // ANSI_CODE_H
