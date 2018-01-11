#ifndef BLINE_H
#define BLINE_H

#define BLINE_VERSION    "1.0.0"
#define BLINE_STACK_SIZE 100 * 8

// '\033['
#define START_ESCAPE_SIZE 2

#if defined( __WINDOES__ ) ||  defined( __WIN32__ ) || defined( _WIN32 )
    #define BLINE_EOL      "\033[m\n\r"
    #define BLINE_EOL_SIZE 5
    #define BLINE_WIN
#elif defined( __APPLE__ ) || defined( __MACH__ )
    #define BLINE_EOL      "\033[m\r\n"
    #define BLINE_EOL_SIZE 5
    #define BLINE_MACH
#else
    #define BLINE_EOL      "\033[m\n"
    #define BLINE_EOL_SIZE 4
    #define BLINE_OTHER
#endif

// man 3 sysconf
// PAGESIZE - _SC_PAGESIZE
// Size of a page in bytes.  Must not be less than 1.  (Some systems use PAGE_SIZE instead.)
#if defined( PAGESIZE )
    #define BLINE_HEAP_SIZE sysconf( PAGESIZE )
#elif defined( _SC_PAGESIZE )
    #define BLINE_HEAP_SIZE sysconf( _SC_PAGESIZE )
#elif defined( _SC_PAGE_SIZE )
    #define BLINE_HEAP_SIZE sysconf( _SC_PAGE_SIZE )
#else
    #define BLINE_HEAP_SIZE 8192
#endif

// this structure serves us iterating over a buffer easily.
// By joggling these three variables, we can parse any buffer

struct parseline
{
    // a pointer to a desired point, like --->'\n'
    char*   find_pos;

    // a pointer to for updating the new position, like after '\n'<---
    char*   new_pos;

    // store difference between 2 the pointers value, like: how are you today?\n
    // from: -->h...?<-- or: find_pos - new_pos
    size_t  distance;
};

size_t parse_colors( char* line, char* iterator );
size_t parse_hex_colors( char* line, char* iterator );
void   hex2ansi24( char* hex, char* ptr );

void   mode();
void   help();
void   example();
void   welcome();

#endif
