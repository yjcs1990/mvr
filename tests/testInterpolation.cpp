#include "MvrInterpolation.h"

int main(void)
{
  MvrInterpolation interp;
  MvrTime t;
  MvrPose p;

  int i;
  int result;

  // adding fake reading with timestamp
  for (i = 1; i <= 6; i++)
  {
    t.setMSec(i * 100);
    p.setPose(i * 100, i * 100, (i - 1) * 60);
    interp.addReading(t, p);
  }

/*
   @param timeStamp the time we are interested in
   @param position the pose to set to the given position

   @param mostRecent the most recent data in the interpolation relevant to this call...
   for a return of 1 this is the near side it interpolated between, for a return of 0 or 1 this 
   is the most recent data in the interpolation.... this is only useful if the return is 1, 0, or -1, 
   and is mostly for use with MvrRobot::applyEncoderOffset 

   @return 1 its good interpolation, 0 its predicting, -1 its too far to 
   predict, -2 its too old, -3 there's not enough data to predict   
*/
  t.setMSec(100);
  result = interp.getPose(t, &p);
  printf("Result of 100 %d\n\n", result);
  if (result == 1)
    p.log();

  t.setMSec(200);
  result = interp.getPose(t, &p);
  printf("Result of 200 %d\n\n", result);
  if (result == 1)
    p.log();

  t.setMSec(300);
  result = interp.getPose(t, &p);
  printf("Result of 300 %d\n\n", result);
  if (result == 1)
    p.log();

  t.setMSec(400);
  result = interp.getPose(t, &p);
  printf("Result of 400 %d\n\n", result);
  if (result == 1)
    p.log();

  t.setMSec(500);
  result = interp.getPose(t, &p);
  printf("Result of 500 %d\n\n", result);
  if (result == 1)
    p.log();

  t.setMSec(600);
  result = interp.getPose(t, &p);
  printf("Result of 600 %d\n\n", result);
  if (result == 1)
    p.log();

  t.setMSec(366);
  result = interp.getPose(t, &p);
  printf("\nResult of 366 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(455);
  result = interp.getPose(t, &p);
  printf("\nResult of 455 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(580);
  result = interp.getPose(t, &p);
  printf("\nResult of 580 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(750);
  result = interp.getPose(t, &p);
  printf("\nResult of 750 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(599);
  result = interp.getPose(t, &p);
  printf("\nResult of 599 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(600);
  result = interp.getPose(t, &p);
  printf("\nResult of 600 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(601);
  result = interp.getPose(t, &p);
  printf("\nResult of 601 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(50);
  result = interp.getPose(t, &p);
  printf("\nResult of 50 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(99);
  result = interp.getPose(t, &p);
  printf("\nResult of 99 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(100);
  result = interp.getPose(t, &p);
  printf("\nResult of 100 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(101);
  result = interp.getPose(t, &p);
  printf("\nResult of 101 %d\n", result);
  if (result == 1)
    p.log();
}