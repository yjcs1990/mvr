#include "Mvria.h"

int main(int argc, char **argv)
{
  int ret;
  bool verbose = true;
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
  bufWrite[0] = 0xa5;
  bufWrite[1] = 0x00;
  bufWrite[2] = 0x09;
  bufWrite[3] = 0x00;
  bufWrite[4] = 0x72;
  bufWrite[5] = 0x03;
  bufWrite[6] = 0xe8;
  bufWrite[7] = 0x01;
  bufWrite[8] = 0x00;
  bufWrite[9] = 0xA0;
  bufWrite[10] = 0x31;
  bufWrite[11] = 0x5a;
  serl.write(bufWrite, 12);
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
          printf("%x ", bufRead[i]);
        printf("\n");
      }
      else
      	printf("%s", bufRead);
    }
    else
      bufRead[0] = '\0';
  }
}
//   while (1)
//   {
//     // if (ser1.write(bufWrite, rand() % 1024) < 0)
// 	  //   printf("Failed write\n");
//     // n = rand() % 1024;
//     if ((ret = ser1.read(bufRead, n)) < 0)
//       printf("Failed read\n");
//     else if (ret > 0) 
//     {
//       for (i = 0; i < ret; i++)
//       {
//         if (bufRead[i] != 0x66)
//         {
//           printf("Failed\n");
//           break;
//         }
//       }
//       bytes1 += ret;
//     } 
//     if (lastPrint.mSecSince() > 1000)
//     {
//     printf("%d\n", bytes1);
//     lastPrint.setToNow();
//     }
//   }
// }
