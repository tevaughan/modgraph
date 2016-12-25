
#ifndef MODGRAPH_NODE_HPP
#define MODGRAPH_NODE_HPP

#include <set>
#include <vector>

namespace modgraph
{
   struct node;

   using subgraph = std::set<node*>;

   struct node
   {
      unsigned            value;
      node *              next;
      std::vector<node *> prev;
      subgraph *          subg;
      node() : value(0), next(nullptr), subg(nullptr) {}
   };
}

#endif // ndef MODGRAPH_NODE_HPP

