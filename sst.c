#define INCLUDED	// Define externs here
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include "conio.h"
#include "sstlinux.h"
#include "sst.h"

#ifndef SSTDOC
#define SSTDOC	"sst.doc"
#endif
	
static char line[128], *linep = line;

/*

Here are Tom Almy's changes:

 Compared to original version, I've changed the "help" command to
   "call" and the "terminate" command to "quit" to better match
   user expectations. The DECUS version apparently made those changes
   as well as changing "freeze" to "save". However I like "freeze".

   When I got a later version of Super Star Trek that I was converting
   from, I added the emexit command.

   That later version also mentions srscan and lrscan working when
   docked (using the starbase's scanners), so I made some changes here
   to do this (and indicating that fact to the player), and then realized
   the base would have a subspace radio as well -- doing a Chart when docked
   updates the star chart, and all radio reports will be heard. The Dock
   command will also give a report if a base is under attack.

   Movecom no longer reports movement if sensors are damaged so you wouldn't
   otherwise know it.

   Also added:

   1. Better base positioning at startup

   2. Deathray improvement (but keeping original failure alternatives)

   3. Tholian Web.

   4. Enemies can ram the Enterprise. Regular Klingons and Romulans can
      move in Expert and Emeritus games. This code could use improvement.

   5. The deep space probe looks interesting! DECUS version

   6. Perhaps cloaking to be added later? BSD version

Here are Stas Sergeev's changes (controlled by the proprocessor symbol
SERGEEV, not yet completely merged):

   1. The Space Thingy can be shoved, if you it ram, and can fire back if 
      fired upon.

   1 The Tholian can be hit with phasers

   2. When you are docked, base covers you with an almost invincible shields 
      (a commander can still ram you, or a Romulan can destroy the base,
      or a SCom can even succeed with direct attack IIRC, but this rarely 
      happens).

    3. SCom can't escape from you if no more enemies remain (without this, 
       chasing SCom can take an eternity).

    4. Probe target you enter is now the destination quadrant. Before I don't 
       remember what it was, but it was something I had difficulty using)

    5. Secret password is now autogenerated.

    6. "Plaque" is adjusted for A4 paper:)

    7. Phasers now tells you how much energy needed, but only if the computer 
       is alive.

    8. Planets are auto-scanned when you enter the quadrant.

    9. Mining or using crystals in presense of enemy now yields an attack.
       There are other minor adjustments to what yields an attack
       and what does not.

    10. Ramming a black hole is no longer instant death.  There is a
        chance you might get timewarped instead.

    11. "freeze" command reverts to "save", most people will understand this
        better anyway.

Eric Raymond's changes:

     1. "sos" and "call" becomes "mayday", "freeze" and "save" are both good.

   */

static struct 
{
    char *name;
    int value;
}
commands[] = {
#ifndef SERGEEV
#define SRSCAN	1
	{"SRSCAN",	SRSCAN},
	{"STATUS",	SRSCAN},
#define LRSCAN	2
	{"LRSCAN",	LRSCAN},
#endif /* SERGEEV */
#define PHASERS	3
	{"PHASERS",	PHASERS},
#define TORPEDO	4
        {"TORPEDO",	TORPEDO},
	{"PHOTONS",	TORPEDO},
#define MOVE	5
	{"MOVE",	MOVE},
#define SHIELDS	6
	{"SHIELDS",	SHIELDS},
#define DOCK	7
	{"DOCK",	DOCK},
#define DAMAGES	8
	{"DAMAGES",	DAMAGES},
#define CHART	9
	{"CHART",	CHART},
#define IMPULSE	10
	{"IMPULSE",	IMPULSE},
#define REST	11
	{"REST",	REST},
#define WARP	12
	{"WARP",	WARP},
#define SCORE	13
	{"SCORE",	SCORE},
#ifndef SERGEEV
#define SENSORS	14
	{"SENSORS",	SENSORS},
#endif /* SERGEEV */
#define ORBIT	15
	{"ORBIT",	ORBIT},
#define TRANSPORT	16
	{"TRANSPORT",	TRANSPORT},
#define MINE	17
	{"MINE",	MINE},
#define CRYSTALS 18
	{"CRYSTALS",	CRYSTALS},
#define SHUTTLE	19
	{"SHUTTLE",	SHUTTLE},
#define PLANETS	20
	{"PLANETS",	PLANETS},
#define REPORT	21
	{"REPORT",	REPORT},
#define COMPUTER	23
	{"COMPUTER",	COMPUTER},
#define COMMANDS	24
	{"COMMANDS",	COMMANDS},
#define EMEXIT	25
	{"EMEXIT",	EMEXIT},
#define PROBE	26
	{"PROBE",	PROBE},
#define SAVE	27
	{"SAVE",	SAVE},
	{"FREEZE",	SAVE},
#define ABANDON	28
	{"ABANDON",	ABANDON},
#define DESTRUCT 29
	{"DESTRUCT",	DESTRUCT},
#define DEATHRAY 30
	{"DEATHRAY",	DEATHRAY},
#define DEBUGCMD	31
	{"DEBUG",	DEBUGCMD},
#define MAYDAY	32
	{"MAYDAY",	MAYDAY},
	{"SOS",		MAYDAY},
	{"CALL",	MAYDAY},
#define QUIT	33
	{"QUIT",	QUIT},
#define HELP	34
	{"HELP",	HELP},
};

