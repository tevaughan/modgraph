/// @file       gsl/block.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition of gsl::block.

#pragma once

#include <gsl/gsl_block.h>

namespace gsl {


class block: protected gsl_block {
  block()= delete;
  block(block const &)= delete;
  block &operator=(block const &)= delete;

public:
  size_t size() const { return gsl_block::size; }
  double *data() { return gsl_block::data; }
  double const *data() const { return gsl_block::data; }

  static block *alloc(size_t n) {
    return static_cast<block *>(gsl_block_alloc(n));
  }

  static block *calloc(size_t n) {
    return static_cast<block *>(gsl_block_calloc(n));
  }

  void free() { gsl_block_free(this); }
  int fwrite(FILE *f) const { return gsl_block_fwrite(f, this); }
  int fread(FILE *f) { return gsl_block_fread(f, this); }

  int fprintf(FILE *file, char const *fmt) const {
    return gsl_block_fprintf(file, this, fmt);
  }

  int fscanf(FILE *f) { return gsl_block_fscanf(f, this); }
};


} // namespace gsl

// EOF
