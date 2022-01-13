
/// @file       asy-adapter.hpp
/// @brief      Definition of modgraph::asy_adapter and descendants.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include <eigen3/Eigen/Core> // Vector3d
#include <sstream> // ostringstream

namespace modgraph {


using Eigen::Vector3d;
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


} // namespace modgraph

// EOF
