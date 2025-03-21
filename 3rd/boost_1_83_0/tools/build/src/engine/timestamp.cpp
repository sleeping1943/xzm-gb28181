/*
 * Copyright 1993-2002 Christopher Seiwald and Perforce Software, Inc.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/* This file is ALSO:
 * Copyright 2001-2004 David Abrahams.
 * Copyright 2020 Nikita Kniazev.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE.txt or
 * https://www.bfgroup.xyz/b2/LICENSE.txt)
 */

/*
 * timestamp.c - get the timestamp of a file or archive member
 *
 * External routines:
 *  timestamp_from_path() - return timestamp for a path, if present
 *  timestamp_done()      - free timestamp tables
 *
 * Internal routines:
 *  free_timestamps() - worker function for freeing timestamp table contents
 */

#include "jam.h"
#include "timestamp.h"

#include "filesys.h"
#include "hash.h"
#include "object.h"
#include "pathsys.h"
#include "jam_strings.h"
#include "output.h"


/*
 * BINDING - all known files
 */

typedef struct _binding
{
    OBJECT * name;
    short flags;

#define BIND_SCANNED  0x01  /* if directory or arch, has been scanned */

    short progress;

#define BIND_INIT     0  /* never seen */
#define BIND_NOENTRY  1  /* timestamp requested but file never found */
#define BIND_SPOTTED  2  /* file found but not timed yet */
#define BIND_MISSING  3  /* file found but can not get timestamp */
#define BIND_FOUND    4  /* file found and time stamped */

    /* update time - cleared if the there is nothing to bind */
    timestamp time;
} BINDING;

static struct hash * bindhash = 0;


#ifdef OS_NT
/*
 * timestamp_from_filetime() - Windows FILETIME --> timestamp conversion
 *
 * Lifted shamelessly from the CPython implementation.
 */

void timestamp_from_filetime( timestamp * const t, FILETIME const * const ft )
{
    /* Seconds between 1.1.1601 and 1.1.1970 */
    static __int64 const secs_between_epochs = 11644473600;

    /* We can not simply cast and dereference a FILETIME, since it might not be
     * aligned properly. __int64 type variables are expected to be aligned to an
     * 8 byte boundary while FILETIME structures may be aligned to any 4 byte
     * boundary. Using an incorrectly aligned __int64 variable may cause a
     * performance penalty on some platforms or even exceptions on others
     * (documented on MSDN).
     */
    __int64 in;
    memcpy( &in, ft, sizeof( in ) );

    /* FILETIME resolution: 100ns. */
    timestamp_init( t, (time_t)( ( in / 10000000 ) - secs_between_epochs ),
        (int)( in % 10000000 ) * 100 );
}
#endif  /* OS_NT */


void timestamp_clear( timestamp * const time )
{
    time->secs = time->nsecs = 0;
}


int timestamp_cmp( timestamp const * const lhs, timestamp const * const rhs )
{
    return int(
        lhs->secs == rhs->secs
        ? lhs->nsecs - rhs->nsecs
        : lhs->secs - rhs->secs );
}


void timestamp_copy( timestamp * const target, timestamp const * const source )
{
    target->secs = source->secs;
    target->nsecs = source->nsecs;
}


void timestamp_current( timestamp * const t )
{
#ifdef OS_NT
    /* GetSystemTimeAsFileTime()'s resolution seems to be about 15 ms on Windows
     * XP and under a millisecond on Windows 7.
     */
    FILETIME ft;
    GetSystemTimeAsFileTime( &ft );
    timestamp_from_filetime( t, &ft );
#elif defined(_POSIX_TIMERS) && defined(CLOCK_REALTIME) && \
    (!defined(__GLIBC__) || (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 17))
    /* Some older versions of XCode define _POSIX_TIMERS, but don't actually
     * have clock_gettime.  Check CLOCK_REALTIME as well.  Prior to glibc 2.17,
     * clock_gettime requires -lrt.  This is a non-critical feature, so
     * we just disable it to keep bootstrapping simple.
     */
    struct timespec ts;
    clock_gettime( CLOCK_REALTIME, &ts );
    timestamp_init( t, ts.tv_sec, ts.tv_nsec );
#else  /* OS_NT */
    timestamp_init( t, time( 0 ), 0 );
#endif  /* OS_NT */
}


int timestamp_empty( timestamp const * const time )
{
    return !time->secs && !time->nsecs;
}


/*
 * timestamp_from_path() - return timestamp for a path, if present
 */

void timestamp_from_path( timestamp * const time, OBJECT * const path )
{
    PROFILE_ENTER( timestamp );

    if ( file_time( path, time ) < 0 )
        timestamp_clear( time );

    PROFILE_EXIT( timestamp );
}


void timestamp_init( timestamp * const time, time_t const secs, int const nsecs
    )
{
    time->secs = secs;
    time->nsecs = nsecs;
}


void timestamp_max( timestamp * const max, timestamp const * const lhs,
    timestamp const * const rhs )
{
    if ( timestamp_cmp( lhs, rhs ) > 0 )
        timestamp_copy( max, lhs );
    else
        timestamp_copy( max, rhs );
}


static char const * timestamp_formatstr( timestamp const * const time,
    char const * const format )
{
    static char result1[ 500 ];
    static char result2[ 500 ];
    strftime( result1, sizeof( result1 ) / sizeof( *result1 ), format, gmtime(
        &time->secs ) );
    sprintf( result2, result1, time->nsecs );
    return result2;
}


char const * timestamp_str( timestamp const * const time )
{
    return timestamp_formatstr( time, "%Y-%m-%d %H:%M:%S.%%09d +0000" );
}


char const * timestamp_timestr( timestamp const * const time )
{
    return timestamp_formatstr( time, "%H:%M:%S.%%09d" );
}


/*
 * free_timestamps() - worker function for freeing timestamp table contents
 */

static void free_timestamps( void * xbinding, void * data )
{
    object_free( ( (BINDING *)xbinding )->name );
}


/*
 * timestamp_done() - free timestamp tables
 */

void timestamp_done()
{
    if ( bindhash )
    {
        hashenumerate( bindhash, free_timestamps, 0 );
        hashdone( bindhash );
    }
}

/*
 * timestamp_delta_seconds() - seconds from time a to b.
 */
double timestamp_delta_seconds( timestamp const * const a , timestamp const * const b )
{
    return difftime(b->secs, a->secs) + (b->nsecs - a->nsecs) * 1.0E-9;
}
