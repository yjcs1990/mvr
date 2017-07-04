#include "Mvria.h"
#include "MvrSystemStatus.h"

void printNewStatus()
{
    printf("CPU Usage: %0.1f%%  Uptime: %f hours  Wireless Link: %d  (Signal:%d Noise:%d)  Discarded Packets: %d  (NetConflict:%d)\n",
        MvrSystemStatus::getCPUPercent(), MvrSystemStatus::getUptimeHours(),
        MvrSystemStatus::getWirelessLinkQuality(), MvrSystemStatus::getWirelessLinkSignal(), MvrSystemStatus::getWirelessLinkNoise(),
        MvrSystemStatus::getWirelessDiscardedPackets(), MvrSystemStatus::getWirelessDiscardedPacketsBecauseNetConflict()
    );
}

int main(int argc, char** argv)
{
  Mvria::init();
  printf("Running with sleep time of 1sec...\n");

  for(int i = 0; i < 10; i++)
  {
    printNewStatus();
    MvrUtil::sleep(1000);
  }

  printf("\nRunning with sleep time of 5ms!...\n");
  for(int i = 0; i < 50; i++)
  {
    printNewStatus();
    MvrUtil::sleep(5);
  }

  printf("\nRunning with sleep time of 10 sec...\n");
  for(int i = 0; i < 4; i++)
  {
    printNewStatus();
    MvrUtil::sleep(10000);
  }
}
