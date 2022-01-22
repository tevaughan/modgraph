# modgraph

Graph produced by picking a modulus N and tracing each orbit that starts at a
number i in {0,1,2,...,N} and proceeds by squaring the current number modulo N
in order to obtain the next number in the orbit.

## cmake, gsl, etc.

Adding support for `cmake` is a work in progress.

Also a work in progress is the adding a C++-library for GSL's vector and for
GSL's minimization-routines.

The code for this library is under the directory `gsl`.
  - The C++-interface to `gsl_vector` is implemented but not fully unit-tested.
  - The C++-interface to the minimizers is not implemented yet.
  - The unit-tests are built, run, and reported on for coverage by way of `cmake`.

The top-level `CMakeLists.txt` controls only building stuff under `gsl`.

The top-level `Makefile` still defaults to building the old code for
`modgraph`, which does not yet use anything under `gsl`.

However, the top-level `Makefile` does have a target, `build`, which generates
a directory, `build` and properly invokes `cmake` there so that the test-code
under `gsl` can be built, run, and reported on for covereage.

So to get started working on the new stuff:

```
$ make build
$ cd build
$ make tests_cov
```

The products of the coverage-analysis will be in
  - `build/tests_cov.txt` and
  - `build/tests_cov.html`.

