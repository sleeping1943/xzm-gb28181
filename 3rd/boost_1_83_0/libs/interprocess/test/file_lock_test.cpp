//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2012. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include "util.hpp"
#include "mutex_test_template.hpp"
#include "sharable_mutex_test_template.hpp"
#include "get_process_id_name.hpp"
#include <fstream>
#include <cstdio> //std::remove

using namespace boost::interprocess;

//This wrapper is necessary to have a default constructor
//in generic mutex_test_template functions
class file_lock_lock_test_wrapper
   : public boost::interprocess::file_lock
{
   public:
   file_lock_lock_test_wrapper()
      :  boost::interprocess::file_lock(get_filename().c_str())
   {}
};

int main ()
{
   //Destroy and create file
   {
      std::remove(get_filename().c_str());
      std::ofstream file(get_filename().c_str());
      if(!file){
         return 1;
      }
      {
         file_lock flock(get_filename().c_str());
         {
         scoped_lock<file_lock> sl(flock);
         }
         {
         scoped_lock<file_lock> sl(flock, try_to_lock);
         }
         {
         scoped_lock<file_lock> sl(flock, test::ptime_delay(1));
         }
         {
         scoped_lock<file_lock> sl(flock, test::boost_systemclock_delay(1));
         }
         {
         scoped_lock<file_lock> sl(flock, test::std_systemclock_delay(1));
         }
      }
      #if defined(BOOST_INTERPROCESS_WCHAR_NAMED_RESOURCES)
      file_lock flock(get_wfilename().c_str());
      #endif
   }
   {
      //Now test move semantics
      file_lock mapping(get_filename().c_str());
      file_lock move_ctor(boost::move(mapping));
      file_lock move_assign;
      move_assign = boost::move(move_ctor);
      mapping.swap(move_assign);
   }

   test::test_all_lock<file_lock_lock_test_wrapper>();
   //test::test_all_mutex<file_lock_lock_test_wrapper>();
   //test::test_all_sharable_mutex<file_lock_lock_test_wrapper>();
   std::remove(get_filename().c_str());

   return 0;
}