#define NUMCOMMANDS	sizeof(commands)/sizeof(commands[0])

static void listCommands(int x) {
    int i;
    prout("LEGAL COMMANDS ARE:");
    for (i = 0; i < NUMCOMMANDS; i++) {
	proutn("%-12s ", commands[i].name);
	if (i % 5 == 4)
	    skip(1);
    }
    skip(1);
}

static void helpme(void) {
	int i, j;
	char cmdbuf[32], *cp;
	char linebuf[132];
	FILE *fp;
	/* Give help on commands */
	int key;
	key = scan();
	while (TRUE) {
		if (key == IHEOL) {
#ifdef SERGEEV
                        setwnd(BOTTOM_WINDOW);
#endif /* SERGEEV */
                        proutn("Help on what command? ");
			key = scan();
		}
#ifdef SERGEEV
                setwnd(LOWER_WINDOW);
#endif /* SERGEEV */
		if (key == IHEOL) return;
		for (i = 0; i < NUMCOMMANDS; i++) {
		    if (strcasecmp(commands[i].name, citem)==0) {
			i = commands[i].value;
			break;
		    }
		}
		if (i != NUMCOMMANDS) break;
		skip(1);
		prout("Valid commands:");
		listCommands(FALSE);
		key = IHEOL;
		chew();
		skip(1);
	}
	if (i == COMMANDS) {
		strcpy(cmdbuf, " ABBREV");
	}
	else {
	    for (j = 0; commands[i].name[j]; j++)
		cmdbuf[j] = toupper(commands[i].name[j]);
	    cmdbuf[j] = '\0';
	}
	fp = fopen(SSTDOC, "r");
	if (fp == NULL) {
		prout("Spock-  \"Captain, that information is missing from the");
		prout("   computer.\"");
		/*
		 * This used to continue: "You need to find SST.DOC and put 
		 * it in the current directory."
		 */
		return;
	}
	for (;;) {
	    if (fgets(linebuf, sizeof(linebuf), fp) == NULL) {
			prout("Spock- \"Captain, there is no information on that command.\"");
			fclose(fp);
			return;
		}
	    if (linebuf[0] == '%' && linebuf[1] == '%'&& linebuf[2] == ' ') {
		for (cp = linebuf+3; isspace(*cp); cp++)
			continue;
		linebuf[strlen(linebuf)-1] = '\0';
		if (strcasecmp(cp, cmdbuf) == 0)
		    break;
	    }
	}

	skip(1);
	prout("Spock- \"Captain, I've found the following information:\"");
	skip(1);

	while (fgets(linebuf, sizeof(linebuf),fp)) {
		if (strstr(linebuf, "******"))
			break;
		proutc(linebuf);
	}
	fclose(fp);
}

void drawmaps(short l) {
/* hook to be called after moving to redraw maps */
#ifdef SERGEEV
     _setcursortype(_NOCURSOR);
     if (l==1) sensor();
     if (l!=2) setwnd(LEFTUPPER_WINDOW);
     gotoxy(1,1);
     strcpy(line,"s");
     srscan(1);
     if (l!=2){
        setwnd(SRSCAN_WINDOW);
        clrscr();
        srscan(2);
        setwnd(LRSCAN_WINDOW);
        clrscr();
        strcpy(line,"l");
        lrscan();
        _setcursortype(_NORMALCURSOR);
     }
#endif /* SERGEEV */
}

