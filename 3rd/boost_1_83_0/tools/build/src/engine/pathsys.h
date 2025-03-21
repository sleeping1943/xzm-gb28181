/*
 * Copyright 1993-2002 Christopher Seiwald and Perforce Software, Inc.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
Copyright 2020 René Ferdinand Rivera Morell
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.txt or https://www.bfgroup.xyz/b2/LICENSE.txt)
*/


/*
 * pathsys.h - PATHNAME struct
 */

/*
 * PATHNAME - a name of a file, broken into <grist>dir/base/suffix(member)
 *
 * <grist> - salt to distinguish between targets that would otherwise have the
 * same name - it never appears in the bound name of a target.
 *
 * (member) - archive member name: the syntax is arbitrary, but must agree in
 * path_parse(), path_build().
 */

#ifndef PATHSYS_VP_20020211_H
#define PATHSYS_VP_20020211_H

#include "config.h"
#include "object.h"
#include "jam_strings.h"

#include <string>


typedef struct _pathpart
{
    char const * ptr;
    int32_t len;
} PATHPART;

typedef struct _pathname
{
    PATHPART part[ 6 ];

#define f_grist   part[ 0 ]
#define f_root    part[ 1 ]
#define f_dir     part[ 2 ]
#define f_base    part[ 3 ]
#define f_suffix  part[ 4 ]
#define f_member  part[ 5 ]
} PATHNAME;


void path_build( PATHNAME *, string * file );
void path_parse( char const * file, PATHNAME * );
void path_parent( PATHNAME * );
int path_translate_to_os( char const *, string * file );

/* Given a path, returns an object containing an equivalent path in canonical
 * format that can be used as a unique key for that path. Equivalent paths such
 * as a/b, A\B, and a\B on NT all yield the same key.
 */
OBJECT * path_as_key( OBJECT * path );

/* Called as an optimization when we know we have a path that is already in its
 * canonical/long/key form. Avoids the need for some subsequent path_as_key()
 * call to do a potentially expensive path conversion requiring access to the
 * actual underlying file system.
 */
void path_register_key( OBJECT * canonic_path );

/* Returns a static pointer to the system dependent path to the temporary
 * directory. NOTE: Does *not* include a trailing path separator.
 */
string const * path_tmpdir( void );

/* Returns a new temporary name. */
OBJECT * path_tmpnam( void );

/* Returns a new temporary path. */
OBJECT * path_tmpfile( void );

/* Give the first argument to 'main', return a full path to our executable.
 * Returns null in the unlikely case it cannot be determined. Caller is
 * responsible for freeing the string.
 *
 * Implemented in jam.c
 */
char * executable_path( char const * argv0 );

void path_done( void );

namespace b2
{
    namespace paths
    {
        inline bool is_rooted(const std::string &p)
        {
            #if NT
            return
                (p.size() >= 1 && (p[0] == '/' || p[0] == '\\')) ||
                (p.size() >= 3 && p[1] == ':' && (p[2] == '/' || p[2] == '\\'));
            #else
            return
                (p.size() >= 1 && (p[0] == '/' || p[0] == '\\'));
            #endif
        }
        inline bool is_relative(const std::string &p)
        {
            return
                (p.size() >= 3 && (
                    (p[0] == '.' && p[1] == '.' && (p[2] == '/' || p[2] == '\\')) ||
                    (p[0] == '.' && (p[1] == '/' || p[1] == '\\'))
                    ));
        }
        std::string normalize(const std::string &p);
    }
}

#endif
