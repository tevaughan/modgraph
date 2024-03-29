# Beauty in Easy Math

![33](doc/example/33.png)

## A Tool for Mathematical Art

The `modgraph` program produces a different three-dimensional scene for each
integer N > 1.  The scene for any choice of N is a representation of the
abstract mathematical graphs produced by squaring integers under [modular
arithmetic][mod], with N as the *modulus*.  For each N, there are at least two
graphs.

I have long seen some beauty in such graphs, and `modgraph` represents an
attempt to automate the exploration of mathematical art.

[mod]: https://en/wikipedia.org/wiki/Modular_arithmetic

## How to Install and to Run

In order to use this tool on a unix-like machine:
- Install [asymptote](https://asymptote.sourceforge.io).
- Install [libgsl-dev](https://www.gnu.org/software/gsl).
- Install [libeigen3-dev](https://eigen.tuxfamily.org).
- Install a C++-compiler like `clang++` or `g++`.
  - For `g++`, it is recommended to use `g++-10` or later.
  - There is a known problem with `g++-9` and `-O2` optimization.
  - In that combination, some vertices are misplaced by a small but noticeable
    amount.
- Install GNU `make`.
- Clone this repository, change into the `modgraph` subdirectory, and type (for
  example) `make 33` in order to bring up a rendering of the graphs for the
  modulus, 33.
- Note that the top-level directory of the repository is by default named
  `modgraph`, and one needs to change into the `modgraph` *subdirectory* of the
  local copy, which is likely also named `modgraph`!
```
$ git clone https://github.com/tevaughan/modgraph.git
$ cd modgraph/modgraph
$ make 33  # or whatever number one likes!
```
- The viewer `asy` (part of `asymptote`) allows one interactively with the
  mouse to spin the three-dimensionally rendered scene around and to zoom in or
  out.
- `asy` is launched automatically when an integer is given as the goal to the
  `make` command while in the `modgraph` subdirectory.

## Examples to Illustrate the Idea

The first example gives the idea about what each graph is.  The second gives an
idea of the structure that one can find.

Sometimes the complexity of the graphs for a large modulus makes it so that the
structure cannot be fully appreciated by viewing from a single point of view.
Note that when `modgraph`'s Makefile launches the viewer, one can spin the
three-dimensional model around in real-time.

### First, consider N = 8.

Each [node][node] in each [graph][graph] is just one of the numbers, 0, 1, 2,
3, 4, 5, 6, and 7, that are used in modular arithmetic when the modulus is 8.
For modulus N, the allowable numbers are 0, 1, 2, ..., N - 1.

To see how the nodes are connected, one squares each node's value and sees what
the result is under the rules of modular arithmetic: One squares each number
and sees what the remainder is after dividing by the modulus.

Each of 0 and 1 is has a simple result: The square of each is itself.
- 0 points back to 0 (to itself).
- 1 points back to 1 (to itself).

But what about the others?

The square of 2 is 4.
- 2 points to 4.

The square of 3 is ordinarily 9.
- However, if the modulus be 8, then then biggest allowable number is 7.
- If the modulus be N, then any m > N is represented by the remainder of m / N.
- So if the modulus be 8, then 9 is represented by the remainder of 9 / 8.
- That remainder is 1, and so the square of 3 is 1 when the modulus is 8.
- 3 points to 1.

The square of 4 is ordinarily 16.
- The remainder of 16 / 8 is 0.
- 4 points to 0.

The square of 5 is ordinarily 25.
- The remainder of 25 / 8 is 1.
- 5 points to 1.

The square of 6 is ordinarily 36.
- The remainder of 36 / 8 is 4.
- 6 points to 4.

The square of 7 is ordinarily 49.
- The remainder of 49 / 8 is 1.
- 7 points to 1.

So the result might be rendered as something like this:

![8](doc/example/8.png)

[node]: https://en.wikipedia.org/wiki/Vertex_(graph_theory)
[graph]: https://en.wikipedia.org/wiki/Graph_(discrete_mathematics)

### Next, consider N = 23.

![23](doc/example/23.png)

## Brief Mathematical Description

For each integer N > 1, a unique set of two or more disjoint, [directed
graphs][digraph] corresponds to the map of squared integers modulo N.

The word "map" is just another word for "function."  The phrase "modulo N"
means something like either "to the modulus N" (if I interpret "modulo" in the
dative case) or "in the modulus N" (if I interpreset "modulo" in the ablative
case).

The idea of exploring the map of *squares* in modular arithmetic is something
that I first saw in *Scientific American* in the 1980s, but I do not have a
reference for the article.

The [orbit][orbit] of a node n under [iteration][iteration] of the function
(the map)

<img src="https://latex.codecogs.com/svg.latex?f(n)=n^2\mod{N}"/>

follows the edges of the graph until either
- the orbit terminates in a node that maps to itself, or
- the orbit enters a cycle consisting of two or more nodes.

The "orbit" of n is just the sequence of numbers consisting of n, the square of
n, the square of the square of n, the square of the square of the square of n,
etc.

The `modgraph` program allows one to pick a modulus and then easily and quickly
to see and to interact with a three-dimensional rendering of the graphs
associated with that modulus.

[digraph]: https://en.wikipedia.org/wiki/Directed_graph
[orbit]: https://en.wikipedia.org/wiki/Orbit_(dynamics)#Discrete_time_dynamical_system
[iteration]: https://en.wikipedia.org/wiki/Iterated_function

## Beauty, Symmetry, and Simplicity

There seems to be beauty in symmetry.

Also, there seems always to be symmetry--and therefore beauty--in any complex
thing that is arranged in the simplest possible way.

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
ordering of parts.

For example, consider liquid water, which is understood as a collection of
water-molecules.  When liquid water cools, the standard physical model of the
water holds that the water loses energy.  Eventually, the loss of energy from
the water allows the molecules to form a highly ordered state, a crystal of
ice.  A crystal of ice seems beautiful both in its ordinary appearance and in
its theoretical description, as the symmetric, minimum-energy state for a
collection of water-molecules.

The idea, that there is beauty, symmetry, and order in a minimum-energy state,
comes into play in `modgraph`, as I describe below.

## My First Encounter With Modular Arithmetic

A few years before I met Vitebsky, I was an undergraduate at the University of
Oklahoma.  While there I had the habit of reading every issue of *Scientific
American*.  At the time (in the late 1980s), there used to be, in that
magazine, a regularly appearing column of words, always under the same title,
"Mathematical Recreations."  In each installment of Mathematical Recreations,
the author illustrated some simple but fun mathematical idea.  In one of those
articles, the author introduced the map of squared integers under modular
arithmetic.

What attracted me to the idea was the beauty in the illustrations accompanying
the article. Each illustration was a two-dimensional depiction of the graphs
associated with a different modulus.  Most likely, each illustration was drawn
by an artist who arranged the nodes and edges in the most symmetric, most
pleasing way that occurred to him.

## From Doodles to Computer-Programs

Over the decades that have elapsed since I was a young man, I have revisited
the idea of the map of squares under a modulus.  At first, as a passtime, I
drew graphs by hand and tried to see how to elicit maximal symmetry in two
dimensions on paper.  After a while, I wrote some software to generate the
graphs in an automated way.  This was a great help because the computations
required to find the graphs for a large modulus are tedious.  Yet I still had
to draw the graphs iteratively by hand and manually to search for the most
symmetric rendering on paper.

Eventually I realized that, to automate the process, I should make a physical
model in which every node repels every other node by a force that varies as the
inverse of the square of the distance between nodes.  In order to keep the
system from flying apart because of universal repulsion between every node and
every other node, and in order to establish spatial relationships, each of
which mirrored a numerical relationship, for each of certain pairs of nodes I
established an attraction by a spring-force that increases linearly with the
distance between the nodes in the pair.  After doing this, I could find the
minimum-energy configuration numerically!

One problem with this approach is that, in two dimensions, the nodes are not
sufficiently free to move. One can find a configuration that *locally*
minimizes the energy, but, when the modulus grows above 20 or so, one sees that
the nonlinear minimizer often fails to find the *global* minimum. In three
dimensions, though, there is plenty of room for nodes to slip around each other
as the system relaxes into its minimum-energy configuration.

Hence the project here.

## Forces of Attraction

The springs, each attracting a pair of nodes, are invisible in a rendered
scene.  There is an attractive force between any two nodes i and j
- if  f(i) = j  (so that there is an arrow from one to the other),
- if  i be a factor of N,
- if  j be a factor of N,
- if  N - i be a factor of N,
- if  N - j be a factor of N,
- if  [i + j] mod N  be 0,
- if  [i + j] mod N  be a factor of N, or
- if  N - [[i + j] mod N]  be a factor of N.

Note that 1 is *not* considered to be a factor of N, but 0 *is* considered to
be a factor.  (0 stands for N, which is a factor of itself, in modular
arithmetic.)

These attractions give almost every graph a high degree of symmetry at minimum
potential energy.

