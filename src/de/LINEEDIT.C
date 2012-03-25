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



#include "lineedit.h"


/* Returnvariabeln */
char LE_Ret_MausXPos	  =0; /* Bei Klick-Ausstieg, Rueckgabe der  */
char LE_Ret_MausYPos	  =0; /* Koordinaten .			    */
char LE_Ret_letztes_Zeichen=0; /* Beinhaltet immer das letzte Zeichen*/
char LE_Ret_XPos		  =0; /* Beinh. die letzte Cursorposition   */


char LE_LineEdit(struct LE_parameter *para)
{
  char laenge=(char)(strlen(para->editstr)-1),
       xpos=para->x,
       ypos=para->y;
  char zeile[81],
       insert=para->insert,
       xcursor=para->begin_xpos;  /* beginnt ab 0 */

  char taste=0,
       raus=0,
       retcode=0,
       flag=0,
       za;


  /* CursorOn*/
  strcpy(zeile,para->editstr);

  _settextposition(ypos,xpos);
  _outtext(zeile);

  while(!raus)
   {
      /* Missbrauch des Systemcursors */
      _settextposition(ypos,xpos+xcursor);

      /* Warte auf Taste */
      if (para->erstetaste==0)
	  {
	    while (!kbhit());
	    taste=(char)getch();
	  }
	else {
	       taste=para->erstetaste;
	       para->erstetaste=0;
	     }

      if (taste==0 || taste<31 || taste>126)
	/* Controllcodes */
	{
	  if (taste==0)
	    taste=(char)getch();
	  switch(taste)
	   {
	     case 27: /* ESC */
		      if (para->esc_erlaubt)
			{
			  raus=1;
			  retcode=LE_ESC;
			}
		      break;
	     case 77: /* rechts */
		      {
			 xcursor++;
			 if (xcursor>laenge && para->raus_rechts)
			    {
			       raus=1;
			       retcode=LE_RECHTS_RAUS;
			    }
			 if (xcursor>laenge && !para->raus_rechts)
			    xcursor--;
		      }
		      break;
	     case 75: /* links */
		      {
			 xcursor--;
			 if (xcursor<0 && para->raus_links)
			    {
			       raus=1;
			       retcode=LE_LINKS_RAUS;
			    }
			 if (xcursor<0 && !para->raus_links)
			    xcursor++;
		      }
		      break;
	     case 72: /* rauf */
		      if (para->raus_oben)
			{
			  raus=1;
			  retcode=LE_OBEN_RAUS;
			}
		      break;
	     case 80: /* runter */
		      if (para->raus_unten)
			{
			  raus=1;
			  retcode=LE_UNTEN_RAUS;
			}
		      break;
	     case 79: /* Ende */
		      {
			for (za=laenge;za>0 && zeile[za]==' ';za--)
			xcursor=za;
			_settextposition(ypos,xpos+xcursor);
		      }
		      break;
	     case 71: /* Pos 1 */
		      {
			xcursor=0;
			_settextposition(ypos,xpos);
		      }
		      break;
	     case 82: /* Insert */
		      insert=(char)!insert;
		      break;

	     case 83: /* Clr */
		      {
			for (za=(char)(xcursor+1);za<=laenge;za++)
			    zeile[za-1]=zeile[za];
			zeile[laenge]=' ';
			_settextposition(ypos,xpos);
			_outtext(zeile);
		      }
		      break;
	     case 13: /* Enter */
		      {
			raus=1;
			retcode=LE_OK;
		      }
		      break;
	     case 8:  /* Del */
		      if (xcursor!=0)
		      {
			for (za=xcursor;za<=laenge;za++)
			    zeile[za-1]=zeile[za];
			zeile[laenge]=' ';
			xcursor--;
			_settextposition(ypos,xpos);
			_outtext(zeile);
		      }
		      break;
	     case 10: /* CTRL + ENTER */
		      {
			raus=1;
			retcode=LE_CTRL_RET;
		      } break;
	     case 25: /* CTRL + Y */
		      {
			raus=1;
			retcode=LE_CTRL_Y;
		      } break;
	     case (char)146: /* CTRL + ins */
		      {
			raus=1;
			retcode=LE_CTRL_INS;
		      } break;
	     case (char)118: /* CTRL + PD */
		      {
			raus=1;
			retcode=LE_CTRL_PD;
		      } break;
	     case (char)132: /* CTRL + PU */
		      {
			raus=1;
			retcode=LE_CTRL_PU;
		      } break;
	     case 59: /* F1 */
		      {
			raus=1;
			retcode=LE_FUN_1;
		      } break;
	     case (char)134:/* F12 */
		      {
			raus=1;
			retcode=LE_FUN_12;
		      } break;
	   } /* von switch taste */

	}/* 0-Code-Behandlung */
	else
	   /* Normale Zeichen */
	   {
	     for (za=0,flag=0;para->zeichenmenge[za]!='\0';za++)
		if (para->zeichenmenge[za]==taste)
		    flag=1;

	     if (flag) /* Zeichen erlaubt */
		{
		  LE_Ret_letztes_Zeichen=taste;
		  if (insert)
		    /* Einfuegen */
		    {
		      if (zeile[laenge]==' ')
		       {
			 for (za=(char)(laenge-1);za>=xcursor;za--)
			    zeile[za+1]=zeile[za];
			 zeile[xcursor]=taste;
			 _settextposition(ypos,xpos);
			 _outtext(zeile);
			 xcursor++;
			 if (xcursor>laenge && para->raus_ueberlauf)
			   {
			      raus=1;
			      retcode=LE_RECHTS_RAUS;
			   }
			 if (xcursor>laenge && !para->raus_ueberlauf)
			    xcursor--;
		       }
		    }
		    else
		    /* Ueberschreiben */
		    {
		       zeile[xcursor]=taste;
		       XLE_outchar(taste);
		       xcursor++;
		       if (xcursor>laenge && para->raus_ueberlauf)
			  {
			     raus=1;
			     retcode=LE_RECHTS_RAUS;
			  }
		       if (xcursor>laenge && !para->raus_ueberlauf)
			  xcursor--;
		    }

		}/* erlaubte Zeichen */
	    }/* von normale Zeichen */

   }

  /* cursoroff*/
  strcpy(para->editstr,zeile);
  LE_Ret_XPos	 =xcursor;
  LE_Ret_MausYPos=0;
  LE_Ret_MausYPos=0;

  return retcode;
}

/* Ausgabe eines Zeichens */
void XLE_outchar(char c)
{
  char s[2]=" ";

  s[0]=c;
  _outtext(s);
}
