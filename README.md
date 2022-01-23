# Beauty in Easy Math

![33](33.png)

## A Tool for Mathematical Art

The purpose of the `modgraph` project is, for any integer N > 1 that one might
pick, to produce a three-dimensional graph that is beautiful to behold.  Thus
the project might qualify as an effort at mathematical art.

## Brief Mathematical Description

For each integer N > 1, a unique set of two or more disjoint, [directed
graphs][graph] corresponds to the map of squared integers modulo N.

For information about what "modulo N" means, see the [article in
Wikipedia][mod] on modular arithmetic. The word "map" is just another word for
"function." The idea of the map of *squares* in modular arithmetic is something
that I first saw in *Scientific American* in the 1980s, but I do not have a
reference for the article.

The [orbit][orbit] of a point n under [iteration (self-composition)][iteration]
of the function (the "*map*")

<img src="https://latex.codecogs.com/svg.latex?f(n)=n^2\mod{N}"/>

follows the edges of the graph until either
- the orbit terminates in a node that maps to itself, or
- the orbit enters a cycle consisting of two or more nodes.

For example, consider N = 8.  The [nodes (or vertices)][node] that make up the
graphs are just the numbers (0, 1, 2, 3, 4, 5, 6, and 7) that are used in
modular arithmetic when the modulus is 8.  To see how they are connected, we
square each one and see what the result is under the rules of modular
arithmetic.  That is, we square each number and see what the remainder of the
square is after dividing by the modulus.

Each of 0 and 1 is has a simple result: Each maps to itself.  But what about
the others?

2 maps to 4, which maps to 0.

3 maps to 1.

5 maps to 1.

6 maps to 4.

7 maps to 1.

So the result might be rendered as something like this:

![8](8.png)

The `modgraph` program allows one to pick a modulus and then easily and quickly
to see and to interact with a three-dimensional rendering of the graphs
associated with that modulus.

[graph]: https://en.wikipedia.org/wiki/Directed_graph
[mod]: https://en/wikipedia.org/wiki/Modular_arithmetic
[orbit]: https://en.wikipedia.org/wiki/Orbit_(dynamics)#Discrete_time_dynamical_system
[iteration]: https://en.wikipedia.org/wiki/Iterated_function
[node]: https://en.wikipedia.org/wiki/Vertex_(graph_theory)

## Beauty, Symmetry, and Simplicity

There seems to be beauty in symmetry.

Moreover, there seems always to be symmetry, and therefore beauty, in any
complex thing that is arranged in the simplest possible way.

In the physical world, the idea of simplicity in a complex thing seems to be
associated with the minimum internal energy of the thing.  One way to search
for beauty in the physical world is to look for things that are symmetric.
When I was a graduate-student in physics (still enrolled as a student but
working at a job away from school), I met a physicist, Ilya Vitebsky, who
pointed out to me that, in nature, any physical system tends to show its
maximally symmetric arrangement when the system falls into the configuration
corresponding to its minimum internal energy.  At minimum energy in a system
made of parts, maximum symmetry is also associated with the idea of maximum
order among the parts, and there seems to be something beautiful in a right
ordering of parts. That idea comes into play here, as I describe below.

A few years earlier, while I was an undergraduate at the University of
Oklahoma, I had the habit of reading every issue of *Scientific American*.  At
the time (in the late 1980s), there used to be, in that magazine, a regularly
appearing article, always under the same title, "Mathematical Recreations." In
each installment of Mathematical Recreations, the author illustrated some
simple but fun mathematical idea.  In one of those articles, the author
introduced the map of squared integers under modular arithmetic.

What attracted me to the idea was the beauty in the illustrations accompanying
the article. Each illustration was a two-dimensional depiction of the graphs
associated with a different modulus.  Most likely, each illustration was drawn
by an artist who arranged the nodes and edges in the most symmetric, most
pleasing way that occurred to him.

Over the decades that have elapsed since I was a young man, I have revisited
the idea of the map of squares under a modulus. At first, as a passtime, I drew
graphs by hand and tried to see how to elicit maximal symmetry in two
dimensions on paper.  After a while, I wrote some software to generate the
graphs in an abstract way. This was a great help because the computations
required to find the graphs for a large modulus are tedious. But I still had to
draw the graphs iteratively by hand and manually to search for the most
symmetric rendering on paper.

Eventually I realized that, to automate the process, I should make a physical
model in which every node repels every other node by a force that varies as the
inverse of the square of the distance between nodes. Then certain pairs of
nodes would be attracted by a spring-force that varies linearly with the
distance between nodes. Thus I could find the minimum-energy configuration
numerically!

One problem with this approach is that, in two dimensions, the nodes are not
sufficiently free to move. One can find a configuration that *locally*
minimizes the energy, but, when the modulus grows above 20 or so, one sees that
the nonlinear minimizer often fails to find the *global* minimum. In three
dimensions, though, there is plenty of room for nodes to slip around each other
as the system relaxes into its minimum-energy configuration.

Hence the project here.

## How to Install and to Run

In order to use this tool on a unix-like machine:
- Install [asymptote](https://asymptote.sourceforge.io).
- Install [libgsl-dev](https://www.gnu.org/software/gsl).
- Install [libeigen3-dev](https://eigen.tuxfamily.org).
- Install a C++-compiler like `clang++` or `g++`.
- Install GNU `make`.
- Clone this repository.
- At the top level, type (for example) `make 33` in order to bring up a
  rendering of the graphs for the modulus, 33.
- The viewer is `asy` (part of `asymptote`), which allows one interactively
  with the mouse to spin the three-dimensionally rendered scene around and to
  zoom in or out.

## In Progress: cmake, gsl, etc.

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

