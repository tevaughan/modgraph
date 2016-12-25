
#ifndef MODGRAPH_GRAPH_HPP
#define MODGRAPH_GRAPH_HPP

#include <fstream>
#include <iostream>
#include <sstream>

#include "node.hpp"

namespace modgraph
{
   class graph
   {
      std::vector<subgraph> subgraphs_;
      std::vector<node>     nodes_;

      // Check previous or next node for connection to subgraph s.
      void check_node(int off, subgraph &s)
      {
         node &n = nodes_[off];
         if (n.subg == nullptr) {
            traverse(off, s);
         } else if (n.subg != &s) {
            throw "conflict between subgraphs";
         }
      }

      /// Mark node at offset off and every connected node as belonging to
      /// subgraph s.
      void traverse(int off, subgraph &s)
      {
         node &n = nodes_[off];
         n.subg  = &s;
         s.insert(off);
         // Handle next node.
         int const nxt_off = n.next;
         check_node(nxt_off, s);
         // Handle previous nodes.
         for (int const prv_off : n.prev) {
            check_node(prv_off, s);
         }
      }

      /// Write text files for neato.
      void write() const
      {
         unsigned const m = nodes_.size();
         using namespace std;
         for (unsigned s = 0; s < subgraphs_.size(); ++s) {
            ostringstream oss;
            oss << m << "." << s << ".neato";
            ofstream ofs(oss.str());
            ofs << "digraph G {\n";
            for (int i : subgraphs_[s]) {
               ofs << "   " << i << " -> " << nodes_[i].next << "\n";
            }
            ofs << "}\n";
         }
      }

   public:
      /// Construct graph for modulus m.
      graph(unsigned m) : nodes_(m)
      {
         // Establish all interconnections among nodes.
         for (unsigned cur_off = 0; cur_off < m; ++cur_off) {
            unsigned const nxt_off = (cur_off * cur_off) % m; // offset of next
            nodes_[cur_off].next   = nxt_off;
            nodes_[nxt_off].prev.push_back(cur_off);
         }
         // Partition into subgraphs.
         for (unsigned off = 0; off < m; ++off) {
            if (nodes_[off].subg == nullptr) {
               // Need new subgraph.
               subgraphs_.push_back(subgraph());
               traverse(off, *subgraphs_.rbegin());
            }
         }
         // Write text files.
         write();
      }
   };
}

#endif // ndef MODGRAPH_GRAPH_HPP