static void makemoves(void) {
	int i, hitme;
#ifdef SERGEEV
        clrscr();
        setwnd(LOWER_WINDOW);
#endif /* SERGEEV */
	while (TRUE) { /* command loop */
                drawmaps(1);
                while (TRUE)  { /* get a command */
			hitme = FALSE;
			justin = 0;
			Time = 0.0;
			i = -1;
			chew();
#ifdef SERGEEV
                        setwnd(BOTTOM_WINDOW);
                        clrscr();
#endif /* SERGEEV */
			proutn("COMMAND> ");
                        if (scan() == IHEOL) {
#ifdef SERGEEV
                            _setcursortype(_NOCURSOR);
                            setwnd(LOWER_WINDOW);
                            clrscr();
                            chart(0);
                            _setcursortype(_NORMALCURSOR);
#endif /* SERGEEV */
                            continue;
                        }
#ifdef SERGEEV
                        ididit=0;
                        clrscr();
                        setwnd(LOWER_WINDOW);
                        clrscr();
#endif /* SERGEEV */
			for (i=0; i < ABANDON; i++)
			    if (isit(commands[i].name)) {
				i = commands[i].value;
				break;
			    }
			if (i < ABANDON) break;
			for (; i < NUMCOMMANDS; i++)
			    if (strcasecmp(commands[i].name, citem) == 0) {
			    	    i = commands[i].value;
				    break;
			    }
			if (i < NUMCOMMANDS) break;

			listCommands(TRUE);
		}
		commandhook(commands[i].name, TRUE);
		switch (i) { /* command switch */
#ifndef SERGEEV
                        case SRSCAN:                 // srscan
 				srscan(1);
 				break;
 			case LRSCAN:			// lrscan
 				lrscan();
                                break;
#endif /* SERGEEV */
			case PHASERS:			// phasers
				phasers();
				if (ididit) hitme = TRUE;
				break;
			case TORPEDO:			// photons
				photon();
				if (ididit) hitme = TRUE;
				break;
			case MOVE:			// move
				warp(1);
				break;
			case SHIELDS:			// shields
				doshield(1);
				if (ididit) {
					hitme=TRUE;
					shldchg = 0;
				}
				break;
			case DOCK:			// dock
                                dock(1);
                                if (ididit) attack(0);
				break;
			case DAMAGES:			// damages
				dreprt();
				break;
			case CHART:			// chart
				chart(0);
				break;
			case IMPULSE:			// impulse
				impuls();
				break;
			case REST:		// rest
				wait();
				if (ididit) hitme = TRUE;
				break;
			case WARP:		// warp
				setwrp();
				break;
                        case SCORE:                // score
                                score();
				break;
#ifndef SERGEEV
			case SENSORS:			// sensors
				sensor();
				break;
#endif /* SERGEEV */
			case ORBIT:			// orbit
				orbit();
				if (ididit) hitme = TRUE;
				break;
			case TRANSPORT:			// transport "beam"
				beam();
				break;
			case MINE:			// mine
				mine();
				if (ididit) hitme = TRUE;
				break;
			case CRYSTALS:			// crystals
				usecrystals();
                                if (ididit) hitme = TRUE;
				break;
			case SHUTTLE:			// shuttle
				shuttle();
				if (ididit) hitme = TRUE;
				break;
			case PLANETS:			// Planet list
				preport();
				break;
			case REPORT:			// Game Report 
				report();
				break;
			case COMPUTER:			// use COMPUTER!
				eta();
				break;
			case COMMANDS:
				listCommands(TRUE);
				break;
			case EMEXIT:		// Emergency exit
				clrscr();	// Hide screen
				freeze(TRUE);	// forced save
				exit(1);		// And quick exit
				break;
			case PROBE:
				probe();		// Launch probe
                                if (ididit) hitme = TRUE;
				break;
			case ABANDON:			// Abandon Ship
				abandn();
				break;
			case DESTRUCT:			// Self Destruct
				dstrct();
				break;
			case SAVE:			// Save Game
				freeze(FALSE);
#ifdef SERGEEV
                                clrscr();
#endif /* SERGEEV */
				if (skill > 3)
                                        prout("WARNING--Saved games produce no plaques!");
				break;
			case DEATHRAY:		// Try a desparation measure
				deathray();
				if (ididit) hitme = TRUE;
				break;
			case DEBUGCMD:		// What do we want for debug???
#ifdef DEBUG
				debugme();
#endif
				break;
			case MAYDAY:		// Call for help
				help();
                                if (ididit) hitme = TRUE;
				break;
			case QUIT:
				alldone = 1;	// quit the game
#ifdef DEBUG
				if (idebug) score();
#endif
				break;
			case HELP:
				helpme();	// get help
				break;
		}
		commandhook(commands[i].name, FALSE);
		for (;;) {
			if (alldone) break;		// Game has ended
#ifdef DEBUG
			if (idebug) prout("2500");
#endif
			if (Time != 0.0) {
				events();
				if (alldone) break;		// Events did us in
			}
			if (game.state.galaxy[quadx][quady] == 1000) { // Galaxy went Nova!
				atover(0);
				continue;
			}
			if (hitme && justin==0) {
				attack(2);
				if (alldone) break;
				if (game.state.galaxy[quadx][quady] == 1000) {	// went NOVA! 
					atover(0);
					hitme = TRUE;
					continue;
				}
			}
			break;
		}
		if (alldone) break;
	}
}


