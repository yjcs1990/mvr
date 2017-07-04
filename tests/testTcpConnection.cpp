#include "Mvria.h"

/* Test MvrTcpConnection */

const char* host = "127.0.0.1";

int main(int argc, char **argv)
{
  MvrRobot robot;
  Mvria::init();
  MvrTcpConnection tcpCon;

  printf("Opening TCP connection to %s...\n", host);
  int status = tcpCon.open(host);
  if(status != 0)
  {
    printf("Failed to connect via TCP to %s: %s.\n", host, tcpCon.getOpenMessage(status));
    exit(1);
  }
  
  printf("Connecting to robot...\n");
  robot.setDeviceConnection(&tcpCon);
  if(!robot.blockingConnect())
  {
    printf("Failed to connect to robot.\n");
    exit(2);
  }

  printf("Connected to robot.\n");

  // Start the robot running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  // Sleep for 3 seconds.
  printf("Sleeping for 3 seconds...\n");
  MvrUtil::sleep(3000);
  
  printf("Ending robot thread...\n");
  robot.stopRunning();

  printf("Exiting.\n");
  return 0;
}
