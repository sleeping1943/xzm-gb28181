// Copyright 2018-2022 Emil Dotchevski and Reverge Studios, Inc.

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This is the program presented in
// https://boostorg.github.io/leaf/#introduction-eh.

// It reads a text file in a buffer and prints it to std::cout, using LEAF to
// handle errors. This version uses exception handling. The version that does
// not use exception handling is in print_file_result.cpp.

#include <boost/leaf.hpp>
#include <iostream>
#include <stdio.h>

namespace leaf = boost::leaf;


// First, we need an enum to define our error codes:
enum error_code
{
    bad_command_line = 1,
    open_error,
    read_error,
    size_error,
    eof_error,
    output_error
};


// We will handle all failures in our main function, but first, here are the
// declarations of the functions it calls, each communicating failures by
// throwing exceptions

// Parse the command line, return the file name.
char const * parse_command_line( int argc, char const * argv[] );

// Open a file for reading.
std::shared_ptr<FILE> file_open( char const * file_name );

// Return the size of the file.
std::size_t file_size( FILE & f );

// Read size bytes from f into buf.
void file_read( FILE & f, void * buf, std::size_t size );


// The main function, which handles all errors.
int main( int argc, char const * argv[] )
{
    return leaf::try_catch(

        [&]
        {
            char const * file_name = parse_command_line(argc,argv);

            auto load = leaf::on_error( leaf::e_file_name{file_name} );

            std::shared_ptr<FILE> f = file_open(file_name);

            std::size_t s = file_size(*f);

            std::string buffer(1 + s, '\0');
            file_read(*f, &buffer[0], buffer.size()-1);

            std::cout << buffer;
            std::cout.flush();
            if( std::cout.fail() )
                leaf::throw_exception(output_error, leaf::e_errno{errno});

            return 0;
        },

        // Each of the lambdas below is an error handler. LEAF will consider
        // them, in order, and call the first one that matches the available
        // error objects.

        // This handler will be called if the error includes:
        // - an object of type error_code equal to open_error, and
        // - an object of type leaf::e_errno that has .value equal to ENOENT,
        //   and
        // - an object of type leaf::e_file_name.
        []( leaf::match<error_code, open_error>, leaf::match_value<leaf::e_errno, ENOENT>, leaf::e_file_name const & fn )
        {
            std::cerr << "File not found: " << fn.value << std::endl;
            return 1;
        },

        // This handler will be called if the error includes:
        // - an object of type error_code equal to open_error, and
        // - an object of type leaf::e_errno (regardless of its .value), and
        // - an object of type leaf::e_file_name.
        []( leaf::match<error_code, open_error>, leaf::e_errno const & errn, leaf::e_file_name const & fn )
        {
            std::cerr << "Failed to open " << fn.value << ", errno=" << errn << std::endl;
            return 2;
        },

        // This handler will be called if the error includes:
        // - an object of type error_code equal to any of size_error,
        //   read_error, eof_error, and
        // - an optional object of type leaf::e_errno (regardless of its
        //   .value), and
        // - an object of type leaf::e_file_name.
        []( leaf::match<error_code, size_error, read_error, eof_error>, leaf::e_errno const * errn, leaf::e_file_name const & fn )
        {
            std::cerr << "Failed to access " << fn.value;
            if( errn )
                std::cerr << ", errno=" << *errn;
            std::cerr << std::endl;
            return 3;
        },

        // This handler will be called if the error includes:
        // - an object of type error_code equal to output_error, and
        // - an object of type leaf::e_errno (regardless of its .value),
        []( leaf::match<error_code, output_error>, leaf::e_errno const & errn )
        {
            std::cerr << "Output error, errno=" << errn << std::endl;
            return 4;
        },

        // This handler will be called if we've got a bad_command_line
        []( leaf::match<error_code, bad_command_line> )
        {
            std::cout << "Bad command line argument" << std::endl;
            return 5;
        },

        // This last handler matches any error: it prints diagnostic information
        // to help debug logic errors in the program, since it failed to match
        // an appropriate error handler to the error condition it encountered.
        // In this program this handler will never be called.
        []( leaf::error_info const & unmatched )
        {
            std::cerr <<
                "Unknown failure detected" << std::endl <<
                "Cryptic diagnostic information follows" << std::endl <<
                unmatched;
            return 6;
        } );
}


// Implementations of the functions called by main:


// Parse the command line, return the file name.
char const * parse_command_line( int argc, char const * argv[] )
{
    if( argc==2 )
        return argv[1];
    else
        leaf::throw_exception(bad_command_line);
}


// Open a file for reading.
std::shared_ptr<FILE> file_open( char const * file_name )
{
    if( FILE * f = fopen(file_name, "rb") )
        return std::shared_ptr<FILE>(f, &fclose);
    else
        leaf::throw_exception(open_error, leaf::e_errno{errno});
}


// Return the size of the file.
std::size_t file_size( FILE & f )
{
    auto load = leaf::on_error([] { return leaf::e_errno{errno}; });

    if( fseek(&f, 0, SEEK_END) )
        leaf::throw_exception(size_error);

    long s = ftell(&f);
    if( s==-1L )
        leaf::throw_exception(size_error);

    if( fseek(&f,0,SEEK_SET) )
        leaf::throw_exception(size_error);

    return std::size_t(s);
}


// Read size bytes from f into buf.
void file_read( FILE & f, void * buf, std::size_t size )
{
    std::size_t n = fread(buf, 1, size, &f);

    if( ferror(&f) )
        leaf::throw_exception(read_error, leaf::e_errno{errno});

    if( n!=size )
        leaf::throw_exception(eof_error);
}