int main(int argc, char **argv) {
	int i, option, usecurses = TRUE;

	while ((option = getopt(argc, argv, "t")) != -1) {
	    switch (option) {
	    case 't':
		usecurses = FALSE;
		break;
	    default:
		fprintf(stderr, "usage: sst [-t] [startcommand...].\n");
		exit(0);
	    }
	}

#ifndef SERGEEV
	iostart(usecurses);
#else
	randomize();
        textattr(7);
        clrscr();
        setwnd(FULLSCREEN_WINDOW);
#endif /* SERGEEV */
	line[0] = '\0';
	for (i = optind; i < argc;  i++) {
		strcat(line, argv[i]);
		strcat(line, " ");
	}
	while (TRUE) { /* Play a game */
	        prelim();
		setup(line[0] == '\0');
		if (alldone) {
			score();
			alldone = 0;
		}
		else makemoves();
		skip(1);
		stars();
		skip(1);

		if (tourn && alldone) {
			proutn("Do you want your score recorded?");
			if (ja()) {
				chew2();
				freeze(FALSE);
			}
		}
		proutn("Do you want to play again? ");
		if (!ja()) break;
#ifdef SERGEEV
		setwnd(FULLSCREEN_WINDOW);
		clrscr();
#endif /* SERGEEV */
	}
	skip(1);
#ifndef SERGEEV
	ioend();
#endif /* SERGEEV */
	prout("May the Great Bird of the Galaxy roost upon your home planet.");
	return 0;
}


void cramen(int i) {
	/* return an enemy */
	char *s;
	
	switch (i) {
		case IHR: s = "Romulan"; break;
		case IHK: s = "Klingon"; break;
		case IHC: s = "Commander"; break;
		case IHS: s = "Super-commander"; break;
		case IHSTAR: s = "Star"; break;
		case IHP: s = "Planet"; break;
		case IHB: s = "Starbase"; break;
		case IHBLANK: s = "Black hole"; break;
		case IHT: s = "Tholian"; break;
		case IHWEB: s = "Tholian web"; break;
                case IHQUEST: s = "Stranger"; break;
		default: s = "Unknown??"; break;
	}
	proutn(s);
}

char *cramlc(enum loctype key, int x, int y) {
	static char buf[32];
	buf[0] = '\0';
	if (key == quadrant) strcpy(buf, "Quadrant ");
	else if (key == sector) strcpy(buf, "Sector ");
	sprintf(buf+strlen(buf), "%d - %d", x, y);
	return buf;
}

void crmena(int i, int enemy, int key, int x, int y) {
	if (i == 1) proutn("***");
	cramen(enemy);
	proutn(" at ");
	proutn(cramlc(key, x, y));
}

void crmshp(void) {
	char *s;
	switch (ship) {
		case IHE: s = "Enterprise"; break;
		case IHF: s = "Faerie Queene"; break;
		default:  s = "Ship???"; break;
	}
	proutn(s);
}

void stars(void) {
	prouts("******************************************************");
	skip(1);
}

double expran(double avrage) {
	return -avrage*log(1e-7 + Rand());
}

double Rand(void) {
	return rand()/(1.0 + (double)RAND_MAX);
}

