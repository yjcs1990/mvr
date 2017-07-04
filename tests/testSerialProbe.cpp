#include <inttypes.h>
#include "Mvria.h"

bool tryPort(const char *port, bool is422, int baud, char **argv, int argc, int maxread, int timeout);
int main(int argc, char **argv)
{
  const int num_params_reqd = 5;
  if (argc < num_params_reqd + 1)     // plus one for command name
  {
    puts("usage: serialProbe <port|\"all\"> <baud> <\"rs232\"|\"rs422\"> <maxread> <timeout> [sendbyte1 [sendbyte2]...]");
    return 1;
  }
  
  Mvria::init();

  const char *port = argv[1];
  int baud = atoi(argv[2]);

  const char *proto = argv[3];
  bool is422 = false;
  if (strcmp(proto, "rs232") == 0)
    is422 = false;
  else if (strcmp(proto, "rs422") == 0)
    is422 = true;
  else
  {
    puts("Error: must specify rs232 or rs422 for protocol type\n");
    Mvria::exit(1);
  }  

  int maxread = atoi(argv[4]);
  int timeout = atoi(argv[5]);

  argv += num_params_reqd+1;    // start of optional list of bytes to send
  argc -= num_params_reqd+1;

  if (strcmp(port, "all") == 0)
  {
    char portname[14];
    for (int i = 0; i <= 15; ++i)
    {
      if (i >= 10)
        snprintf(portname, 14, "/dev/ttyUSB%d", i);
      else
        snprintf(portname, 13, "/dev/ttyUSB%d", i);
      printf("\n------------------ %s ------------------\n", portname);
      tryPort(portname, is422, baud, argv, argc, maxread, timeout);  
    }
  }
  else
  {
    tryPort(port, is422, baud, argv, argc, maxread, timeout);  
  }
  Mvria::exit(0);
  return 0;
}

bool tryPort(const char *port, bool is422, int baud, char **argv, int argc, int maxread, int timeout)
{
  MvrSerialConnection ser1(is422);
  ser1.setPort(port);
  ser1.setBaud(baud);
  printf("Trying %s port %s at baud rate %d:\n", is422?"RS-422":"RS-232", port, baud);

  if (!ser1.openSimple())
  {
    MvrLog::logErrorFromOS(MvrLog::Terse, "Error opening %s.", port);
    return false;    
  }

  if (argc > 0)
  {
    printf("->%3d bytes: ", argc);
    fflush(stdout);
  }

  for (int i = 0; i < argc; ++i)
  {
    int d = strtol(argv[i], NULL, 0);
    if (d == 0 && errno != 0)
    {
      MvrLog::logErrorFromOS(MvrLog::Terse, "error parsing command argument %d (\"%s\"). Must be decimal, hexidecimal, or octal number.", i, argv[i]);
      Mvria::exit(3);
    }
    unsigned char c = (unsigned char) d;
    printf("0x%.2X %c ", (unsigned char) c, (c >= ' ' && c <= '~') ? c : ' ');
    fflush(stdout);
    ser1.write((char*)&c, 1);
  }

  if (argc > 0)
    puts("");
  
  char buf[256];  
  int n = 0;
  MvrTime t;
  while (1)
  {
    MvrUtil::sleep(1);
    int r = 0;
    if ((r = ser1.read(buf, sizeof(buf))) < 0)
    {
      puts("Error reading data from serial port.");
      return false;
    }
    else if (r > 0)
    {
      printf("<- %3d bytes: ", r);
      for (int i = 0; i < r; ++i)
      {
        //printf("[%d] ", i); fflush(stdout);
        char c = buf[i];
        printf("0x%.2X %c ", (unsigned char)c, (c >= ' ' && c <= '~') ? c : '-');
        if ((i+1) % 8 == 0)
          printf("\n              ");
      }
      printf("\n");
    }
    if ((n += r) >= maxread)
    {
      puts("max");
      return true;
    }
    if (t.secSince() > timeout)
    {
      puts("timeout");
      return false;
    }
  }
  return true;
}
// int _test(int argc, char **argv)
// {
//   char myBuf[5];
//   myBuf[0] = 0xfa;
//   myBuf[1] = 0xcd;
//   myBuf[2] = 4;
//   myBuf[3] = 2;
//   myBuf[4] = 2;

//   int i;
//   unsigned char n;
//   int c = 0;

//   i = 3;
//   n = myBuf[2] - 2;
//   while (n > 1) {
//     c += ((unsigned char)myBuf[i] << 8) | (unsigned char)myBuf[i + 1];
//     c = c & 0xffff;
//     n -= 2;
//     i += 2;
//   }
//   if (n > 0)
//     c = c ^ (int)((unsigned char)myBuf[i]);
//   printf("0x%x 0x%x\n", (unsigned char)(c>>8), (unsigned char)c);
//   exit(0);
// }