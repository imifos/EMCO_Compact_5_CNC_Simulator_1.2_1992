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


#include "discerr.h"

/* Interne definitionen */
#define ASM_INIT  0
#define ASM_HOLEN 1
void DiscErr_ASMRout(char,unsigned*,unsigned*);
void GeneriereErrorStr(unsigned,unsigned,char*,char*,char*);
void AusgabeZentrieren(char*,char);

/* Functionen */

char DE_FileExit(char *name)
{
   FILE *datei;
   short ax,di;
   char de_ret,ret=DE_EXIST,pd_ret;
   char buf[_MAX_PATH],fname[20],fext[5];

   /* Pulldownmenue def */
   struct PD_parameter pd={9,3,1,1,1,0,0,2,16,NULL,NULL};
   char pdarray[2][16]={" šberschreiben "," Neue  Eingabe "},
	pdrp   [2]    ={1,1};
   pd.array=pdarray[0];
   pd.relpos=pdrp;

   do {
	datei=fopen(name,"r");
	fclose(datei);
	DE_HoleReg(&ax,&di);

	/* Quit mit 0 wenn keine Datei gefunden & kein Diskerror */
	if (datei==NULL && ax==0) return DE_NOEXIST;

	/* Datei vorhanden oder Diskerror */
	_splitpath(name,buf,buf,fname,fext);
	strcat(fname,fext);
	de_ret=DE_DiskError(datei,fname);
	if (de_ret==DE_ERROR) return DE_ESC;

      }while(de_ret!=DE_NOERROR); /* Raus wenn no error -> Datei vorhanden */

   SYS_PushColor();
   _settextcolor(7);
   _setbkcolor(1);
   TW_Open_Window(4,5,33,4,TW_RAHMEN_S1,TW_TITEL_OBEN,3,"!WARNUNG!");
   _outtext(" *** Datei existiert bereits ***");
   pd_ret=PD_Menue(&pd);
   switch(pd_ret)
   {
     case PD_RET_ESC:ret=DE_ESC;     break;
     case 0	    :ret=DE_NOEXIST; break;
     case 1	    :ret=DE_EXIST;
   }
   TW_Close_Window();
   SYS_PopColor();
   return ret;
}

void DE_ResetErrorMemory(void)
{
   short axdi;
   DiscErr_ASMRout(ASM_HOLEN,&axdi,&axdi);
}

/* ax=0 -> Kein Fehler */
/* Darft nie zusammen mit DE_DiskError benutzt werden */
void DE_HoleReg(int *ax,int *di)
{
   DiscErr_ASMRout(ASM_HOLEN,ax,di);
}

char DE_DiskError(FILE *dateizgr,char *dateiname)
{
    unsigned di,ax;			     /* Returnregister */
    char errstr1[80],errstr2[80],errstr3[80];/* Errormeldungen */
    short oldpen=_gettextcolor();	     /* Alte Farben */
    long  oldpaper=_getbkcolor();
    /* PD-Menue-Var */
    char relpos[2]={1,1};
    char array[2][14]={"  Abbrechen  "," Wiederholen "};
    struct PD_parameter RetCan={27,7,0,1,1,0,0,2, 14,NULL,NULL};
    char wahl,ret;

    /* Init */
    RetCan.array=array[0];
    RetCan.relpos=relpos;

    /* Hole eventuellen Diskerror */
    DiscErr_ASMRout(ASM_HOLEN,&ax,&di);
    if (ax==0 && dateizgr!=NULL)
	ret=DE_NOERROR;
     else
      { /* Diskettenerror */

	/* Fehlerausgabe herstellen */
	if (ax==0) /* Prioritaet auf AX/DI-Fehler */
	    { /* Fehler beim Oeffnen */
	      strcpy(errstr1,"Datei: ");
	      strcat(errstr1,dateiname);
	      strcpy(errstr2,"Unm”glich die Datei zu ™ffnen!");
	      strcpy(errstr3,"* Bitte berprfen Sie Dateinamen & Pfad! *");
	    }
	   else {
		  GeneriereErrorStr(ax,di,errstr1,errstr2,errstr3);
		  strcat(errstr1,dateiname);
		}

	/* Neue Farben setzten */
	_settextcolor(15); /*auf mono testn*/
	_setbkcolor(1);

	/* Window oeffnen */
	TW_Open_Window(7,6,66,8,TW_RAHMEN_D1,TW_TITEL_OBEN,3,
					     "Fehler beim Diskettenzugriff");
	_settextcolor(14);
	AusgabeZentrieren(errstr1,2);
	_settextcolor(12);
	AusgabeZentrieren(errstr2,3);
	_settextcolor(14);
	_outtext("\n   ÄÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄ");
	AusgabeZentrieren(errstr3,5);
	_outtext("\n   ÄÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄÄÍÍÍÍÍÄ");
	_settextcolor(12);
	/* PullDown-Menue */
	_settextcolor(7);
	wahl=PD_Menue(&RetCan);

	switch(wahl)
	{
	  case PD_RET_ESC:;
	  case 0	 :ret=DE_ERROR;
			  break;
	  case 1	 :ret=DE_REPEAT;
	}
	TW_Close_Window();

	/* Farben zuruecksetzten */
	_settextcolor(oldpen);
	_setbkcolor(oldpaper);
      }
    return(ret);
}

