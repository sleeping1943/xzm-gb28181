/*
 * Copyright 1993, 2000 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/* This file is ALSO:
 * Copyright 2001-2004 David Abrahams.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.txt or https://www.bfgroup.xyz/b2/LICENSE.txt)
 */

/*
 * parse.h - make and destroy parse trees as driven by the parser.
 */

#ifndef PARSE_DWA20011020_H
#define PARSE_DWA20011020_H

#include "config.h"
#include "frames.h"
#include "lists.h"
#include "modules.h"


#define PARSE_APPEND    0
#define PARSE_FOREACH   1
#define PARSE_IF        2
#define PARSE_EVAL      3
#define PARSE_INCLUDE   4
#define PARSE_LIST      5
#define PARSE_LOCAL     6
#define PARSE_MODULE    7
#define PARSE_CLASS     8
#define PARSE_NULL      9
#define PARSE_ON        10
#define PARSE_RULE      11
#define PARSE_RULES     12
#define PARSE_SET       13
#define PARSE_SETCOMP   14
#define PARSE_SETEXEC   15
#define PARSE_SETTINGS  16
#define PARSE_SWITCH    17
#define PARSE_WHILE     18
#define PARSE_RETURN    19
#define PARSE_BREAK     20
#define PARSE_CONTINUE  21


/*
 * Parse tree node.
 */

typedef struct _PARSE PARSE;

struct _PARSE {
    int      type;
    PARSE  * left;
    PARSE  * right;
    PARSE  * third;
    OBJECT * string;
    OBJECT * string1;
    int      num;
    OBJECT * rulename;
    OBJECT * file;
    int      line;
};

void parse_file( OBJECT *, FRAME * );
void parse_string( OBJECT * name, const char * * lines, FRAME * frame );
void parse_save( PARSE * );

PARSE * parse_make(
    int type, PARSE * & left, PARSE * & right, PARSE * & third,
    OBJECT * string, OBJECT * string1, int num );
inline PARSE * parse_make(
    int type, PARSE * & left, PARSE * && right, PARSE * & third,
    OBJECT * string, OBJECT * string1, int num )
{
    PARSE * r = right;
    return parse_make( type, left, r, third, string, string1, num );
}

void parse_free( PARSE * & );
LIST * parse_evaluate( PARSE *, FRAME * );
void parse_done();

#endif
