#include "mflags.h"

DECLARE_MFLAG(int, q);

extern "C" int F() {
  return MFLAGS_q;
}

