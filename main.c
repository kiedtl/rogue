/*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 * @(#)main.c		3.27 (Berkeley) 6/15/81
 */

#include "curses.h"
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <termios.h>
#include <stdlib.h>

#include "rogue.h"

struct termios terminal;
WINDOW *cw;               /* Window that the player sees */
WINDOW *hw;               /* Used for the help command */
WINDOW *mw;               /* Used to store mosnters */

int
main(int argc, char **argv, char **envp)
{
	char *env;
	struct passwd *pw;
	struct linked_list *item;
	struct object *obj;

	/*
	 * check for print-score option
	 */
	if (argc == 2 && strcmp(argv[1], "-s") == 0) {
		waswizard = TRUE;
		score(0, -1);
		exit(0);
	}

	/*
	 * get home and options from environment
	 */
	if ((env = getenv("HOME")) != NULL)
		strcpy(home, env);
	else if ((pw = getpwuid(getuid())) != NULL)
		strcpy(home, pw->pw_dir);
	else
		home[0] = '\0';
	strcat(home, "/");

	strcpy(file_name, home);
	strcat(file_name, "rogue.sav");

	if ((env = getenv("ROGUEOPTS")) != NULL)
		parse_opts(env);

	if (env == NULL || whoami[0] == '\0') {
		if ((pw = getpwuid(getuid())) == NULL) {
			printf("Say, who the hell are you?\n");
			exit(1);
		}

		strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
	}

	if (env == NULL || fruit[0] == '\0')
		strcpy(fruit, "slime-mold");

	if (argc == 2) {
		if (!restore(argv[1], envp)) { /* Note: restore will never return */
			endwin();
			exit(1);
		}
	} 

	/* setup seed */
	if (wizard && getenv("SEED") != NULL)
		seed = strtol(getenv("SEED"), NULL, 0);
	else
		seed = time(NULL) + getpid();

	if (wizard)
		printf("Hello %s, welcome to dungeon (seed %d)", whoami, seed);
	else
		printf("Hello %s, just a moment while I dig the dungeon...", whoami);

	fflush(stdout);
	init_player();        /* Roll up the rogue */
	init_things();        /* Set up probabilities of things */
	init_names();         /* Set up names of scrolls */
	init_colors();        /* Set up colors of potions */
	init_stones();        /* Set up stone settings of rings */
	init_materials();     /* Set up materials of wands */
	initscr();            /* Start up cursor package */

	if (COLS < 70 || LINES < 22) {
		printf("min terminal size: %d lines, %d cols\n", LINES, COLS);
		endwin();
		exit(1);
	}

	setup();

	/*
	 * Set up windows
	 */
	cw = newwin(LINES, COLS, 0, 0);
	mw = newwin(LINES, COLS, 0, 0);
	hw = newwin(LINES, COLS, 0, 0);
	waswizard = wizard;
	new_level();        /* Draw current level */

	/*
	 * Start up daemons and fuses
	 */
	daemon(doctor, 0, AFTER);
	fuse(swander, 0, WANDERTIME, AFTER);
	daemon(stomach, 0, AFTER);
	daemon(runners, 0, AFTER);

	/*
	 * Give the rogue his weaponry.  First a mace.
	 */
	item = new_item(sizeof *obj);
	obj = (struct object *) ldata(item);
	obj->o_type = WEAPON;
	obj->o_which = MACE;
	init_weapon(obj, MACE);
	obj->o_hplus = 1;
	obj->o_dplus = 1;
	obj->o_flags |= ISKNOW;
	add_pack(item, TRUE);
	cur_weapon = obj;

	/*
	 * Now a +1 bow
	 */
	item = new_item(sizeof *obj);
	obj = (struct object *) ldata(item);
	obj->o_type = WEAPON;
	obj->o_which = BOW;
	init_weapon(obj, BOW);
	obj->o_hplus = 1;
	obj->o_dplus = 0;
	obj->o_flags |= ISKNOW;
	add_pack(item, TRUE);

	/*
	 * Now some arrows
	 */
	item = new_item(sizeof *obj);
	obj = (struct object *) ldata(item);
	obj->o_type = WEAPON;
	obj->o_which = ARROW;
	init_weapon(obj, ARROW);
	obj->o_count = 25+rnd(15);
	obj->o_hplus = obj->o_dplus = 0;
	obj->o_flags |= ISKNOW;
	add_pack(item, TRUE);

	/*
	 * And his suit of armor
	 */
	item = new_item(sizeof *obj);
	obj = (struct object *) ldata(item);
	obj->o_type = ARMOR;
	obj->o_which = RING_MAIL;
	obj->o_ac = a_class[RING_MAIL] - 1;
	obj->o_flags |= ISKNOW;
	cur_armor = obj;
	add_pack(item, TRUE);

	/*
	 * Give him some food too
	 */
	item = new_item(sizeof *obj);
	obj = (struct object *) ldata(item);
	obj->o_type = FOOD;
	obj->o_count = 1;
	obj->o_which = 0;
	add_pack(item, TRUE);
	playit();
}

/*
 * endit:
 *		Exit the program abnormally.
 */
_Noreturn void
endit(int p)
{
	endwin();
	exit(0);
}

/*
 * rnd:
 *		Pick a very random number.
 */
int
rnd(int range)
{
	return range == 0 ? 0 : abs(RN) % range;
}

/*
 * roll:
 *		roll a number of dice
 */
int
roll(int number, int sides)
{
	int dtotal = 0;
	while(number--)
		dtotal += rnd(sides)+1;
	return dtotal;
}

/*
 * handle stop and start signals
 */
# ifdef SIGTSTP
void
tstp(int p)
{
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
	fflush(stdout);
	kill(0, SIGTSTP);
	signal(SIGTSTP, tstp);
	crmode();
	noecho();
	clearok(curscr, TRUE);
	touchwin(cw);
	draw(cw);
	flush_type();		/* flush input */
}
# endif

void
setup(void)
{
#ifndef DUMP
	signal(SIGHUP, auto_save);
	signal(SIGILL, auto_save);
	signal(SIGTRAP, auto_save);
#ifdef SIGIOT
	signal(SIGIOT, auto_save);
#endif
#ifdef SIGEMT
	signal(SIGEMT, auto_save);
#endif
	signal(SIGFPE, auto_save);
#ifdef SIGBUS
	signal(SIGBUS, auto_save);
#endif
	signal(SIGSEGV, auto_save);
#ifdef SIGSYS
	signal(SIGSYS, auto_save);
#endif
	signal(SIGPIPE, auto_save);
	signal(SIGTERM, auto_save);
#endif

	signal(SIGINT, quit);
#ifndef DUMP
	signal(SIGQUIT, endit);
#endif
#ifdef SIGTSTP
	signal(SIGTSTP, tstp);
#endif
	crmode();		/* Cbreak mode */
	noecho();		/* Echo off */
}

/*
 * playit:
 *		The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */
void
playit(void)
{
	/*
	 * parse environment declaration of options
	 */
	char *opts;
	if ((opts = getenv("ROGUEOPTS")) != NULL)
		parse_opts(opts);

	oldpos = hero;
	oldrp = roomin(&hero);
	while (playing)
		command();         /* Command execution */
	endit(-1);
}
