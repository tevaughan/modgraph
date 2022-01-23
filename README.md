# modgraph

A neat, unique graph is produced by picking a modulus N and then squaring each
nonnegative integer less than N.  The orbit of a point n under composition of
the map

<img src="https://latex.codecogs.com/svg.latex?f(n)=n^2\mod{N}"/>

traces zero or more edges of a directed graph until either
- the orbit terminates in a node that maps to itself, or
- the orbit enters a cycle consisting of two or more nodes.

My project allows one to pick a modulus and then easily and quickly to render
the set of disjoint graphs associated with that modulus.

In order to use this tool on a unix-like machine:
- Install [asymptote](https://asymptote.sourceforge.io).
- Install a C++-compiler like `clang++` or `g++`.
- Install `libgsl-dev`.
- Install GNU `make`.
- Clone this repository.
- At the top level, type (for example) `make 33` in order to bring up a
  rendering of the graphs for the modulus, 33.
- The renderer is `asy` (part of `asymptote`), which allows one to spin the
  three-dimensionally rendered scene around or to zoom in or out.

Here is a rendering of the graphs for 33:

![33](33.png)

## cmake, gsl, etc.

Adding support for `cmake` is a work in progress.

Also a work in progress is the adding of a C++-library
- for GSL's vector and
- for GSL's minimization-routines (which depends on GSL's vector).

The code for this library is under the directory [gsl](gsl).
  - The C++-interface to `gsl_vector` is implemented but not fully unit-tested.
  - The C++-interface to the minimizers is not implemented yet.
  - The unit-tests are built, run, and reported on for coverage by way of `cmake`.

The top-level [CMakeLists.txt](CMakeLists.txt) controls only building things
under [gsl](gsl).

The top-level [Makefile](Makefile) still defaults to building the old code for
`modgraph`, which does not yet use anything under [gsl](gsl).

However, the top-level [Makefile](Makefile) does have a target, `build`, which
generates a directory, `build` and properly invokes `cmake` there so that the
test-code under [gsl](gsl) can be built, run, and reported on for covereage.

To get started working on the new stuff:

```
$ make build
$ cd build
$ make tests_cov
```

After that, products of the coverage-analysis end up in
  - `build/tests_cov.txt` and
  - `build/tests_cov.html`.

