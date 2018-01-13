#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <unistd.h>

#include "bline.h"

// main function
int main( int argc, char** argv )
{
    //if( argc == 1 )
    if( !( argc >> 1 ) )
    {
        welcome();
        exit( EXIT_SUCCESS );
    }

    // some flags to determine which option has been set
    short flag_all   = 0;
    short flag_even  = 0;
    short flag_odd   = 0;
    short flag_first = 0;
    short flag_last  = 0;
    short flag_indi  = 0;
    short flag_range = 0;
    short flag_match = 0;
    short flag_HEX   = 0;
    short flag_dump  = 0;

    // and value of some option

    // -I: any number greater-or-equal than 2
    // for 1 we have -f option that select the first line
    int indi_value = 0;

    //-R: between 1 - grater-than 1 like: 1-22 or 2-10
    int range_value_start = 0;
    int range_value_end   = 0;
    const char* range_value = "";

    // for strore the string we are looking for
    const char* match = "";

    // buffer for storing raw data input from the user
    // like: light:italic:red:green
    char raw_input[ BLINE_STACK_SIZE ];
    memset( raw_input, '\0', BLINE_STACK_SIZE );
    char* ri = raw_input;

    // options that are  available
    //
    // -a    --all
    // -e    --even
    // -o    --odd
    // -f    --first
    // -l    --last
    // -i    --individual
    // -I
    // -r    --range
    // -R
    // -m    --match
    // -H
    // -d    --dump
    // -E    --example
    // -M    --mode
    // -v    --version
    // -h    --help

    // short options
    const char* short_option = "a:e:o:f:l:i:I:r:R:m:dHEMvh";

    // long options
    static struct option long_option [] =
    {
        { "all",        1, NULL, 'a' },
        { "even",       1, NULL, 'e' },
        { "odd",        1, NULL, 'o' },
        { "first",      1, NULL, 'f' },
        { "last",       1, NULL, 'l' },
        { "individual", 1, NULL, 'i' },
        { "",           1, NULL, 'I' }, // for empty long option we should NOT use NULL
        { "range",      1, NULL, 'r' },
        { "",           1, NULL, 'R' }, // for empty long option we should NOT use NULL
        { "match",      1, NULL, 'm' },
        { "HEX",        0, NULL, 'H' },
        { "dump",       0, NULL, 'd' },
        { "example",    0, NULL, 'E' },
        { "mode",       0, NULL, 'M' },
        { "version",    0, NULL, 'v' },
        { "help",       0, NULL, 'h' },
        { NULL,         0, NULL,  0  }
    };

    // turn on/off errors for getopt
    // opterr = 0;

    // storing output of getopt_long()
    int result = 0;

    // paring with short option
    while( ( result = getopt_long( argc, argv, short_option, long_option, NULL ) ) != -1 )
    {
        // store the size of each option length
        size_t length = 0;

        // this switch and all memcpy(3) is for gathering all user input value
        // to a single buffer that later on we can parse. And names like:
        // ALL= or EVEN= are just names that show where an option start from.
        switch( result )
        {
        case 'a':
            memcpy( ri, "ALL=", 4 );
            ri += 4;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_all = 1;
            break;

        case 'e':
            memcpy( ri, "EVEN=", 5 );
            ri += 5;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_even = 1;
            break;

        case 'o':
            memcpy( ri, "ODD=", 4 );
            ri += 4;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_odd = 1;
            break;

        case 'f':
            memcpy( ri, "FIRST=", 6 );
            ri += 6;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_first = 1;
            break;

        case 'l':
            memcpy( ri, "LAST=", 5 );
            ri += 5;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_last = 1;
            break;

        case 'i':
            memcpy( ri, "INDIVIDUAL=", 11 );
            ri += 11;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_indi = 1;
            break;

        case 'I':
            indi_value = atoi( optarg );
            if( indi_value < 2 )
            {
                printf( "the value for -I should be >= than 2\n" );
                exit( EXIT_FAILURE );
            }
            break;

        case 'r':
            memcpy( ri, "RANGE=", 6 );
            ri += 6;

            length = strlen( optarg );
            memcpy( ri, optarg, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_range = 1;
            break;

        case 'R':
            range_value = optarg;
            if( ( range_value = strchr( range_value, '-' ) ) == NULL || *optarg == '-' )
            {
                printf( "pattern is start-end in number format, like: 3-7\n" );
                exit( EXIT_FAILURE );
            }
            else
            {
                const int offset = range_value - optarg;
                range_value -= offset;

                // convert to integer
                range_value_start = atoi( range_value );

                // more the pointer after '-'
                range_value += ( offset + 1 );

                // convert to integer
                range_value_end = atoi( range_value );

                if( range_value_start >= range_value_end )
                {
                    printf( "start: %d should be greater then end: %d\n", range_value_start, range_value_end );
                    exit( EXIT_FAILURE );
                }
            }
            break;

        case 'm':
            // word(s) that we are going to look for
            match = optarg;

            // if argc == optind, it means there is no arguments after optarg
            // if we do not do this check, then when this option is empty we
            // will get segmentation fault: SIGSEGV
            if( argc == optind )
            {
                printf( "require option for -m after %s [enter-color-name]\n", match );
                exit( EXIT_FAILURE );
            }

            const char* color = *( argv + optind ); // or argv[ optind ];

            // the color name should not start with a dash option '-'
            // for example: ./bline -m main.c -e red
            // is this case color becomes equal '-e'
            if( *color == '-' )
            {
                printf( "wrong color name for -m after match: [%s]\n", match );
                exit( EXIT_FAILURE );
            }

            length = strlen( color );

            memcpy( ri, "MATCH=", 6 );
            ri += 6;

            memcpy( ri, color, length );
            ri += length;
            *( ri++ ) = '\n';
            flag_match = 1;
            break;

        case 'H':
            flag_HEX = 1;
            break;

        case 'd':
            flag_dump = 1;
            break;

        case 'E':
            example();
            exit( EXIT_SUCCESS );
            break;

        case 'M':
            mode();
            exit( EXIT_SUCCESS );
            break;

        case 'v':
            printf( "bline version: %s\n", BLINE_VERSION );
            exit( EXIT_SUCCESS );
            break;

        case 'h':
            help();
            exit( EXIT_SUCCESS );
            break;

        case '?':
            exit( EXIT_FAILURE );
            break;

        default:
            printf( "unknown error\n" );
            exit( EXIT_FAILURE );
            break;
        }
    }

    // Ignore, because of conflicting with using two argument in -m flag
    /* if( optind < argc ) */
    /* { */
    /*     while( optind < argc ) */
    /*     { */
    /*         printf("there is no option for: (%s)\n", argv[ optind++ ] ); */
    /*     } */
    /*     exit( EXIT_FAILURE ); */
    /* } */

    // At this point we have our buffer that contains all arguments that
    // user has entered. And we should parse it to make/create an array of
    // pointer, that each of them points to a line

    // to see how this structure works see bline.c
    struct parseline pl;
    pl.find_pos = raw_input;
    pl.new_pos  = raw_input;
    pl.distance = 0;

    // array or pointer and a counter (= vc) for that
    char* vector[ 9 ];
    size_t vc = 0;

    // parse buffer based on newline as the delimiter.
    // We added this newline in getopt. see the above while loop *( ri++ ) = '\n';
    while( ( pl.find_pos = strchr( pl.find_pos, '\n' ) ) != NULL )
    {
        pl.distance = pl.find_pos - pl.new_pos;
        pl.new_pos[ pl.distance ] = '\0';

        // or just:
        //
        // pl.new_pos[ pl.find_pos - pl.new_pos ] = '\0';
        //
        // I prefer NOT to use this style because it is not readable
        // also I tested and it has the same speed/performance

        // points to this line
        vector[ vc ] = pl.new_pos;

        if( flag_dump == 1 )
        {
            #if defined( BLINE_WIN )
            printf( "%s\n\r", vector[ vc ] );
            #elif defined( BLINE_MACH )
            printf( "%s\r\n", vector[ vc ] );
            #else
            printf( "%s\n", vector[ vc ] );
            #endif
        }

        ++vc;

        #ifdef BLINE_WIN
        // skip \n\r
        pl.find_pos += 2;

        // update new pos
        pl.new_pos += ( pl.distance + 2 );
        #else
        // skip \n
        ++pl.find_pos;

        // update new pos
        pl.new_pos += ( pl.distance + 1 );
        #endif

        // or, just skip the colon and update new pos together
        //
        // pl.new_pos = ++pl.find_pos;
        //
        // I prefer NOT to use this style because it is not readable
        // also I tested and it has the same speed
    }

    // make the end of array NULL just for sure
    vector[ vc ] = NULL;

    // new buffer names: colors for storing the result of substitution
    // to related color code.
    // for example 'red:green' becomes: '\033[31;42m'
    char colors[ BLINE_STACK_SIZE ];
    memset( colors, '\0', BLINE_STACK_SIZE );

    // iterator for colors buffer
    // ci means: color iterator
    char* ci = colors;

    // an iterator to our array or pointer
    char** vptr = vector;
    while( *vptr )
    {
        // pointer to current line but we should ignore option name
        // something like: ALL=
        // +1 is for ignoring '=' equal sign
        char* line = ( strchr( *vptr, '=' ) + 1 );
        // okay, line points to after '=' sign, if we had ALL=red:green
        // than now it is: red:green

        // length of option name (for example ALL= or LAST=) equals
        const size_t length = line - *vptr;

        // copy option name (= like: ALL=, etc) to our buffer: colors
        memcpy( ci, *vptr, length );
        ci += length;

        // this is the length of ANSI color code after substitution
        // for example when: light:italic:red:green
        // becomes:          '1;3;31;32m'

        size_t length2 = 0;
        if( flag_HEX == 1 )
        {
            length2 = parse_hex_colors( line, ci );

            if( length2 > 36 )
            {
                printf( "%s has extra code\n", *vptr );
                exit( EXIT_FAILURE );
            }
        }
        else
        {
            length2 = parse_colors( line, ci );

            if( length2 > 16 )
            {
                printf( "[%.*s] flag has extra code\n", length, *vptr );
                exit( EXIT_FAILURE );
            }
        }
        // length2 should NOT be greater then 20 for non-hex or 40 for hex
        //
        // example for non-hex:
        // or:  \033[         = 2
        // and: (0;|1;|2;)    = 2
        // and: (3;,4:,5;,9;) = 8
        // and: foreground:   = 3
        // and: background:   = 3
        // total: 16
        //
        // you can see the length by:
        // printf( "length2: %d\n", length2 );

        ci += length2;

        // copy the literal 'END' to the and of each line
        // like: ALL=red:greenEND
        memcpy( ci, "END", 3 );
        ci += 3;

        // copy again the option name (= ALL or LAST, etc )
        // to create a end of line for ourselves, so later on
        // we can find a specific line
        // like: ALL=red:greenENDALL
        // -1 is for skipping '=' equal sign
        memcpy( ci, *vptr, length - 1);
        ci += length - 1;

        // and the end of each line should have newline
        *( ci++ ) = '\n';

        // move the iterator to the next line
        ++vptr;
    }


    /*
      for debugging purpose:

      printf( "colors:\n%s", colors );
      exit( 0 );

    */

    // At this point our colors bufferis ready to use
    // each variable is a pointer to its related ANSI color code
    // each length is the length of that color code
    //
    // for example in case of all, we will find ALL= and then pointer to it
    // and also ignoring the 'ALL=' itself
    // for example ALL=1;3;31;32mENDALL
    // then this pointer points to: '1;3;31;32m'
    // and length is the length of: '1;3;31;32m'
    // +4 is for ignoring the ALL= itself
    const char*  all = strstr( colors, "ALL=" ) + 4;

    // also when we found all (= a pointer) then we should find the
    // appropriate length for it and that is strstr(3) subtract from all (=pointer)
    const size_t all_length = strstr( colors, "ENDALL" ) - all;

    const char*  first = strstr( colors, "FIRST=" ) + 6;
    const size_t first_length = strstr( colors, "ENDFIRST" ) - first;

    const char*  last = strstr( colors, "LAST=" ) + 5;
    const size_t last_length = strstr( colors, "ENDLAST" ) - last;

    const char*  even = strstr( colors, "EVEN=" ) + 5;
    const size_t even_length = strstr( colors, "ENDEVEN" ) - even;

    const char*  odd = strstr( colors, "ODD=" ) + 4;
    const size_t odd_length = strstr( colors, "ENDODD" ) - odd;

    // individual
    const char*  indi = strstr( colors, "INDIVIDUAL=" ) + 11;
    const size_t indi_length = strstr( colors, "ENDINDIVIDUAL" ) - indi;

    const char*  range = strstr( colors, "RANGE=" ) + 6;
    const size_t range_length = strstr( colors, "ENDRANGE" ) - range;

    // cmatch, because we already have match variable
    const char*  cmatch = strstr( colors, "MATCH=" ) + 6;
    const size_t cmatch_length = strstr( colors, "ENDMATCH" ) - cmatch;


    // doing dump if the user has used -d option
    if( flag_dump )
    {
        if( flag_HEX )
        {
            printf( "in HEX format\n" );
        }
        else
        {
            printf( "in non-HEX format\n" );
        }

        // these crazy arithmetic with pointers are because I did not want to declare variables twice
        // all the above variables (= pointers) was calculated for printing on lines NOT for dump
        // thus here we have to bring them back
        // and so all + 4 becomes:  all - 4, all_length + 4 and so on.
        //
        // Also the 2 last values, all length - 5 and all pointers + 5 are for skipping the 5 characters
        // at the beginning of  each line: '033[m\033[' which equals 5
        //
        // example
        // 'ALL=\033[m\033[1;32mALLEND'
        // all - 4           :  \033[m\033[1;32mALLEND'
        // and all_length + 4: \033[m\033[1;32m
        //
        // for showing the related code
        // and all + 5       :  1;32mALLEND'
        // and all_length - 5:  1;32m
        if( flag_all )
        {
            printf( "--all        %.*s beautiful line output\033[m        => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( all_length + 4 ),   ( all - 4 ),   ( all_length - START_ESCAPE_SIZE ),    ( all + START_ESCAPE_SIZE )   );
        }
        if( flag_first )
        {
            printf( "--first      %.*s beautiful line output\033[m      => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( first_length + 6 ), ( first - 6 ), ( first_length - START_ESCAPE_SIZE ),  ( first + START_ESCAPE_SIZE ) );
        }
        if( flag_last )
        {
            printf( "--last       %.*s beautiful line output\033[m       => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( last_length + 5 ),  ( last - 5 ),  ( last_length - START_ESCAPE_SIZE ),   ( last + START_ESCAPE_SIZE )  );
        }
        if( flag_even )
        {
            printf( "--even       %.*s beautiful line output\033[m       => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( even_length + 5 ),  ( even - 5 ),  ( even_length - START_ESCAPE_SIZE ),   ( even + START_ESCAPE_SIZE )  );
        }
        if( flag_odd )
        {
            printf( "--odd        %.*s beautiful line output\033[m        => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( odd_length + 4 ),   ( odd - 4 ),   ( odd_length - START_ESCAPE_SIZE ),    ( odd + START_ESCAPE_SIZE )   );
        }
        if( flag_indi )
        {
            printf( "--individual %.*s beautiful line output\033[m => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( indi_length + 11 ), ( indi - 11 ), ( indi_length - START_ESCAPE_SIZE ),   ( indi + START_ESCAPE_SIZE )  );
        }
        if( flag_range )
        {
            printf( "--range      %.*s beautiful line output\033[m      => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( range_length + 6 ), ( range - 6 ), ( range_length - START_ESCAPE_SIZE ),  ( range + START_ESCAPE_SIZE ) );
        }
        if( flag_match )
        {
            printf( "--match      %.*s beautiful line output\033[m      => echo -e '\\033[%.*s#####\\033[m'\n",
                    ( cmatch_length + 6 ), ( cmatch - 6 ), ( cmatch_length - START_ESCAPE_SIZE ),  ( cmatch + START_ESCAPE_SIZE ) );
        }

        exit( EXIT_SUCCESS );
    }


    // allocate form heap
    char* calloc_ptr  = calloc( BLINE_HEAP_SIZE, sizeof( char ) );
    char* cptr        = calloc_ptr;
    size_t calloc_counter = 1;

    // if buffer filled up then extend it
    char* realloc_ptr = NULL;

    // length, nread, and line are required to use with getline(3)
    size_t      length;
    ssize_t      nread;

    // this will be for allocation buffer and should be freed
    char*   line = NULL;

    // storing last nread value
    ssize_t      nread_last;

    // total of nread, if it was about to BLINE_HEAP_SIZE
    // then it time to extend the buffer (= calloc_ptr)
    size_t total_nread = 0;

    // at most ANSI code length (here) for non-hex is 16
    // and hex value is 36
    // because apart from "line" we are also going to copy these
    // code to our buffer so we should count them otherwise we will
    // get heap-buffer-overflow
    const size_t code_length = ( flag_HEX == 0 ? 16 + BLINE_EOL_SIZE : 36 + BLINE_EOL_SIZE );

    // count how many line we have
    size_t  line_counter = 0;

    // main while loop for parsing each line
    //
    // on failure getline(3) returns -1
    while( ( nread = getline( &line, &length, stdin ) ) != -1 )
    {
        // storing nread because for the last line we will need it
        nread_last = nread;

        // we will need a log repetition of nread - 1, so we declare a variable for it
        const size_t nread_minus_one = nread - 1;

        // total of read byte by getline should NOT be treated just: += nread
        // because NOT only we copy line to our buffer but we also copy ANSI
        // code per line.
        //
        // code_length is either: 25 or 45
        // if we do not use +code_length then we will face Heap-Buffer-Overflow
        total_nread += ( nread + code_length );
        ++line_counter;

        // when we are about to reach the end of our currently buffer size
        // then it is time to extend it and add one more page-szie
        // ignoring the current line length is also vital otherwise will face Heap-Buffer-Overflow
        // It means we the buffer is near to fill up completely then we should go for extending it
        if( total_nread >= BLINE_HEAP_SIZE - nread )
        {
            ++calloc_counter;
            total_nread = 0;

            // the last position of cptr - calloc_ptr == length of current buffer
            // when we extend the buffer it may the whole buffer copy to somewhere else
            // in the memory thus we should have this offset so that can update "cptr"
            // after calling realloc(3)
            const size_t offset = cptr - calloc_ptr;

            if( ( realloc_ptr = realloc( calloc_ptr, BLINE_HEAP_SIZE * calloc_counter ) ) == NULL )
            {
                perror( "realloc(3)" );
                free( line );
                free( calloc_ptr );
                exit( EXIT_FAILURE );
            }

            calloc_ptr = realloc_ptr;
            cptr = calloc_ptr;
            cptr += offset;
        }

        // I did not use if-else, because I wanted the program be flexible
        // Thus the user can combine any option with others like:
        // --all red --even green --range yellow -R 2-7
        // And in the case of all lines will be red, even line will be green, and
        // line 2 - 7 will be yellow
        //
        // Three important things here are:
        // 1. using continue
        // 2. and putting end of color code after each line, but before newline: '\033[m\n'
        //    if we do not use (.2) then for setting background color, the entire page
        //    will be colorized.
        // 3. we should not copy the entire line, and we should ignore '\n', thus we have
        //    to: memcpy( cptr, line, nread - 1 )
        //    nread -n means not copy '\n'
        //
        // If you are curious why I am using a buffer instead of directly print these lines to
        // the standard output, then you should try it yourself how to colorize the "last" line
        // without storing all the lines in an intermediate buffer

        if( flag_first )
        {
            // if( line_counter == 1 )
            if( !( line_counter >> 1 ) )
            {
                memcpy( cptr, first, first_length );
                cptr += first_length;

                memcpy( cptr, line, nread_minus_one );
                cptr += nread_minus_one;

                memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
                cptr += BLINE_EOL_SIZE;

                // shift to right so 1 becomes 0
                flag_first >>= 1;

                continue;
            }
        }

        if( flag_match )
        {
            if( strstr( line, match ) != NULL )
            {
                memcpy( cptr, cmatch, cmatch_length );
                cptr += cmatch_length;

                memcpy( cptr, line, nread_minus_one );
                cptr += nread_minus_one;

                memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
                cptr += BLINE_EOL_SIZE;

                continue;
            }
        }


        if( flag_indi )
        {
            // if( line_counter == indi_value )
            if( !(line_counter ^ indi_value ) )
            {
                memcpy( cptr, indi, indi_length );
                cptr += indi_length;

                memcpy( cptr, line, nread_minus_one );
                cptr += nread_minus_one;

                memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
                cptr += BLINE_EOL_SIZE;

                // shift to right so 1 becomes 0
                flag_indi >>= 1;

                continue;
            }
        }

        if( flag_range )
        {
            // if( ( line_counter >= range_value_start ) && ( line_counter <= range_value_end ) )
            if( line_counter >= range_value_start )
            {
                if( line_counter <= range_value_end )
                {
                    memcpy( cptr, range, range_length );
                    cptr += range_length;

                    memcpy( cptr, line, nread_minus_one );
                    cptr += nread_minus_one;

                    memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
                    cptr += BLINE_EOL_SIZE;

                    continue;
                }
                else
                {
                    // shift to right so 1 becomes 0
                    flag_range >>= 1;
                }
            }
        }

        if( flag_even )
        {
            // if( line_counter % 2 == 0 )
            // an odd number, its LSB is always 1,  7 => 011[1]
            // an even number, its LSB is always 0, 8 => 100[0]
            // ( ???[1] & [0] ) ^ 1 => 1
            if( ( line_counter & 0x1 ) ^ 0x1  )
            {
                memcpy( cptr, even, even_length );
                cptr += even_length;

                memcpy( cptr, line, nread_minus_one );
                cptr += nread_minus_one;

                memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
                cptr += BLINE_EOL_SIZE;

                continue;
            }
        }

        if( flag_odd )
        {
            //if( line_counter % 2 != 0 )
            // an odd number, its LSB is always 1,  7 => 011[1]
            // an even number, its LSB is always 0, 8 => 100[0]
            // ???[1] & [1] => 1
            if( line_counter & 0x1 )
            {
                memcpy( cptr, odd, odd_length );
                cptr += odd_length;

                memcpy( cptr, line, nread_minus_one );
                cptr += nread_minus_one;

                memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
                cptr += BLINE_EOL_SIZE;

                continue;
            }
        }

        if( flag_all )
        {
            memcpy( cptr, all, all_length );
            cptr += all_length;

            memcpy( cptr, line, nread_minus_one );
            cptr += nread_minus_one;

            memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
            cptr += BLINE_EOL_SIZE;

            continue;
        }

        // if there is no flag(s) then we just copy lines without any colorization
        memcpy( cptr, line, nread );
        cptr += nread ;

    } // end of while loop over standard input

    // in the case of empty standard input
    // if( nread == -1 && line_counter == 0 )
    if( !( nread ^ -1 ) && !( line_counter ^ 0 ) )
    {
        printf( "stdin is empty: there is no line(s) to parse\n" );

        // fire line and calloc_ptr, they were allocated from heap and should be free(3)ed
        free( line );
        free( calloc_ptr );

        exit( EXIT_FAILURE );
    }

    // At this point we are out of while() and we can print our output buffer
    // but we should check for: flag_last that is for last line: -l
    //
    // In this case we already stored last line in our buffer: output
    // Thus if the -l option is enabled, then we have to move our iterator: cptr
    // -- that pointed to: output -- backward to just before the last line
    // and modify that color of that line

    if( flag_last )
    {
        // move the iterator back to the begging of the line (=last line)
        // if there is not color code in this line
        cptr -= nread_last;

        // but what if the user has entered -a option so all lines in color
        // but then -l flag is enabled as well
        // in this case we have to move back more than nread_last
        // -4 for end of color code
        // -5 for beginning of color code
        if( strstr( cptr, BLINE_EOL ) != NULL )
        {
            // ignoring \033[m\n (= 4) at the end and \033[ (= 2 ) at the beginning
            cptr -= 6;
        }

        // okay put the --last option color code in buffer: output
        memcpy( cptr, last, last_length );
        cptr += last_length;

        // we still have access to the last line, so copy it as well.
        memcpy( cptr, line, nread_last - 1 );
        cptr += ( nread_last - 1 );

        // and end of the last line
        memcpy( cptr, BLINE_EOL, BLINE_EOL_SIZE );
        cptr += BLINE_EOL_SIZE;
    }

    // make the buffer as a single NUL-terminated string
    *cptr = '\0';

    // and eventually print the buffer
    // printf( "%s", calloc_ptr );
    //
    // since we have only a single write to standard output thus we
    // can call write(2) system-call once
    write( STDOUT_FILENO, calloc_ptr, cptr - calloc_ptr );

    // fire line, it was allocated from heap and should be free(3)ed
    free( line );

    // free our buffer in heap
    free( calloc_ptr );

    // end of main. main return value
    return EXIT_SUCCESS;
}
