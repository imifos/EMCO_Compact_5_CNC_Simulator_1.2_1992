/* MSC-BIBLIOTHEKEN */
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>

/* BIBLIOTHEKEN */
#include "2HRCMON.H"
#include "BALKENM2.H"
#include "KMENUE.H"
#include "LINEEDIT.H"
#include "PDMENU.H"
#include "SYSTEM.H"
#include "TFENSTER.H"
#include "DISCERR.H"
#include "GETDIR.H"
#include "SERIELL.H"
#include "2SCREEN.H"

#define controll "}sM“7ﬂëœ€…"
#define PVERSION 1.1

/* Globale Prototypen */
void Bildschirmmaske(void);

/*=== Configuration ===*/
struct configstr {
		    char intern1[10],
			 password[20],
			 monitor2,
			 lpt,
			 com,
			 intern2[10];
		   short wait_g00,
			 wait_g01;
		   short h_norm;
		 };

/*=== CNC-Programm ===*/

#define max_line  222
#define max_tools 8

struct cncprgstr {
		   char GM[5];
		   char XI[7];
		   char ZK[7];
		   char FLKT[5];
		   char H[5];
		   char BEM[41];
		 };
extern char VGA;
