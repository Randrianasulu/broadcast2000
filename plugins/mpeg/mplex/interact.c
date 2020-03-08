#include "main.h"
/*************************************************************************
    Startbildschirm und Anzahl der Argumente

    Intro Screen and argument check
*************************************************************************/

void intro(argc)
int argc;
{
    printf("\n***************************************************************\n");
    printf(  "*               MPEG1/SYSTEMS      Multiplexer                *\n");
    printf(  "*               (C)  Christoph Moar, 1994/1995                *\n");
    printf(  "*               moar@informatik.tu-muenchen.de                *\n");
    printf(  "*               Technical University of Munich                *\n");
    printf(  "*               SIEMENS ZFE  ST SN 11 / T SN 6                *\n");
    printf(  "*                                                             *\n");
    printf(  "*  This program is free software. See the GNU General Public  *\n");
    printf(  "*  License in the file COPYING for more details.              *\n");
    printf(  "*  Release %s (%s)                                  *\n",MPLEX_VER,MPLEX_DATE);
    printf(  "***************************************************************\n\n");

    if(argc < 3)
    {	
		printf("Usage: mplex <input stream1> [<input stream2> ...] <output system stream>\n\n");
		exit (1);
    }
}


/*************************************************************************
    File vorhanden?

    File found?
*************************************************************************/

int open_file(name, bytes)			
char *name;
unsigned int *bytes;				
{
    FILE* datei;

    datei=fopen (name, "rw");
    if (datei==NULL)
    {	
	printf("File %s not found.\n", name);
	return (TRUE);
    }
    fseek (datei, 0, 2);
    *bytes = ftell(datei);
    fclose(datei);
    return (FALSE);
}


/*************************************************************************
	ask_continue
	Nach Anzeige der Streaminformationen Abfrage, ob weiter
	gearbeitet werden soll.

	After displaying Stream informations there is a check, wether
	we should continue computing or not.
*************************************************************************/

void ask_continue ()
{
    char input[20];

return;

    printf ("\nContinue processing (y/n) : ");
    do fgets (input, 20, stdin);
    while (input[0]!='N'&&input[0]!='n'&&input[0]!='y'&&input[0]!='Y');

    if (input[0]=='N' || input[0]=='n')
    {
	printf ("\nStop processing.\n\n");
	exit (0);

    }

}

/*************************************************************************
	ask_verbose
	Soll die letzte, MPEG/SYSTEM Tabelle vollstaendig ausgegeben
	werden?

	Should we print the MPEG/SYSTEM table very verbose or not?
*************************************************************************/

unsigned char ask_verbose ()
{
    char input[20];

return (FALSE);
    printf ("\nVery verbose mode (y/n) : ");
    do fgets (input, 20, stdin);
    while (input[0]!='N'&&input[0]!='n'&&input[0]!='y'&&input[0]!='Y');

    if (input[0]=='N' || input[0]=='n') return (FALSE); else return (TRUE);
}

/******************************************************************
	Status_Info
	druckt eine Statuszeile waehrend des Multiplexens aus.

	prints a status line during multiplexing
******************************************************************/

void status_info (nsectors_a, nsectors_v, nsectors_p, nbytes, 
		  buf_v, buf_a,verbose)
unsigned int nsectors_a;
unsigned int nsectors_v;
unsigned int nsectors_p;
unsigned int nbytes;
unsigned int buf_v;
unsigned int buf_a;
unsigned char verbose;
{
	if(nbytes )
	{
    	printf ("| %7d | %7d |",nsectors_a,nsectors_v);
    	printf (" %7d | %11d |",nsectors_p,nbytes);
    	printf (" %6d | %6d |",buf_a,buf_v);
    	printf ((verbose?"\n":"\r"));
    	fflush (stdout);
	}
}

void status_header ()
{
return;
    status_footer();
    printf("|  Audio  |  Video  | Padding | Bytes  MPEG | Audio  | Video  |\n");
    printf("| Sectors | Sectors | Sectors | System File | Buffer | Buffer |\n");
    status_footer();
}


void status_message (what)
unsigned char what;
{
return;
  switch (what)
  {
  case STATUS_AUDIO_END:
  printf("\n|file  end|         |         |             |        |        |\n");
  break;
  case STATUS_AUDIO_TIME_OUT:
  printf("\n|time  out|         |         |             |        |        |\n");
  break;
  case STATUS_VIDEO_END:
  printf("\n|         |file  end|         |             |        |        |\n");
  break;
  case STATUS_VIDEO_TIME_OUT:
  printf("\n|         |time  out|         |             |        |        |\n");
  }
}

void status_footer ()
{
return;
  printf("+---------+---------+---------+-------------+--------+--------+\n");
}
