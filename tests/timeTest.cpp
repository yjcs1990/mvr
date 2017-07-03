#include "Mvria.h"

int main(void)
{
  Mvria::init();

  MvrLog::log(MvrLog::Terse, "\nTesting MvrTime with large values:");
  
  MvrTime startLarge, testlarge;
  MvrLog::log(MvrLog::Normal, "Setting an MvrTime object \"startLarge\" to now...");
  startLarge.setToNow();
  MvrLog::log(MvrLog::Normal, "mSecSince %ld secSince %ld", startLarge.mSecSince(), startLarge.secSince());
}