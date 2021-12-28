// Use '$ asy -V test' to view scene corresponding to this file.
//
// This file is intended to demonstrate basic approach for visualizing graph
// corresponding to modulo-N square of each nonnegative integer i < N.
//
// Each i can be represented by a billboarded label, with an arrow drawn from
// one label to another.
//
// A file like this one is to be generated by the program modgraph, after it
// randomly places each node in a region of 3-d space and then allows nodes to
// migrate into equilibrium in attractive and repulsive force scheme.

settings.outformat = "pdf";
settings.prc = false;
unitsize(1cm);
import three;

currentprojection=perspective(1,-2,1);
label("2",(0,0,0),red,Billboard);
label("4",(1,1,1),red,Billboard);
draw((0,0,0)--(1,1,1), arrow=Arrow3(), p=gray(0.6), light=currentlight);
