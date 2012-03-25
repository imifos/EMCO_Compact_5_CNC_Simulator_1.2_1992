#include <dos.h>

#ifndef bool
   typedef char bool;
   #define TRUE  1
   #define FALSE 0
#endif

#define SER_ERR    0
#define SER_OK	   1
#define DATABITS_7 2
#define DATABITS_8 3
#define STOPBIT_1  0
#define STOPBIT_2  4
#define PARIT_NO   0
#define PARIT_ODD  16	 /* ungerade */
#define PARIT_EVEN 24
#define BAUD_300   64
#define BAUD_1200  128
#define BAUD_2400  140
#define BAUD_9600  224

extern char SER_status;

bool SER_Zeichen_wartet(char);
char SER_Lese_Zeichen(char,char*);
char SER_Schreibe_Zeichen(char,char);
char SER_Hole_Status(char);
char SER_Setzte_Parameter(char,unsigned char);
