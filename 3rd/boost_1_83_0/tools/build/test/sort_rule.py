#!/usr/bin/env python3

# Copyright (C) 2008. Jurko Gospodnetic
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at
# https://www.bfgroup.xyz/b2/LICENSE.txt)

# Tests for the Boost Jam builtin SORT rule.

from __future__ import print_function

import BoostBuild


###############################################################################
#
# testSORTCorrectness()
# ---------------------
#
###############################################################################

def testSORTCorrectness():
    """Testing that Boost Jam's SORT builtin rule actually sorts correctly."""
    t = BoostBuild.Tester(["-ftest.jam", "-d1"], pass_toolset=False,
        use_test_config=False)

    t.write("test.jam", """\
NOCARE all ;
source-data = 1 8 9 2 7 3 4 7 1 27 27 9 98 98 1 1 4 5 6 2 3 4 8 1 -2 -2 0 0 0 ;
target-data = -2 -2 0 0 0 1 1 1 1 1 2 2 27 27 3 3 4 4 4 5 6 7 7 8 8 9 9 98 98 ;
ECHO "starting up" ;
sorted-data = [ SORT $(source-data) ] ;
ECHO "done" ;
if $(sorted-data) != $(target-data)
{
    ECHO "Source       :" $(source-data) ;
    ECHO "Expected     :" $(target-data) ;
    ECHO "SORT returned:" $(sorted-data) ;
    EXIT "SORT error" : -2 ;
}
""")

    t.run_build_system()
    t.expect_output_lines("starting up")
    t.expect_output_lines("done")
    t.expect_output_lines("SORT error", False)

    t.cleanup()


###############################################################################
#
# testSORTDuration()
# ------------------
#
###############################################################################

def testSORTDuration():
    """
      Regression test making sure Boost Jam's SORT builtin rule does not get
    quadratic behaviour again in this use case.

    """
    t = BoostBuild.Tester(["-ftest.jam", "-d1"], pass_toolset=False,
        use_test_config=False)

    f = open(t.workpath("test.jam"), "w")
    print("data = ", file=f)
    for i in range(0, 20000):
        if i % 2:
            print('"aaa"', file=f)
        else:
            print('"bbb"', file=f)
    print(""";

ECHO "starting up" ;
sorted = [ SORT $(data) ] ;
ECHO "done" ;
NOCARE all ;
""", file=f)
    f.close()

    t.run_build_system(expected_duration=1)
    t.expect_output_lines("starting up")
    t.expect_output_lines("done")

    t.cleanup()


###############################################################################
#
# main()
# ------
#
###############################################################################

testSORTCorrectness()
testSORTDuration()
