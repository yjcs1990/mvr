#include "Mvria.h"

bool encoderPrinter(char *packet, int len)
{
  long int left;
  long int right;

  printf("%3d:\t", len);
  for (i = 0; i < len; i++)
    // printf("%x(%x) ", bufRead[i], (unsigned char)bufRead[i]);
    printf("%x ", (unsigned char)bufRead[i]);
  printf("\n");

  printf("### %ld %ld\n", left, right);
  return true;
}

int main(int argc, char **argv)
{
  int ret;
  bool verbose = false;
  char bufWrite[1024];
  char bufRead[1024];
  int i, n;



  srand(time(NULL));
  
  int bytes1 = 0;
  //int bytes2 = 0;
  //int numToWrite = 1;

  MvrTime lastPrint;
  
  if (argc < 2)
    {
      printf("Usage: %s <port>\n", argv[0]);
      exit(0);
    }


    MvrSerialConnection serl;
    serl.setPort(argv[1]);
    serl.setBaud(115200);
    if (!serl.openSimple())
    {
      printf("Exiting since open failed\n");
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
          // printf("%x(%x) ", bufRead[i], (unsigned char)bufRead[i]);
          printf("%x ", (unsigned char)bufRead[i]);
        printf("\n");
      }
      else
      	printf("%s", bufRead);
    }
    else
      bufRead[0] = '\0';
  }
}