void iran8(int *i, int *j) {
	*i = Rand()*8.0 + 1.0;
	*j = Rand()*8.0 + 1.0;
}

void iran10(int *i, int *j) {
	*i = Rand()*10.0 + 1.0;
	*j = Rand()*10.0 + 1.0;
}

void chew(void) {
	linep = line;
	*linep = 0;
}

void chew2(void) {
	/* return IHEOL next time */
	linep = line+1;
	*linep = 0;
}

int scan(void) {
	int i;
	char *cp;

	// Init result
	aaitem = 0.0;
	*citem = 0;

	// Read a line if nothing here
	if (*linep == 0) {
		if (linep != line) {
			chew();
			return IHEOL;
		}
		cgetline(line, sizeof(line));
#ifdef SERGEEV
                fflush(stdin);
                if (curwnd==BOTTOM_WINDOW){
                   clrscr();
                   setwnd(LOWER_WINDOW);
                   clrscr();
                }
#endif /* SERGEEV */
		linep = line;
	}
	// Skip leading white space
	while (*linep == ' ') linep++;
	// Nothing left
	if (*linep == 0) {
		chew();
		return IHEOL;
	}
	if (isdigit(*linep) || *linep=='+' || *linep=='-' || *linep=='.') {
		// treat as a number
	    i = 0;
	    if (sscanf(linep, "%lf%n", &aaitem, &i) < 1) {
		linep = line; // Invalid numbers are ignored
		*linep = 0;
		return IHEOL;
	    }
	    else {
		// skip to end
		linep += i;
		return IHREAL;
	    }
	}
	// Treat as alpha
	cp = citem;
	while (*linep && *linep!=' ') {
		if ((cp - citem) < 9) *cp++ = tolower(*linep);
		linep++;
	}
	*cp = 0;
	return IHALPHA;
}

int ja(void) {
	chew();
	while (TRUE) {
		scan();
		chew();
		if (*citem == 'y') return TRUE;
		if (*citem == 'n') return FALSE;
		proutn("Please answer with \"Y\" or \"N\": ");
	}
}

void huh(void) {
	chew();
	skip(1);
	prout("Beg your pardon, Captain?");
}

int isit(char *s) {
	/* New function -- compares s to scaned citem and returns true if it
	   matches to the length of s */

	return strncasecmp(s, citem, max(1, strlen(citem))) == 0;

}

#ifdef DEBUG
void debugme(void) {
	proutn("Reset levels? ");
	if (ja() != 0) {
		if (energy < inenrg) energy = inenrg;
		shield = inshld;
		torps = intorps;
		lsupres = inlsr;
	}
	proutn("Reset damage? ");
	if (ja() != 0) {
		int i;
		for (i=0; i <= NDEVICES; i++) if (damage[i] > 0.0) damage[i] = 0.0;
		stdamtim = 1e30;
	}
	proutn("Toggle idebug? ");
	if (ja() != 0) {
		idebug = !idebug;
		if (idebug) prout("Debug output ON");
		else prout("Debug output OFF");
	}
	proutn("Cause selective damage? ");
	if (ja() != 0) {
		int i, key;
		for (i=1; i <= NDEVICES; i++) {
			proutn("Kill ");
			proutn(device[i]);
			proutn("? ");
			chew();
			key = scan();
			if (key == IHALPHA &&  isit("y")) {
				damage[i] = 10.0;
				if (i == DRADIO) stdamtim = game.state.date;
			}
		}
	}
	proutn("Examine/change events? ");
	if (ja() != 0) {
		int i;
		for (i = 1; i < NEVENTS; i++) {
			int key;
			if (future[i] == 1e30) continue;
			switch (i) {
				case FSNOVA:  proutn("Supernova       "); break;
				case FTBEAM:  proutn("T Beam          "); break;
				case FSNAP:   proutn("Snapshot        "); break;
				case FBATTAK: proutn("Base Attack     "); break;
				case FCDBAS:  proutn("Base Destroy    "); break;
				case FSCMOVE: proutn("SC Move         "); break;
				case FSCDBAS: proutn("SC Base Destroy "); break;
			}
			proutn("%.2f", future[i]-game.state.date);
			chew();
			proutn("  ?");
			key = scan();
			if (key == IHREAL) {
				future[i] = game.state.date + aaitem;
			}
		}
		chew();
	}
}
			

#endif
