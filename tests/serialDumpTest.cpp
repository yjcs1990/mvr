#include "Mvria.h"

int main(int argc, char **argv)
{
  int ret;
  char bufWrite[1024];
  char bufRead[1024];
  bool verbose = true;
  int i, n;

  const char *port = "/dev/ttyUSB0";
  if (argc > 0)
    port = argv[1];
  
  int bytes1 = 0;
  int bytes2 = 0;

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
    MvrUtil::sleep(1);

    if ((ret = serl.read(bufRead, sizeof(bufRead))) < 0)
      printf("Failed read \n");
    else if (ret > 0)
    {
      bufRead[ret] = '\0';
      if (verbose)
      {
        printf("%3d:\t", ret);
        for (i = 0; i < ret; i++)
          printf("%x(%x) ", bufRead[i], (unsigned char)bufRead[i]);
        printf("\n");
      }
      else
      	printf("%s", bufRead);
    }
    else
      bufRead[0] = '\0';
  }
}