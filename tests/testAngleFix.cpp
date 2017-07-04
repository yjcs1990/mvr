#include "Mvria.h"

void fixTest(double th, double sth)
{
  double cth;
  if ((cth = MvrMath::fixAngle(th)) == sth)
    printf("Good fix of %f result in %f\n", th, cth);
  else
  {
    printf("Bad fix of %f result in %f instead of %f\n", th, cth, sth);
    printf("Failed tests\n");
    exit(1);
  }
}

int main(void)
{
  printf("Some test of fixAngle\n");

  fixTest(405, 45);
  fixTest(-405, -45);
  fixTest(450, 90);
  fixTest(-450, -90);
  fixTest(495, 135);
  fixTest(-495, -135);
  fixTest(540, 180);
  fixTest(-540, 180);
  fixTest(585, -135);
  fixTest(-585, 135);
  fixTest(630, -90);
  fixTest(-630, 90);
  fixTest(675, -45);
  fixTest(-675, 45);
  fixTest(720, 0);
  fixTest(-720, 0);
  fixTest(765, 45);
  fixTest(-765, -45);

  printf("\n");
  fixTest(225, -135);
  fixTest(-225, 135);
  fixTest(315, -45);
  fixTest(-315, 45);
  fixTest(270, -90);
  fixTest(-270, 90);
  fixTest(-180, 180);
  fixTest(180, 180);
  
  printf("\nPassed all tests!!\n");
}