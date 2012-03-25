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

#include "tfenster.h"

/* INTERNE VERW.VARIABELN */

struct WInfo {
	      char x,y,br,ho;
	      unsigned char _far *adr;
	     };

char TW_anz_fenster=-1;
struct WInfo fenster[TW_MAXWINDOWS];

/* INTERNE PROTOTYPEN */

unsigned char _far *ScreenAdr(void);
unsigned char _far *SaveWindow(char,char,char,char);
void	      LoadWindow(unsigned char _far*,char,char,char,char);
void	      printchar(char);

/* FUNCTIONEN */

char TW_Open_Window (char x,char y,char br,char ho,char rahmen,
		     char titelart,char ofs,char *titel)
{
   unsigned char _far *adr;
   char ret=0,z,zw,
	rstr[11];

   /* TM_Prot(); */
   _settextwindow(1,1,25,80);
   adr=SaveWindow((char)(x-1),(char)(y-1),(char)(br+2),(char)(ho+2));
   if (adr!=NULL)
     {
       TW_anz_fenster++;
       ret=1;
       fenster[TW_anz_fenster].x=x;
       fenster[TW_anz_fenster].y=y;
       fenster[TW_anz_fenster].ho=ho;
       fenster[TW_anz_fenster].br=br;
       fenster[TW_anz_fenster].adr=adr;

       switch(rahmen)
	 {
	   case  TW_RAHMEN_S1:strcpy(rstr,"Ä³Ú¿ÙÀ´ÃÁÂ");
			      break;
	   case  TW_RAHMEN_D1:strcpy(rstr,"ÍºÉ»¼ÈµÆÐÒ");
			      break;
	   case  TW_RAHMEN_D2:strcpy(rstr,"Í³Õ¸¾Ô´ÃÁÂ");
			      break;
	   default:strcpy(rstr,"          ");
	 }

       _settextposition(y-1,x-1);
       printchar(rstr[2]);
       _settextposition(y-1,x+br);
       printchar(rstr[3]);
       _settextposition(y+ho,x+br);
       printchar(rstr[4]);
       _settextposition(y+ho,x-1);
       printchar(rstr[5]);
       for (z=x;z<x+br;z++)
	 {
	    _settextposition(y-1,z);
	    printchar(rstr[0]);
	    _settextposition(y+ho,z);
	    printchar(rstr[0]);
	 }
       for (z=y;z<y+ho;z++)
	 {
	    _settextposition(z,x-1);
	    printchar(rstr[1]);
	    for (zw=1;zw<=br;zw++)
		_outtext(" ");
	    printchar(rstr[1]);
	 }

       /* Titel setzten */
	if (strcmp(titel,""))
	{
	  switch (titelart)
	    {
	      case TW_TITEL_OBEN:
		   {
		     _settextposition(y-1,x+ofs);
		     printchar(rstr[6]);
		     _outtext(titel);
		     printchar(rstr[7]);
		   }
		   break;
	      case TW_TITEL_UNTEN:
		   {
		     _settextposition(y+ho,x+ofs);
		     printchar(rstr[6]);
		     _outtext(titel);
		     printchar(rstr[7]);
		   }
		   break;
	      case TW_TITEL_RECHTS:
		   {
		     _settextposition(y+ofs,x-1);
		     printchar(rstr[8]);
		     for (z=1;(size_t)z<strlen(titel);z++)
			{
			  _settextposition(y+ofs+z,x-1);
			  printchar(titel[z]);
			}
		     _settextposition(y+ofs+z,x-1);
		     printchar(rstr[9]);
		   }
		   break;
	      case TW_TITEL_LINKS:
		   {
		     _settextposition(y+ofs,x+br);
		     printchar(rstr[8]);
		     for (z=1;(size_t)z<strlen(titel);z++)
			{
			  _settextposition(y+ofs+z,x+br);
			  printchar(titel[z]);
			}
		     _settextposition(y+ofs+z,x+br);
		     printchar(rstr[9]);
		   }
		   break;
	    }  /*von SWITCH */
	}   /*von IF*/

       _settextwindow((short)y,
		       (short)x,
		       (short)(y+ho-1),
		       (short)(x+br-1));
     }

   /* TM_Prot(); */
   return(ret);
 }

void TW_Close_Window(void)
{
   if (TW_anz_fenster>-1)
    {
      /* TM_Prot(); */

      LoadWindow(fenster[TW_anz_fenster].adr,
		 (char)(fenster[TW_anz_fenster].x-1),
		 (char)(fenster[TW_anz_fenster].y-1),
		 (char)(fenster[TW_anz_fenster].br+2),
		 (char)(fenster[TW_anz_fenster].ho+2));
      TW_anz_fenster--;
      if (TW_anz_fenster<0)
	_settextwindow(1,1,25,80);
	else
	_settextwindow((short)(fenster[TW_anz_fenster].y),
		       (short)(fenster[TW_anz_fenster].x),
		       (short)(fenster[TW_anz_fenster].y+
				    fenster[TW_anz_fenster].ho-1),
		       (short)(fenster[TW_anz_fenster].x+
				    fenster[TW_anz_fenster].br-1));
    }
    /* TM_Prot(); */
}

/* not used */
void TW_Next_Window (void){}
void TW_Prev_Windows(void){}

unsigned char _far *ScreenAdr(void)
{
  unsigned char _far *basis=(unsigned char _far *)0xB0000000;
  union REGS outregs,inregs;

  inregs.h.ah=0x0f;
  int86(0x10,&inregs,&outregs);
  if (outregs.h.al!=7)
     basis=(unsigned char _far *)0xB8000000;

  return(basis);
}

unsigned char _far *SaveWindow(char x1,char y1,char br,char ho)
{
  unsigned char _far *adr,_far *zgr,_far *scrbasis=ScreenAdr();
  char x,y;
  size_t size=(x1+br-x1)*(y1+ho-y1)*2;
  unsigned offset;

  x1--;
  y1--;
  adr=_fmalloc(size);
  if (adr!=NULL)
    {
      zgr=adr;
      for (y=y1+ho;y>y1;y--)
	for (x=x1+br;x>x1;x--)
	    {
	      offset=(y-1)*160+(x-1)*2;
	      *zgr=*(scrbasis+offset);
	      zgr++;
	      *zgr=*(scrbasis+offset+1);
	      zgr++;
	    }
    }
  return(adr);
}

void LoadWindow(unsigned char _far *adr,char x1,char y1,char br,char ho)
{
  unsigned char _far *scrbasis=ScreenAdr();
  char x,y;
  unsigned offset;

  x1--;
  y1--;
  for (y=y1+ho;y>y1;y--)
    for (x=x1+br;x>x1;x--)
	{
	   offset=(y-1)*160+(x-1)*2;
	   *(scrbasis+offset)=*adr;
	   adr++;
	   *(scrbasis+offset+1)=*adr;
	   adr++;
	}
}

void printchar(char c)
{
  char buf[2]={" "};

  buf[0]=c;
  _outtext(buf);
}
