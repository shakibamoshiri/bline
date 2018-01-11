// bline.c
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include "ansi_code.h"

// 13 bytes buffer for ff-ff-ff conversion
#define ANSI24_INDEX_SIZE 13
#define START_ESCAPE_SIZE 2

// for paring line
struct parseline
{
    // a pointer to a desired point, like --->'\n'
    char*   find_pos;

    // a pointer for updating the new position, like after '\n'<---
    char*   new_pos;

    // store difference between these two pointers value, like: how are you today?\n
    // from: -->h...?<-- or: find_pos - new_pos
    size_t  distance;
};


// parse non-hex color
size_t parse_colors( char* line, char* buffer )
{
    char* iterator = buffer;

    // escape sequence code
    memcpy( iterator, "\033[", START_ESCAPE_SIZE );
    iterator += START_ESCAPE_SIZE;

    struct parseline pl;
    pl.find_pos = line;
    pl.new_pos  = line;
    pl.distance = 0;

    // flags
    short optional        = 0;
    short foreground_flag = 0;
    short background_flag = 0;

    // iterate over command-line option(s) value, like: light:red:green
    while( ( pl.find_pos = strchr( pl.find_pos, ':' ) ) != NULL )
    {
        // early break, because we have met a foreground color
        // and no longer need to stay in the loop
        if( foreground_flag == 1 )
        {
            break;
        }

        pl.distance = pl.find_pos - pl.new_pos;
        pl.new_pos[ pl.distance ] = '\0';

        // if we have double colons like ::, it means foreground should be optional
        // so we will exit the loop and in another loop will choose the appropriate value
        if( pl.distance == 0 )
        {
            optional = 1;

            // slipping ':'
            ++pl.find_pos;

            // updating to a new position, +1 is for skipping ':'
            pl.new_pos += ( pl.distance + 1 );

            break;
        }

        // we have ANSI_INDEX value in ansi_code.h
        int index = 0;
        while( index < ANSI_INDEX )
        {
            if( strcmp( pl.new_pos, color[ index ] ) == 0 )
            {
                // length is either 2 or 3, see ansi_code.h
                const short length = ( index <= 8 ? 2 : 3 );
                memcpy( iterator, ansi[ index ], length );
                iterator += length;

                // for avoid add extra color, like: white:white:white and son on
                // only once we should have foreground index not twice
                if( index <= 16 && index > 8 )
                {
                    foreground_flag = 1;
                    break;
                }
            }
            ++index;
        }

        // slipping ':'
        ++pl.find_pos;

        // updating to a new position, +1 is for skipping ':'
        pl.new_pos += ( pl.distance + 1 );

        // or, just:
        // pl.new_pos = ++pl.find_pos;
        // I prefer NOT to use this style because it is not readable
        // also I tested and it has the same speed
    }

    // look for extra ':' after last iteration:
    if( ( pl.find_pos = strchr( pl.new_pos, ':' ) ) != NULL )
    {
        pl.distance = pl.find_pos - pl.new_pos;
        pl.new_pos[ pl.distance ] = '\0';

        // or just:
        // pl.new_pos[ pl.find_pos - pl.new_pos ] = '\0';
        // I prefer NOT to use this style because it is not readable
        // also I tested and it has the same speed
    }

    // try to substitute the last match
    // getting background color or foreground
    int index = 0;
    while( index < ANSI_INDEX )
    {
        if( strcmp( pl.new_pos, color[ index ] ) == 0 )
        {
            if( optional == 1 || ( foreground_flag == 1 && index > 8 ) )
            {
                // then this color is for background and since it is for example 32
                // we should add 10 to it so it becomes 42
                background_flag = 1;
                size_t value = atoi( ansi[ index ] );
                value += 10;

                // copy value and end it with 'm'
                snprintf( iterator, 4, "%dm", value );
            }

            if( foreground_flag == 0 && optional == 0 )
            {
                background_flag = 1;

                // length is either 2 or 3, see ansi_code.h
                const short length = ( index <= 8 ? 2 : 3 );
                memcpy( iterator, ansi[ index ], length );
                iterator += length;

                // an explicit adding of 'm'
                *( iterator - 1 ) = 'm';
            }
        }

        ++index;
    }

    // we should go to one of the two above if statement
    // and background_flag should be 1.
    // the confusing part of this flag is may user did not add background color
    // so if we still get error? NO
    // the second if does 2 things
    //     add 'm' to the end
    //     and also set foreground color if we have net set in the previous while loop
    // I should have used extra flags for this, but I only used one flag for both
    if( background_flag != 1 )
    {
        printf( "background color: error\n" );
        exit( EXIT_FAILURE );
    }

    return strlen( buffer );
}

