#include "mvriaUtil.h"
#include <stdio.h>
#include <assert.h>
#include <cmath>

int main(int argc, char ** argv)
{
  // Check that MvrMath::roundInt is a correct replacement for rint():
  assert(rint(0.001) == MvrMath::roundInt(0.001));
  assert(rint(0.5) == MvrMath::roundInt(0.5));
  assert(rint(0.9999) == MvrMath::roundInt(0.999));
  assert(rint(0.449) == MvrMath::roundInt(0.449));
  assert(rint(999999.9999) == MvrMath::roundInt(999999.999));
  assert(MvrMath::roundInt(INT_MAX) == INT_MAX);
  assert(MvrMath::roundInt(INT_MIN) == INT_MIN);
  assert(rint(0) == MvrMath::roundInt(0));
  assert(rint(0.000000001) == MvrMath::roundInt(0.000000001));
  assert(MvrMath::isFinite( (float)23.0 ));
  assert(MvrMath::isFinite( (double)42.0 ));
  assert(!MvrMath::isFinite(sqrt(-1.0)));
  assert(MvrMath::isNan(nan("")));
  assert(MvrMath::isNan(nanf("")));
  assert(!MvrMath::isNan( 123.2 ));
  assert(MvrMath::compareFloats(0.01, 0.02, 0.05));
  MvrLog::log(MvrLog::Normal, "All tests completed.");
  return 0; 
}
