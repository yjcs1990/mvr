#include "Mvria.h"
#include "MvrSystemStatus.h"

int main(int argc, char** argv)
{
  Mvria::init();
  MvrSystemStatus::startPeriodicUpdate();
  while(true)
  {
    printf("CPU Usage: %d%%  Uptime: %f hours  Wireless Link: %d  (Signal:%d Noise:%d)  Discarded Packets: %d  (NetConflict:%d)\n",
        (int)MvrSystemStatus::getCPUPercent(), MvrSystemStatus::getUptimeHours(),
        MvrSystemStatus::getWirelessLinkQuality(), MvrSystemStatus::getWirelessLinkSignal(), MvrSystemStatus::getWirelessLinkNoise(),
        MvrSystemStatus::getWirelessDiscardedPackets(), MvrSystemStatus::getWirelessDiscardedPacketsBecauseNetConflict()
    );
    MvrUtil::sleep(1000);
  }
}