// convert string to hex value
size_t str2hex( const char* hex )
{
    // for make the foreground optional
    // like: :ff0000
    if( *hex == ':' )
    {
        // eliminate ':'
        ++hex;
    }

    const char* begin = hex;
    const char* end   = hex + strlen( hex );

    if( end - begin != 6 )
    {
        printf( "error: %s NOT equals 6 hex-digits\n", hex );
        exit( EXIT_FAILURE );
    }

    size_t result = 0;
    short offset  = 0;

    // iteration should be from end to begin NOT begin to end
    //
    // say we have fafa. In reality of computer world it means:
    // f == 15 == 1111
    // a == 10 == 1010
    // f == 15 == 1111
    // a == 10 == 1010
    //
    // Thus we can split a string to a characters that can be represented by a single hex value
    // and if we shift the numeric value of this characters by 4 and sum them, at the end we
    // will have an integer value of that hex string
    //
    // fafa:
    // 1111.1010.1111.0000
    //
    // f   .a   .f   .a
    // 1111.____.____.____
    // ____.1010.____.____
    // ____.____.1111.____
    // ____.____.____.1010
    //
    // from end to beginning
    // ( a << 0 ) + ( f << 4 ) + ( a << 8 ) + ( f << 12 ) == integer value
    while( --end >= begin  )
    {
        switch( tolower( *end ) )
        {
            case '0': result += ( 0 << offset ); break;
            case '1': result += ( 1 << offset ); break;
            case '2': result += ( 2 << offset ); break;
            case '3': result += ( 3 << offset ); break;
            case '4': result += ( 4 << offset ); break;
            case '5': result += ( 5 << offset ); break;
            case '6': result += ( 6 << offset ); break;
            case '7': result += ( 7 << offset ); break;
            case '8': result += ( 8 << offset ); break;
            case '9': result += ( 9 << offset ); break;
            case 'a': result += ( 10 << offset ); break;
            case 'b': result += ( 11 << offset ); break;
            case 'c': result += ( 12 << offset ); break;
            case 'd': result += ( 13 << offset ); break;
            case 'e': result += ( 14 << offset ); break;
            case 'f': result += ( 15 << offset ); break;
            default:
                printf( "error. [%c] is not Hex value in %s\n", *end, hex );
                exit( EXIT_FAILURE );
                break;
        }
        offset += 4;
    }

    return result;
}

// parse 24 bits and convert it to ANSI style escape-code
//
// convert string of hex value to string of integer value
// for example FFFFFF ended up becomes: 255;255;255
void hex2ansi24( const char* hex, char* result )
{
    const size_t value = str2hex( hex );
    //                                                       red     .green   .blue
    //                                                       0-255   .0-255   .0-255
    // value >> 16 && 0xFF  --> extract the first 8 bits :   ????????.________.________
    // value >> 8 && 0xFF   --> extract the second 8 bits:   ________.????????.________
    // value >> 0 && 0xFF   --> extract the third 8 bits :   ________.________.????????
    const short  nread = sprintf( result, "%d;%d;%d;", ( value >> 16 ) & 0xFF, ( value >> 8 ) & 0xFF, value & 0xFF );
    result[ nread ] = '\0';
}

// parse and substitute code with equivalent integer values
size_t parse_hex_colors( char* line, char* buffer )
{
    char* iterator = buffer;

    memcpy( iterator, "\033[", START_ESCAPE_SIZE );
    iterator += START_ESCAPE_SIZE;

    struct parseline pl;
    pl.find_pos = line;
    pl.new_pos  = line;
    pl.distance = 0;

    // length is 5 for both
    const char* foreground = "38;2;";
    const char* background = "48;2;";

    char ansi24[ ANSI24_INDEX_SIZE ];
    size_t ansi24_length = 0;

    // if there is NO colon ':' in this line then we only have to deal with foreground
    if( ( pl.find_pos = strchr( pl.find_pos, ':' ) ) == NULL )
    {
        // just foreground, like: ff0000
        memcpy( iterator, foreground, 5 );
        iterator += 5;

        // convert ff0000 to 255;0;0
        memset( ansi24, '\0', ANSI24_INDEX_SIZE );
        hex2ansi24( pl.new_pos, ansi24 );

        // add 'm' to the end
        ansi24_length = strlen( ansi24 );
        ansi24[ ansi24_length - 1 ] = 'm';

        memcpy( iterator, ansi24, ansi24_length );
        iterator += ansi24_length;
    }
    // otherwise we have to deal with foreground:background OR optional foreground and background
    else
    {
        pl.distance = pl.find_pos - pl.new_pos;
        // if it is zero we have something like: :ff0000
        if( pl.distance == 0 )
        {
            // no foreground but required background
            memcpy( iterator, background, 5 );
            iterator += 5;

            memset( ansi24, '\0', ANSI24_INDEX_SIZE );
            hex2ansi24( pl.new_pos, ansi24 );

            // add 'm' to the end
            ansi24_length = strlen( ansi24 );
            ansi24[ ansi24_length - 1 ] = 'm';

            memcpy( iterator, ansi24, ansi24_length );
            iterator += ansi24_length;
        }
        else
        {
            // both foreground and background
            // here we have to part
            // and we should separate them

            // this is for foreground
            pl.new_pos[ pl.distance ] = '\0';

            memcpy( iterator, foreground, 5 );
            iterator += 5;

            memset( ansi24, '\0', ANSI24_INDEX_SIZE );
            hex2ansi24( pl.new_pos, ansi24 );

            ansi24_length = strlen( ansi24 );
            memcpy( iterator, ansi24, ansi24_length );
            iterator += ansi24_length;

            // and this is for background
            pl.new_pos += ( pl.distance + 1 );

            memcpy( iterator, background, 5 );
            iterator += 5;

            memset( ansi24, '\0', ANSI24_INDEX_SIZE );
            hex2ansi24( pl.new_pos, ansi24 );

            ansi24_length = strlen( ansi24 );

            // one single 'm' add to the end
            ansi24[ ansi24_length - 1 ] = 'm';

            memcpy( iterator, ansi24, ansi24_length );
            iterator += ansi24_length;
        }
    }

    return strlen( buffer );
}

