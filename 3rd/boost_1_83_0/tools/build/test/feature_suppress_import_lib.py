#!/usr/bin/env python3

# Copyright 2018 Steven Watanabe
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at
# https://www.bfgroup.xyz/b2/LICENSE.txt)

# Tests the suppress-import-lib feature

# This used to cause the pdb and the import lib to get mixed up
# if there are any exports.

import BoostBuild

t = BoostBuild.Tester(use_test_config=False)

t.write("Jamroot.jam", """
lib l : l.cpp : <suppress-import-lib>true ;
""")

t.write("l.cpp", """
void
#ifdef _WIN32
__declspec(dllexport)
#endif
f() {}
""")

t.run_build_system()
t.expect_addition("bin/$toolset/debug*/l.obj")
t.expect_addition("bin/$toolset/debug*/l.dll")

t.cleanup()
