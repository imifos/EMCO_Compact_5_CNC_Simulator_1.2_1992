/*
This code is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 only, as
published by the Free Software Foundation.

This code is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
version 2 for more details.

Source & README published 2012
https://github.com/imifos/EMCO_Compact_5_CNC_Simulator_1.2_1992
*/



#include "seriell.h"

char SER_status=0;

bool SER_Zeichen_wartet(char com)
{
  union REGS inregs,outregs;

  inregs.h.ah=3;
  inregs.x.dx=com;
  int86(0x14,&inregs,&outregs);

  SER_status=outregs.h.ah;
  if (outregs.h.ah & 1) return TRUE; else return FALSE;
}

char SER_Lese_Zeichen(char com,char *ch)
{
  union REGS inregs,outregs;

  *ch=0;
  inregs.h.ah=2;
  inregs.x.dx=com;
  int86(0x14,&inregs,&outregs);

  SER_status=outregs.h.ah;
  if (outregs.h.ah & 128) return SER_ERR;
     else {
	    *ch=outregs.h.al;
	    return SER_OK;
	  }
}

char SER_Schreibe_Zeichen(char com,char ch)
{
  union REGS inregs,outregs;

  inregs.h.ah=1;
  inregs.h.al=ch;
  inregs.x.dx=com;
  int86(0x14,&inregs,&outregs);

  SER_status=outregs.h.ah;
  if (outregs.h.ah & 128) return SER_ERR; else return SER_OK;
}

char SER_Hole_Status(char com)
{
  union REGS inregs,outregs;

  inregs.h.ah=3;
  inregs.x.dx=com;
  int86(0x14,&inregs,&outregs);

  SER_status=outregs.h.ah;
  return outregs.h.ah;
}

char SER_Setzte_Parameter(char com,unsigned char para)
{
  union REGS inregs,outregs;

  inregs.h.ah=0;
  inregs.h.al=para;
  inregs.x.dx=com;

  int86(0x14,&inregs,&outregs);

  SER_status=outregs.h.ah;
  return outregs.h.ah;
}
