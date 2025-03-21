#!/usr/bin/env python3

# Copyright (C) Vladimir Prus 2006.
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE.txt or copy at
# https://www.bfgroup.xyz/b2/LICENSE.txt)

# Test the 'libraries' example.

import BoostBuild

t = BoostBuild.Tester(use_test_config=False)

t.set_tree("../example/libraries")

t.run_build_system()

t.expect_addition(["app/bin/$toolset/debug*/app.exe",
                   "util/foo/bin/$toolset/debug*/bar.dll"])

t.cleanup()
