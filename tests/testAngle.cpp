#include "Mvria.h"

/* See also angleBetweenTest, angleFixTest, mathTests. */

bool findDifference(MvrPose origin, MvrPose sonar, double th,  double dist)
{
  double cth, cdist;
  cth = MvrMath::subAngle(origin.findAngleTo(sonar), origin.getTh());
  cdist = origin.findDistanceTo(sonar);
  if (MvrMath::fabs(th - cth) < .1 && MvrMath::fabs(cdist - dist) < .1)
  {
    return true;
  }
  else
  {
    printf("bad  wanted %.2f %.0f got ", th, dist);
    printf("%.2f %.0f origin %.0f %.0f %.0f sonar %.0f %.0f %.0f angleTo %.0f\n",
	   cth, cdist, origin.getX(), origin.getY(), 
	   origin.getTh(), sonar.getX(), sonar.getY(), sonar.getTh(),
	   origin.findAngleTo(sonar));
 
	    
    printf("Failed tests!\n");
    return false;
  }
  
}

void testSet(double x1, double y1, double x2, double y2, double initial, double distance)
{
  double add;

  MvrPose origin;
  MvrPose sonar(x2, y2);

  for (add = -135; add <= 180; add += 45)
  {
    origin.setPose(x1, y1, add);
    if (!findDifference(origin, sonar, MvrMath::subAngle(initial, add),
			distance))
    {
      printf("Failed that one from (%.0f, %.0f) (%.0f, %.0f) %3.0f %4.0f\n", 
	     x1, y1, x2, y2, initial, distance);
      exit(0);
    }
  }
  
}

int main(void)
{
  MvrPose origin;
  MvrPose sonar;
  double halfDiag = sqrt(500 * 500 + 500 * 500);
  double diag = sqrt(1000 * 1000 + 1000 * 1000);

  origin.setPose(0, 0, 0);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 90, 1000);

  origin.setPose(0, 0, 45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 45, 1000);

  origin.setPose(0, 0, 90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 0, 1000);

  origin.setPose(0, 0, 135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -45, 1000);

  origin.setPose(0, 0, 180);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -90, 1000);

  origin.setPose(0, 0, -135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -135, 1000);

  origin.setPose(0, 0, -90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 180, 1000);

  origin.setPose(0, 0, -45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 135, 1000);
  
  //printf("\n");

  origin.setPose(500, 500, 0);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 135, halfDiag);

  origin.setPose(500, 500, 45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 90, halfDiag);

  origin.setPose(500, 500, 90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 45, halfDiag);

  origin.setPose(500, 500, 135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 0, halfDiag);

  origin.setPose(500, 500, 180);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -45, halfDiag);
  
  origin.setPose(500, 500, -135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -90, halfDiag);

  origin.setPose(500, 500, -90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -135, halfDiag);

  origin.setPose(500, 500, -45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 180, halfDiag);


  testSet(0, 0, 0, 0, 0, 0);
  testSet(0, 0, 0, 1000, 90, 1000);
  testSet(0, 0, 1000, 1000, 45, diag);
  testSet(0, 0, 1000, 0, 0, 1000);
  testSet(0, 0, 500, 500, 45, halfDiag);

  testSet(0, 1000, 0, 0, -90, 1000);
  testSet(0, 1000, 0, 1000, 0, 0);
  testSet(0, 1000, 1000, 1000, 0, 1000);
  testSet(0, 1000, 1000, 0, -45, diag);
  testSet(0, 1000, 500, 500, -45, halfDiag);

  testSet(1000, 1000, 0, 0, -135, diag);
  testSet(1000, 1000, 0, 1000, -180, 1000);
  testSet(1000, 1000, 1000, 1000, 0, 0);
  testSet(1000, 1000, 1000, 0, -90, 1000);
  testSet(1000, 1000, 500, 500, -135, halfDiag);

  testSet(1000, 0, 0, 0, 180, 1000);
  testSet(1000, 0, 0, 1000, 135, diag);
  testSet(1000, 0, 1000, 1000, 90, 1000);
  testSet(1000, 0, 1000, 0, 0, 0);
  testSet(1000, 0, 500, 500, 135, halfDiag);

  testSet(500, 500, 0, 0, -135, halfDiag);
  testSet(500, 500, 0, 1000, 135, halfDiag);
  testSet(500, 500, 1000, 1000, 45, halfDiag);
  testSet(500, 500, 1000, 0, -45, halfDiag);
  testSet(500, 500, 500, 500, 0, 0);

  printf("Passed all tests!\n");
  return 0;
}
