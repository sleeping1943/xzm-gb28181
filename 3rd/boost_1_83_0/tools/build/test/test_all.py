#!/usr/bin/env python3

# Copyright 2002-2005 Dave Abrahams.
# Copyright 2002-2006 Vladimir Prus.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE.txt or copy at
#         https://www.bfgroup.xyz/b2/LICENSE.txt)

from __future__ import print_function

import BoostBuild

import concurrent.futures
import os
import os.path
import time
import signal
import sys

xml = "--xml" in sys.argv
toolset = BoostBuild.get_toolset()


# Clear environment for testing.
#
for s in ("BOOST_ROOT", "BOOST_BUILD_PATH", "JAM_TOOLSET", "BCCROOT",
    "MSVCDir", "MSVC", "MSVCNT", "MINGW", "watcom"):
    try:
        del os.environ[s]
    except:
        pass

BoostBuild.set_defer_annotations(1)


def iterfutures(futures):
    while futures:
        done, futures = concurrent.futures.wait(
            futures,return_when=concurrent.futures.FIRST_COMPLETED)
        for future in done:
            yield future, futures


def run_test(test):
    ts = time.perf_counter()
    exc = None
    try:
        __import__(test)
    except BaseException as e:
        exc = e
    return test, time.perf_counter() - ts, exc, BoostBuild.annotations


def run_tests(critical_tests, other_tests):
    """
      Runs first the critical_tests and then the other_tests.

      Writes the name of the first failed test to test_results.txt. Critical
    tests are run in the specified order, other tests are run starting with the
    one that failed first on the last test run.

    """
    last_failed = last_failed_test()
    other_tests = reorder_tests(other_tests, last_failed)
    all_tests = critical_tests + other_tests

    invocation_dir = os.getcwd()
    max_test_name_len = 10
    for x in all_tests:
        if len(x) > max_test_name_len:
            max_test_name_len = len(x)

    cancelled = False
    executor = concurrent.futures.ProcessPoolExecutor()

    def handler(sig, frame):
        cancelled = True
        processes = executor._processes.values()
        executor.shutdown(wait=False, cancel_futures=True)
        for process in processes:
            process.terminate()

    signal.signal(signal.SIGINT, handler)

    pass_count = 0
    failures_count = 0
    start_ts = time.perf_counter()
    isatty = sys.stdout.isatty() or "--interactive" in sys.argv
    futures = {executor.submit(run_test, test): test for test in all_tests}
    for future, pending in iterfutures(futures):
        test = futures[future]
        if not xml:
            s = "%%-%ds :" % max_test_name_len % test
            if isatty:
                s = f"\r{s}"
            print(s, end='')

        passed = 0
        ts = float('nan')
        try:
            test, ts, exc, annotations = future.result()
            BoostBuild.annotations += annotations
            if exc is not None:
                raise exc from None
            passed = 1
        except concurrent.futures.process.BrokenProcessPool:
            # It could be us who broke the pool by terminating its threads
            if not cancelled:
                raise
        except KeyboardInterrupt:
            """This allows us to abort the testing manually using Ctrl-C."""
            print("\n\nTesting was cancelled by external signal.")
            cancelled = True
            break
        except SystemExit as e:
            """This is the regular way our test scripts are supposed to report
            test failures."""
            if e.code is None or e.code == 0:
                passed = 1
        except:
            exc_type, exc_value, exc_tb = sys.exc_info()
            try:
                BoostBuild.annotation("failure - unhandled exception", "%s - "
                    "%s" % (exc_type.__name__, exc_value))
                BoostBuild.annotate_stack_trace(exc_tb)
            finally:
                #   Explicitly clear a hard-to-garbage-collect traceback
                # related reference cycle as per documented sys.exc_info()
                # usage suggestion.
                del exc_tb

        if passed:
            pass_count += 1
        else:
            failures_count += 1
            if failures_count == 1:
                f = open(os.path.join(invocation_dir, "test_results.txt"), "w")
                try:
                    f.write(test)
                finally:
                    f.close()

        #   Restore the current directory, which might have been changed by the
        # test.
        os.chdir(invocation_dir)

        if not xml:
            if passed:
                print(f"PASSED {ts * 1000:>5.0f}ms")
            else:
                print(f"FAILED {ts * 1000:>5.0f}ms")
                BoostBuild.flush_annotations()

            if isatty:
                msg = ", ".join(futures[future] for future in pending if future.running())
                if msg:
                    msg = f"[{len(futures) - len(pending)}/{len(futures)}] {msg}"
                    max_len = max_test_name_len + len(" :PASSED 12345ms")
                    if len(msg) > max_len:
                        msg = msg[:max_len - 3] + "..."
                    print(msg, end='')
        else:
            rs = "succeed"
            if not passed:
                rs = "fail"
            print('''
<test-log library="build" test-name="%s" test-type="run" toolset="%s" test-program="%s" target-directory="%s">
<run result="%s">''' % (test, toolset, "tools/build/v2/test/" + test + ".py",
                "boost/bin.v2/boost.build.tests/" + toolset + "/" + test, rs))
            if not passed:
                BoostBuild.flush_annotations(1)
            print('''
</run>
</test-log>
''')
        sys.stdout.flush()  # Makes testing under emacs more entertaining.
        BoostBuild.clear_annotations()

    # Erase the file on success.
    if failures_count == 0:
        open("test_results.txt", "w").close()

    if not xml:
        print(f'''
        === Test summary ===
        PASS: {pass_count}
        FAIL: {failures_count}
        TIME: {time.perf_counter() - start_ts:.0f}s
        ''')

    # exit with failure with failures
    if cancelled or failures_count > 0:
        sys.exit(1)

