
#ifndef MODGRAPH_NODE_HPP
#define MODGRAPH_NODE_HPP

#include <vector>

namespace modgraph
{
   struct node
   {
      int              next;
      std::vector<int> prev;
      int              subg;
      node() : next(-1), subg(-1) {}
   };
}

#endif // ndef MODGRAPH_NODE_HPP

