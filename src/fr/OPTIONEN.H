/*
  MODUL: Optionen einstellen
 */

#include "LINEEDIT.H"
#include "PDMENU.H"
#include "SYSTEM.H"
#include "TFENSTER.H"
#include "DISCERR.h"

#include <graph.h>
#include <stdio.h>
#include <stdlib.h>

void LoadConfiguration(struct configstr*);
char Schreibe_Config(struct configstr*);
void Optionen(char);
