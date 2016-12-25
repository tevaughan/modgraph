
#ifndef MODGRAPH_NODE_HPP
#define MODGRAPH_NODE_HPP

#include <set>
#include <vector>

namespace modgraph
{
   using subgraph = std::set<int>;

   struct node
   {
      int              next;
      std::vector<int> prev;
      subgraph *       subg;
      node() : next(-1), subg(nullptr) {}
   };
}

#endif // ndef MODGRAPH_NODE_HPP

