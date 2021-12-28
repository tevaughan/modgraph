
#include "graph.hpp"
#include <iostream> // cerr

using namespace modgraph;
using namespace std;

int main(int argc, char** argv)
{
   if (argc != 2) {
      cerr << "need exactly one argument" << endl;
      return 1;
   }
   istringstream iss(argv[1]);
   unsigned m;
   iss >> m;
   graph g(m);
   return 0;
}