def last_failed_test():
    "Returns the name of the last failed test or None."
    try:
        f = open("test_results.txt")
        try:
            return f.read().strip()
        finally:
            f.close()
    except Exception:
        return None


def reorder_tests(tests, first_test):
    try:
        n = tests.index(first_test)
        return [first_test] + tests[:n] + tests[n + 1:]
    except ValueError:
        return tests


critical_tests = ["docs", "unit_tests", "module_actions", "core_d12",
    "core_typecheck", "core_delete_module", "core_language", "core_arguments",
    "core_varnames", "core_import_module"]

# We want to collect debug information about the test site before running any
# of the tests, but only when not running the tests interactively. Then the
# user can easily run this always-failing test directly to see what it would
# have returned and there is no need to have it spoil a possible 'all tests
# passed' result.
if xml:
    critical_tests.insert(0, "collect_debug_info")

tests = ["abs_workdir",
         "absolute_sources",
         "alias",
         "alternatives",
         "always",
         "bad_dirname",
         "build_dir",
         "build_file",
         "build_hooks",
         "build_no",
         "builtin_echo",
         "builtin_exit",
         "builtin_glob",
         "builtin_readlink",
         "builtin_split_by_characters",
         "bzip2",
         "c_file",
         "chain",
         "clean",
         "cli_property_expansion",
         "command_line_properties",
         "composite",
         "conditionals",
         "conditionals2",
         "conditionals3",
         "conditionals4",
         "conditionals_multiple",
         "configuration",
         "configure",
         "copy_time",
         "core_action_output",
         "core_action_status",
         "core_actions_quietly",
         "core_at_file",
         "core_bindrule",
         "core_dependencies",
         "core_syntax_error_exit_status",
         "core_fail_expected",
         "core_jamshell",
         "core_modifiers",
         "core_multifile_actions",
         "core_nt_cmd_line",
         "core_option_d2",
         "core_option_l",
         "core_option_n",
         "core_parallel_actions",
         "core_parallel_multifile_actions_1",
         "core_parallel_multifile_actions_2",
         "core_scanner",
         "core_source_line_tracking",
         "core_update_now",
         "core_variables_in_actions",
         "custom_generator",
         "debugger",
# Newly broken?
#         "debugger-mi",
         "default_build",
         "default_features",
# This test is known to be broken itself.
#         "default_toolset",
         "dependency_property",
         "dependency_test",
         "disambiguation",
         "dll_path",
         "double_loading",
         "duplicate",
         "example_libraries",
         "example_make",
         "exit_status",
         "expansion",
         "explicit",
         "feature_cxxflags",
         "feature_implicit_dependency",
         "feature_relevant",
         "feature_suppress_import_lib",
         "file_types",
         "flags",
         "generator_selection",
         "generators_test",
         "implicit_dependency",
         "indirect_conditional",
         "inherit_toolset",
         "inherited_dependency",
         "inline",
         "install_build_no",
         "lang_asm",
         "libjpeg",
         "liblzma",
         "libpng",
         "libtiff",
         "libzstd",
         "lib_source_property",
         "lib_zlib",
         "library_chain",
         "library_property",
         "link",
         "load_order",
         "loop",
         "make_rule",
         "message",
         "ndebug",
         "no_type",
         "notfile",
         "ordered_include",
# FIXME: Disabled due to bug in B2
#         "ordered_properties",
         "out_of_tree",
         "package",
         "param",
         "path_features",
         "prebuilt",
         "preprocessor",
         "print",
         "project_dependencies",
         "project_glob",
         "project_id",
         "project_root_constants",
         "project_root_rule",
         "project_test3",
         "project_test4",
         "property_expansion",
# FIXME: Disabled due lack of qt5 detection
#         "qt5",
         "rebuilds",
         "relative_sources",
         "remove_requirement",
         "rescan_header",
         "resolution",
         "rootless",
         "scanner_causing_rebuilds",
         "searched_lib",
         "skipping",
         "sort_rule",
         "source_locations",
         "source_order",
         "space_in_path",
         "stage",
         "standalone",
         "static_and_shared_library",
         "suffix",
         "tag",
         "test_rc",
         "test1",
         "test2",
         "testing",
         "timedata",
         "toolset_clang_darwin",
         "toolset_clang_linux",
         "toolset_clang_vxworks",
         "toolset_darwin",
         "toolset_defaults",
         "toolset_gcc",
         "toolset_intel_darwin",
         "toolset_msvc",
         "toolset_requirements",
         "transitive_skip",
         "unit_test",
         "unused",
         "use_requirements",
         "using",
         "wrapper",
         "wrong_project",
         ]

