
/// @file       graph.cpp
/// @brief      Definition of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream
#include <iostream> // cout, endl

namespace modgraph {


using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::string;


/// Adapter for output of object as string to asy-file.
class asy_adapter {
protected:
  ostringstream oss_; /// Representation of object as string for asy.

  /// Integrate adapter with standard output-stream in C++.
  /// @param s  Reference to output-stream.
  /// @param a  Reference to adapter.
  /// @return  Reference to modified output-stream.
  friend ostream &operator<<(ostream &s, asy_adapter const &a) {
    return s << a.oss_.str();
  }
};


/// Adapter for output of Vector3d to asy-file as position.
struct pos: public asy_adapter {
  /// Initialize representation of position as string for asy.
  /// @param v  Reference to Vector3d that stores components of position.
  pos(Vector3d const &v) {
    oss_ << "(" << v[0] << "," << v[1] << "," << v[2] << ")";
  }
};


/// Adapter for output of Vector3d to asy-file as shift.
struct shift: public asy_adapter {
  /// Initialize representation of shift as string for asy.
  /// @param v  Reference to Vector3d that stores components of shift.
  shift(Vector3d const &v) { oss_ << "shift" << pos(v); }
};


/// Adapter for output of draw-command for sphere to asy-file.
struct sphere: public asy_adapter {
  /// Initialize representation of sphere as string for asy.
  /// @param v  Reference to position of sphere.
  /// @param s  Scale of sphere.
  /// @param c  Color of sphere.
  /// @param op  Opacity of sphere.
  sphere(Vector3d const v,
      double s= 0.25,
      string const &c= "white",
      double op= 0.5) {
    oss_ << "draw(" << shift(v) << "*scale3(" << s << ")"
         << "*unitsphere," << c << "+opacity(" << op << "));\n";
  }
};


/// Adapter for output of label-command for a numeric label to asy-file.
struct label: public asy_adapter {
  /// Initialize representation of label-command as string for asy.
  /// @param i  Number for numeric label.
  /// @param v  Reference to position of label.
  /// @param c  Color of label.
  /// @param bb  True for billboard-label (camera-facing); false for embedded.
  label(int i, Vector3d const &v, string const &c= "black", bool bb= true) {
    string const orientation= (bb ? "Billboard" : "Embedded");
    oss_ << "label(" << '"' << i << '"' << "," << pos(v) << "," << c << ","
         << orientation << ");\n";
  }
};


/// Adapter for output of arrow-drawing-command to asy-file.
struct arrow: public asy_adapter {
  /// Initialize representation of arrow-drawing-command as string for asy.
  /// @param b  Beginning coordinates for arrow.
  /// @param e  Ending coordinates for arrow.
  /// @param g  Gray-level of material for arrow.
  /// @param l  Light to use for illuminating arrow.
  arrow(Vector3d const &b,
      Vector3d const &e,
      double g= 0.6,
      string const &l= "currentlight") {
    oss_ << "draw(" << pos(b) << "--" << pos(e) << ","
         << "arrow=Arrow3(),"
         << "p=gray(" << g << "),"
         << "light=" << l << ");\n";
  }
};


/// Adapter for output of perspective for current projection to asy-file.
struct perspective: public asy_adapter {
  /// Initialize representation of perspective as string for asy.
  /// @param v  Camera's location.
  perspective(Vector3d const &v) {
    oss_ << "currentprojection = perspective" << pos(v) << ";\n";
  }
};


/// Adapter for output of basic header to asy-file.
struct header: public asy_adapter {
  /// Initialize representation of header as string for asy.
  /// @param ofmt  Format of output-file (when asy be not invoked with '-V').
  /// @param prc  True if PRC-vector-graphics should be embedded in PDF.
  /// @param unit_cm  Unit of distance (in cm).
  header(string const &ofmt= "pdf",
      string const &prc= "false",
      double unit_cm= 1.0) {
    oss_ << "settings.outformat = " << '"' << ofmt << '"' << ";\n"
         << "settings.prc = " << prc << ";\n"
         << "unitsize(" << unit_cm << "cm);\n"
         << "import three;\n";
  }
};


void graph::write_asy() const {
  int const m= nodes_.size();
  ostringstream oss;
  oss << m << ".asy";
  ofstream ofs(oss.str());
  ofs << header();
  double const ycam= -pow(minimizer_.all_attract() * nodes_.size(), 1.0 / 3.0);
  ofs << perspective({0, ycam, 0});
  for(int i= 0; i < m; ++i) {
    auto const &ap= positions_.col(i);
    ofs << sphere(ap);
    ofs << label(i, ap);
    auto const &an= nodes_[i];
    int const j= an.next;
    if(i != j) {
      auto const &bp= positions_.col(j);
      auto const ab_u= (bp - ap).normalized() * 0.25;
      auto const ab= ap + ab_u;
      auto const ba= bp - ab_u;
      ofs << arrow(ab, ba);
    }
  }
}


void graph::connect() {
  int const m= nodes_.size();
  for(int cur_off= 0; cur_off < m; ++cur_off) {
    int const nxt_off= (cur_off * cur_off) % m; // offset of next
    nodes_[cur_off].next= nxt_off;
    nodes_[nxt_off].prev.push_back(cur_off);
  }
}


MatrixXd graph::init_loc(unsigned m) {
  MatrixXd r(3, m); // Return-value.
  for(unsigned i= 0; i < m; ++i) {
    constexpr double u= 1.0 / RAND_MAX;
    r(0, i)= m * (rand() * u - 0.5);
    r(1, i)= m * (rand() * u - 0.5);
    r(2, i)= m * (rand() * u - 0.5);
  }
  return r;
}


graph::graph(int m): positions_(init_loc(m)), nodes_(m), minimizer_(nodes_) {
  if(m < 0) throw "illegal modulus";
  cout << "initializing factors" << endl;
  connect(); // Establish all interconnections among nodes.
  minimizer_.go(positions_); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
