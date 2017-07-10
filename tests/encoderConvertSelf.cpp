#include "Mvria.h"
const double Ticks = 985.6;
const double radius = 33.25;

int main(int argc, char **argv)
{
  int ret;
  // char bufWrite[1024];
  char bufRead[1024];
  int result;
  long  newLeftTicks = 0;
  long  newRightTicks = 0;
  long  oldLeftTicks = 0;
  long  oldRightTicks = 0;
  double leftDis = 0;
  double rightDis = 0;
  double centerDis = 0;
  MvrPose newPose(0,0,0);
  MvrPose oldPose(0,0,0);
  float convTh = 0.0;


  bool myFirstRead = true;

  MvrInterpolation interp;
  MvrPose p;


  char port[13] = "/dev/ttyUSB0";

  MvrTime lastPrint;

  MvrSerialConnection serl;
  serl.setPort(port);
  serl.setBaud(115200);
  printf("opening %s ...\n", port);
  if (!serl.openSimple())
  {
    printf("open failed\n");
    exit(0);
  }
  printf("Port opened\n");
  lastPrint.setToNow();
  while(1)
  {
    MvrUtil::sleep(200);

    if ((ret = serl.read(bufRead, sizeof(bufRead))) < 0)
      printf("Failed read \n");
    else if (ret > 0)
    {
      bufRead[ret] = '\0';
      // if (bufRead[0] == 0xa5 && bufRead[3] == 0x04)
      if (bufRead[3] == 0x04)
      {
        printf("%x\n", bufRead[0]);
        newLeftTicks =((bufRead[5] & 0xff) << 24)| ((bufRead[6] & 0xff) << 16)| ((bufRead[7] & 0xff) << 8) | (bufRead[8] & 0xff);
        newRightTicks =((bufRead[9] & 0xff) << 24)| ((bufRead[10] & 0xff) << 16) | ((bufRead[11] & 0xff) << 8)| (bufRead[12] & 0xff);
        if (myFirstRead)
        {
          oldLeftTicks = newLeftTicks;
          oldRightTicks = newRightTicks;
          leftDis = 0;
          rightDis = 0;
          centerDis = 0;

          myFirstRead = false;
        }
        else
        {
          leftDis = 2 * M_PI * (newLeftTicks - oldLeftTicks) * radius / Ticks;
          rightDis = 2 * M_PI * (newRightTicks - oldRightTicks) * radius / Ticks;
          centerDis = (leftDis + rightDis) / 2;
          convTh = (rightDis - leftDis) / 280 * 180 / M_PI;
          newPose.setX(oldPose.getX() + centerDis * MvrMath::cos(oldPose.getTh()));
          newPose.setY(oldPose.getY() + centerDis * MvrMath::sin(oldPose.getTh()));
          newPose.setTh(oldPose.getTh() + convTh);

          oldLeftTicks = newLeftTicks;
          oldRightTicks = newRightTicks;
          oldPose = newPose;

          printf("%f %f %f\n", newPose.getX(), newPose.getY(), newPose.getTh());
          printf("### %ld %ld\n", newLeftTicks, newRightTicks);
        }
        lastPrint.addMSec(200);

        interp.addReading(lastPrint, newPose);

        // test
          result = interp.getPose(lastPrint, &p);
          printf("Result of %d\n\n", result);
          if (result == 1)
            p.log();
      }
      else
        continue;
    }
    else
      bufRead[0] = '\0';
  }
}