
#ifndef MODGRAPH_GRAPH_HPP
#define MODGRAPH_GRAPH_HPP

#include <iostream>
#include <fstream>
#include <sstream>

#include "node.hpp"

namespace modgraph
{
   class graph
   {
      std::vector<subgraph> subgraphs_;
      std::vector<node>     nodes_;

      /// Mark node n and every connected node as belonging to subgraph s.
      void traverse(node &n, subgraph &s)
      {
         n.subg = &s;
         s.insert(&n);
         if (n.next->subg == nullptr) {
            traverse(*n.next, s);
         } else if (n.next->subg != &s) {
            throw "conflict between subgraphs (next)";
         }
         for (auto p : n.prev) {
            if (p->subg == nullptr) {
               traverse(*p, s);
            } else if (p->subg != &s) {
               throw "conflict between subgraphs (prev)";
            }
         }
      }

   public:
      /// Construct graph for modulus m.
      graph(unsigned m) : nodes_(m)
      {
         // Establish all interconnections among nodes.
         for (unsigned i = 0; i < m; ++i) {
            unsigned const n = (i * i) % m; // offset of next
            nodes_[i].value  = i;
            nodes_[i].next   = &nodes_[n];
            nodes_[n].prev.push_back(&nodes_[i]);
         }
         // Partition into subgraphs.
         for (unsigned i = 0; i < m; ++i) {
            if (nodes_[i].subg == nullptr) {
               // Need new subgraph.
               subgraphs_.push_back(subgraph());
               traverse(nodes_[i], *subgraphs_.rbegin());
            }
         }
      }

      /// Write text file for neato.
      void write() const
      {
         unsigned const m = nodes_.size();
         using namespace std;
         ostringstream oss;
         oss << m << ".neato";
         ofstream ofs(oss.str());
         ofs << "digraph G {\n";
         for (unsigned i = 0; i < m; ++i) {
            ofs << "   " << i << " -> " << nodes_[i].next->value << "\n";
         }
         for (unsigned i = 0; i < subgraphs_.size(); ++i) {
            ofs << "   subgraph s" << i << " {\n";
            for (auto j : subgraphs_[i]) {
               ofs << "      " << j->value << "\n";
            }
            ofs << "   }\n";
         }
         ofs << "}\n";
      }
   };
}

#endif // ndef MODGRAPH_GRAPH_HPP