void GeneriereErrorStr(unsigned ax,unsigned di,char *es1,char *es2,char *es3)
{
   char drive[4]={"A: "},*ax_ptr=(char*)&ax,drive_za;

   /* Fehlerlaufwerk bestimmen */
   drive[0]=(char)(ax_ptr[0]+65);
   strcpy(es1,"Laufwerk ");
   strcat(es1,drive);
   strcat(es1,"  /   Datei:");

   /* Fehlerart & Hilfe aus DI herstellen */
   switch(di)
   {
      case 0x00:{
		  strcpy(es2,"Diskette ist schreibgeschtzt!");
		  strcpy(es3,"Bitte Schreibschutz entfernen.");
		}
		break;
      case 0x01:{
		  strcpy(es2,"Unbekannte Laufwerksbezeichnung!");
		  strcpy(es3,"Erlaubte Laufwerksbuchstaben: ");
		  for (drive_za=1;drive_za<=26;drive_za++)
		    if(!_chdrive(drive_za))
		      {
			drive[0]=(char)(drive_za+'A'-1);
			strcat(es3,drive);
		      }
		}
		break;
      case 0x02:{
		  strcpy(es2,"Laufwerk nicht bereit!");
		  strcpy(es3,"Bitte berprfen Sie ob eine Disk. eingelegt ist.");
		}
		break;
      case 0x03:{
		  strcpy(es2,"Invalid Command!");
		  strcpy(es3,"!! Interner Fehler !!");
		}
		break;
      case 0x04:{
		  strcpy(es2,"CRC Error!");
		  strcpy(es3,"!! Interner Fehler !!");
		}
		break;
      case 0x05:{
		  strcpy(es2,"Bad request structure length");
		  strcpy(es3,"!! Interner Fehler !!");
		}
		break;
      case 0x06:{
	       strcpy(es2,"Zugriffsfehler");
	       strcpy(es3,"Diskette oder Datei fehlerhaft");
	     }
	     break;
      case 0x07:{
	       strcpy(es2,"Diskettenformat nicht erkannt!");
	       strcpy(es3,"Bitte benutzen Sie eine andere Diskette.");
	     }
	     break;
      case 0x08:{
		  strcpy(es2,"Sector nicht gefunden!");
		  strcpy(es3,"Zugriffsfehler durch fehlerhaften Sektor.");
		}
		break;
      case 0x09:{
		  strcpy(es2,"Printer out of paper!");
		  strcpy(es3,"(Drucker-Fehler)");
		}
		break;
      case 0x0A:{
		  strcpy(es2,"Schreibfehler!");
		  strcpy(es3,"Fehlerhafter Cluster/Sektor gefunden.");
		}
		break;
      case 0x0B:{
		  strcpy(es2,"Lesefehler!");
		  strcpy(es3,"Fehlerhafter Cluster/Sektor gefunden.");
		}
		break;
      case 0x0C:{
		  strcpy(es2,"Genereller Diskettenfehler!");
		  strcpy(es3,"( Haben Sie eine formatierte Diskette benutzt? )");
		}
		break;
      default  :{
		  strcpy(es2,"!! Unbekannter Diskettenfehler !!");
		  strcpy(es3,"");
		}
		break;
   }
}

