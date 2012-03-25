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


#include "pdmenu.h"

/* Interne Prototypen */

void Ausg_Punkt(char nr,char maxbr,char *basisadr,char x,char y);
void Ausg_Punkt_Inv(char nr,char maxbr,char *basisadr,char x,char y);

/* Functionen */
char PD_Menue(struct PD_parameter *para)
{
   register char za;
   char ypos[15],wahl=para->vorwahl,button,xmouse,ymouse;
   int taste,wx1,wx2,wy1,wy2;

   /* Fenstergrenzen holen */
   _gettextwindow(&wy1,&wx1,&wy2,&wx2);
   wx1--;
   wy1--;

   /* Menuepos. berechnen */
   for (za=0;za<para->anz;za++)
       if (za==0) ypos[za]=para->yap;
	 else ypos[za]=ypos[za-1]+*(para->relpos+za);

   /* Menue schreiben,wenn erlaubt */
   if (para->neuschreiben)
      for (za=0;za<para->anz;za++)
	  Ausg_Punkt(za,para->strbr,para->array,para->xap,ypos[za]);

   /* 1.x ausgeben */
   Ausg_Punkt_Inv(wahl,para->strbr,para->array,para->xap,ypos[wahl]);

   do {
	taste=0;
	button=0;

	while (!kbhit() && !button) /*button=TM_Button()*/;

	if (!button)
	   {
	     taste=getch();
	     if (taste==0) taste=getch();
	   }
	  else taste=10;

	switch (taste)
	 {
	    case 27:if (!para->escerlaubt) taste=27;
		    break;
	    case 77:if (para->rechts_raus)
		     {
		       taste=13;
		       wahl=PD_RECHTS_RAUS;
		     }
		     break;
	    case 75:if (para->links_raus)
		     {
		       taste=13;
		       wahl=PD_LINKS_RAUS;
		     }
		    break;
	    case 72:if (wahl>0)
		       {
			 Ausg_Punkt(wahl,para->strbr,para->array,
				    para->xap,ypos[wahl]);
			 wahl--;
			 Ausg_Punkt_Inv(wahl,para->strbr,para->array,
					para->xap,ypos[wahl]);
		       }
		    break;
	    case 80:if (wahl<(char)(para->anz-1))
		       {
			 Ausg_Punkt(wahl,para->strbr,para->array,
				     para->xap,ypos[wahl]);
			 wahl++;
			 Ausg_Punkt_Inv(wahl,para->strbr,para->array,
					para->xap,ypos[wahl]);
		       }
		    break;
	    case 10:{
		      do{
			  /* xmouse=TM_XPos()-(char)wx1; */
			  /* ymouse=TM_YPos()-(char)wy1; */
			  for (za=0;za<para->anz;za++)
			     if (ymouse==ypos[za] && xmouse>para->xap &&
				 xmouse<para->xap+para->strbr && za!=wahl)
				 {
				   Ausg_Punkt(wahl,para->strbr,para->array,
					      para->xap,ypos[wahl]);
				   wahl=za;
				   za=para->anz;
				   Ausg_Punkt_Inv(wahl,para->strbr,para->array,
						  para->xap,ypos[wahl]);
				 }
			  /* button=TM_Button(); */
			} while (button);
			if (ymouse==ypos[wahl] && xmouse>para->xap &&
				 xmouse<para->xap+para->strbr) taste=13;
			if ((xmouse<para->xap || xmouse>para->xap+para->strbr) &&
			      para->escerlaubt) taste=27;
		     }
	 }

      } while (taste!=13 && taste!=27);

   if (taste==27) wahl=PD_RET_ESC;
   return wahl;
}

void Ausg_Punkt(char nr,char maxbr,char *basisadr,char x,char y)
{
   char memstr[40];

   _settextposition(y,x);
   strcpy(memstr,basisadr+nr*maxbr);
   /* TM_Prot(); */
   _outtext(memstr);
   /* TM_Prot(); */
}

void Ausg_Punkt_Inv(char nr,char maxbr,char *basisadr,char x,char y)
{
   char memstr[40];
   short oldtxtcol=_gettextcolor();
   long  oldbkcol =_getbkcolor();

   _settextposition(y,x);
   strcpy(memstr,basisadr+nr*maxbr);
   _settextcolor((short)oldbkcol); /*0*/
   _setbkcolor((long)oldtxtcol);	/*15*/
   /* TM_Prot(); */
   _outtext(memstr);
   /* TM_Prot(); */
   _settextcolor(oldtxtcol);
   _setbkcolor(oldbkcol);
}
