
#ifndef MODGRAPH_GRAPH_HPP
#define MODGRAPH_GRAPH_HPP

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "node.hpp"

namespace modgraph
{
   class graph
   {
      using subgraph = std::set<int>;
      std::vector<subgraph> subgraphs_;
      std::vector<node>     nodes_;

      // Check previous or next node for connection to subgraph at offset
      // s_off.
      void check_node(int n_off, int s_off)
      {
         node &n = nodes_[n_off];
         if (n.subg == -1) {
            traverse(n_off, s_off);
         } else if (n.subg != s_off) {
            throw "conflict between subgraphs";
         }
      }

      /// Mark node at offset n_off and every connected node as belonging to
      /// subgraph at offset s_off.
      void traverse(int n_off, int s_off)
      {
         node &n = nodes_[n_off];
         n.subg  = s_off;
         subgraphs_[s_off].insert(n_off);
         // Handle next node.
         int const nxt_off = n.next;
         check_node(nxt_off, s_off);
         // Handle previous nodes.
         for (int const prv_off : n.prev) {
            check_node(prv_off, s_off);
         }
      }

      /// Write text files for neato.
      void write() const
      {
         unsigned const m = nodes_.size();
         using namespace std;
         for (unsigned s_off = 0; s_off < subgraphs_.size(); ++s_off) {
            ostringstream oss;
            oss << m << "." << s_off << ".neato";
            ofstream ofs(oss.str());
            ofs << "digraph G {\n";
            for (int n_off : subgraphs_[s_off]) {
               ofs << "   " << n_off << " -> " << nodes_[n_off].next << "\n";
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
         for (unsigned n_off = 0; n_off < m; ++n_off) {
            if (nodes_[n_off].subg == -1) {
               // Need new subgraph.
               int const s_off = subgraphs_.size();
               subgraphs_.push_back(subgraph());
               traverse(n_off, s_off);
            }
         }
         // Write text files.
         write();
      }
   };
}

#endif // ndef MODGRAPH_GRAPH_HPP

