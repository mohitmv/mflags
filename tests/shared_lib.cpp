#include "mflags.h"

DECLARE_MFLAG(int, q);
DEFINE_MFLAG(int, q, 10, "Input value q in shared lib");

extern "C" int F() {
  return MFLAGS_q;
}