void mode()
{
    printf(
        "modes:\n"
        "    color-mode:\n"
        "    [dark|normal|light]\n"
        "    only one can be used\n"
        "\n"
        "    text-mode:\n"
        "    [italic,underline,blink,reverse,hide,cross]\n"
        "    all four can be used together\n"
        "\n"
        "    foreground:\n"
        "    [black|red|green|yellow|blue|purple|cyan|white]\n"
        "    only one can be used\n"
        "\n"
        "    background:\n"
        "    [black|red|green|yellow|blue|purple|cyan|white]\n"
        "    only one can be used\n"
        "\n"
        "order of modes (delimiter is ':'):\n"
        "    color-mode:text-mode:foreground:background\n"
        "    example:\n"
        "           italic\n"
        "           normal:red\n"
        "           light:italic:cross:red:green\n"
        );
}

void example()
{
    printf(
        "examples:\n"
        "# lines, foreground red:\n"
        "ls | bline -a red\n"
        "\n"
        "# all lines, foreground red, background green:\n"
        "ls | bline -a red:green\n"
        "\n"
        "# all lines, foreground is default (NOTE.3), background is green:\n"
        "ls | bline -a :green\n"
        "\n"
        "# add color-mode, odd lines (foreground) will be in light,red\n"
        "ls | bline -o light:red\n"
        "\n"
        "# add text-mode, as well\n"
        "ls | bline -o light:italic:cross:red\n"
        "\n"
        "# combine options are okay:\n"
        "ls | bline -a light:green -i red -I 7 -r yellow -R 5-10\n"
        "\n"
        "# using hex value, foreground in red for all lines\n"
        "df | bline -H -a ff00ff\n"
        "\n"
        "# hex, foreground green, background red\n"
        "df | bline -H -a 00ff00:ff0000\n"
        "\n"
        "# hex, just background, omitting the foreground\n"
        "df | bline -H -a :ffff00\n"
        "\n"
        "# wrong pattern. DO NOT USE\n"
        "df | bline -H a red         # please read NOTE.4\n"
        "df | bline -H italic:ff00ff # please read NOTE.5\n"
        "df | bline -a ff00ff        # please read NOTE.6\n"
        );
}

void help()
{
    printf(
        "bline: beautiful line output\n"
        "\n"
        "    -a    --all           set all lines\n"
        "    -f    --first         set just first line\n"
        "    -l    --last          set just last line\n"
        "    -e    --even          set just even lines\n"
        "    -o    --odd           set just odd lines\n"
        "    -I                    set value for -i option\n"
        "    -i    --individual    set an individual line\n"
        "                          -I (= a number) is required for this\n"
        "    -R                    set values for -r option\n"
        "    -r    --range         set a range of lines\n"
        "                          -R (= a range) is required for this\n"
        "    -H    --HEX           deal with hex values, not string names\n"
        "    -d    --dump          dump code and its color\n"
        "\n"
        "    -E    --example       some examples\n"
        "    -M    --mode          available modes\n"
        "\n"
        "    -h    --help          print (this) help\n"
        "    -v    --version       print version\n"
        "\n"
        "NOTE.1    When you use -r then use -R and for -i use -I.\n"
        "    .2    Colon (:) is the delimiter between modes.\n"
        "    .3    Empty colon before a color name makes the foreground\n"
        "          optional.\n"
        "    .4    To deal with hex value just use -H plus the regular options.\n"
        "    .5    Hex style does not have color or text mode.\n"
        "    .6    Without -H option you can not use hex value\n"
        "\n"
        "bline Copyright (C) 2017 Shakiba\n"
        "source   github.com/k-five/bline\n"
        );
}

void welcome()
{
    printf(
        "█░░░█ █▀▀ █░░ █▀▀ █▀▀█ █▀▄▀█ █▀▀\n"
        "█▄█▄█ █▀▀ █░░ █░░ █░░█ █░▀░█ █▀▀\n"
        "░▀░▀░ ▀▀▀ ▀▀▀ ▀▀▀ ▀▀▀▀ ▀░░░▀ ▀▀▀\n"
        "to beautiful line output CLI app\n"
        "\n"
        "for help    type: bline -h or --help\n"
        "for example type: bline -E or --example\n"
        "for mode    type: bline -M or --mode\n"
        );
}