void AusgabeZentrieren(char *str,char yp)
{
  _settextposition(yp,33-(char)(strlen(str)/2));
  _outtext(str);
}

void DE_Init(void)
{
    /* Interupt 24h umbiegen */
    DiscErr_ASMRout(ASM_INIT,NULL,NULL);
}


/* Function beinhaltet alle ASM-Routinen die zur Verwaltung des
   "Critical-Dos-Error-Interrupt" 24h noetig sind:
   - Int24h umbiegen	     (Switch sw=0)
   - Neue Int24h-Routine
   - Fehlervariabeln holen   (Switch sw=1,Adressen von AX- & DI-Variabl.)
 */
void DiscErr_ASMRout(char sw,unsigned *ax,unsigned *di)
{
   unsigned acc=0,dest=0;

   if (sw==0)
	{
	   _asm{
		 ;
		 ;INT 24h umbiegen
		 ;----------------
		 ;
		 push ds		    ;DSeg=CSeg
		 push cs
		 pop ds

		 ;alter INT24h retten
		 mov ah,0x35		  ;Funct 35h
		 mov al,0x24		  ;Int24h
		 int 0x21
		 mov [oldadr],bx
		 mov [oldadr+2],es

		 ;INT24h auf neue Routine umbiegen
		 mov dx,offset newadr
		 mov ah,0x25		  ;Funct 25h
		 mov al,0x24		  ;Int24h
		 int 0x21

		 mov bx,offset speicher
		 mov [bx+0],0		   ;Speicher reset'en
		 mov [bx+1],0
		 mov [bx+2],0
		 mov [bx+3],0

		 pop ds 		  ;altes DSeg
		 jmp raus		  ;Ende

		 ;Speicher fuer die alte Adresse
		 oldadr:
		 nop
		 nop
		 nop
		 nop

		 ;Speicher fuer die Fehlerregister AX und DI
		 ;AL=Drive
		 ;AX=Fehlerart
		 ;DI=Fehlerort
		 speicher:
		 nop
		 nop
		 nop
		 nop

		 ;
		 ;INT 24h: neue Routine
		 ;---------------------
		 ;Dieser Interrupt wird von DOS im Falle eines
		 ;kritischen Fehlers aufgerufen. Dies ist nicht
		 ;nur bei Diskettenfehlern der Fall. Es muss
		 ;Vorsorge getroffen werden: (Bit 7 AL=1 -> NoDisk)
		 newadr:

		 ;Test ob Discerror oder nicht
		 pushf
		/* test al,128
		 jz discerr
		 popf
		 ;richtige Routine anspringen


		 iret
		 ;Discerror erkannt
		 discerr: */
		 popf
		 push ds
		 push bx

		 push cs		  ;Register AX und DI im Code-
		 pop ds 		  ;Segm. sichern.
		 mov bx,offset speicher
		 mov [bx],ax
		 mov [bx+2],di

		 pop bx
		 pop ds

		 mov al,0		  ;von DOS erwarteter Ret-Wert:
		 iret			  ;AL=0=Ignore (1=Retry/2=Break)
		 raus:
	       }
	}
	else
	      {
		 _asm{
		       ;
		       ;Werte der im CodeSegm gespeicherten AX und DI-
		       ;Register in Variabeln uebertragen.
		       ;
		       push ds

		       push cs			   ;DS=CS
		       pop ds

		       mov bx,offset speicher
		       mov ax,[bx]		   ;AX
		       mov [bx+0],0		   ;Speicher reset'en
		       mov [bx+1],0
		       mov cx,[bx+2]		   ;DI
		       mov [bx+2],0
		       mov [bx+3],0

		       pop ds			   ;Altes DataSegm
		       mov acc,ax		   ;und uebergeben
		       mov dest,cx
		     }
		  *ax=acc;			   /* Returnwerte */
		  *di=dest;
	      }
}