if os.name == "posix":
    tests.append("symlink")
    # On Windows, library order is not important, so skip this test. Besides,
    # it fails ;-). Further, the test relies on the fact that on Linux, one can
    # build a shared library with unresolved symbols. This is not true on
    # Windows, even with cygwin gcc.

#   Disable this test until we figure how to address failures due to --as-needed being default now.
#    if "CYGWIN" not in os.uname()[0]:
#        tests.append("library_order")

if toolset.startswith("gcc") and os.name != "nt":
    # On Windows it's allowed to have a static runtime with gcc. But this test
    # assumes otherwise. Hence enable it only when not on Windows.
    tests.append("gcc_runtime")

if toolset.startswith("clang") or toolset.startswith("gcc") or toolset.startswith("msvc"):
    tests.append("pch")
    tests.append("feature_force_include")

# Clang includes Objective-C driver everywhere, but GCC usually in a separate gobj package
if toolset.startswith("clang") or "darwin" in toolset:
    tests.append("lang_objc")

# Disable on OSX as it doesn't seem to work for unknown reasons.
if sys.platform != 'darwin':
    tests.append("builtin_glob_archive")

if "--extras" in sys.argv:
    tests.append("boostbook")
    tests.append("qt4")
    tests.append("qt5")
    tests.append("example_qt4")
    # Requires ./whatever.py to work, so is not guaranteed to work everywhere.
    tests.append("example_customization")
    # Requires gettext tools.
    tests.append("example_gettext")
elif not xml and __name__ == "__main__":
    print("Note: skipping extra tests")

if __name__ == "__main__":
    run_tests(critical_tests, tests)
