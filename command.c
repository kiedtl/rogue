/*
 * Read and execute the user commands
 *
 * @(#)command.c		3.45 (Berkeley) 6/15/81
 */

#include <curses.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

#include "dungeon.h"
#include "rogue.h"

/*
 * command:
 *		Process the user commands
 */

char countch = FALSE, direction = FALSE, newcount = FALSE;

_Bool
command(void)
{
	char ch;
	int ntimes = 1;		/* Number of player moves */

	if (on(player, ISHASTE))
		ntimes++;

	while (ntimes) {
		look(TRUE);

		if (!running)
			door_stop = FALSE;

		status();
		lastscore = purse;

		if (!((running || count) && jump))
			draw();

		take = 0;
		after = TRUE;

		/*
		 * Read command or continue run
		 */
		if (wizard)
			waswizard = TRUE;

		if (!no_command) {
			if (running) {
				ch = runch;
			} else if (count) {
				ch = countch;
			} else {
				ch = readchar();
				if (mpos != 0 && !running)    /* Erase message */
					msg("");
			}
		} else {
			ch = ' ';
		}

		if (no_command) {
			if (--no_command == 0)
				msg("You can move again.");
		} else {
			/*
			 * check for prefixes
			 */
			if (isdigit(ch)) {
				count = 0;
				newcount = TRUE;
				while (isdigit(ch)) {
					count = count * 10 + (ch - '0');
					ch = readchar();
				}

				countch = ch;

				/*
				 * turn off count for commands which don't make sense
				 * to repeat
				 */
				switch (ch) {
				case 'h': case 'j': case 'k': case 'l':
				case 'y': case 'u': case 'b': case 'n':
				case 'H': case 'J': case 'K': case 'L':
				case 'Y': case 'U': case 'B': case 'N':
				case 'q': case 'r': case 's': case 'f':
				case 't': case 'C': case 'I': case ' ':
				case 'z': case 'p':
					break;
				default:
					count = 0;
				}
			}

			switch (ch) {
			case 'f':
				if (!on(player, ISBLIND)) {
					door_stop = TRUE;
					firstmove = TRUE;
				}

				if (count && !newcount)
					ch = direction;
				else
					ch = readchar();

				switch (ch)
				{
				case 'h': case 'j': case 'k': case 'l':
				case 'y': case 'u': case 'b': case 'n':
					ch = toupper(ch);
				}

				direction = ch;
			}

			newcount = FALSE;

			/*
			 * execute a command
			 */
			if (count && !running)
				count--;

			switch (ch) {
			break; case 'h': do_move(0, -1);
			break; case 'j': do_move(1, 0);
			break; case 'k': do_move(-1, 0);
			break; case 'l': do_move(0, 1);
			break; case 'y': do_move(-1, -1);
			break; case 'u': do_move(-1, 1);
			break; case 'b': do_move(1, -1);
			break; case 'n': do_move(1, 1);
			break; case 'H': case 'J': case 'K': case 'L':
			       case 'Y': case 'U': case 'B': case 'N':
				do_run(ch);
			break; case 't':
				if (!get_dir())
					after = FALSE;
				else
					missile(delta.y, delta.x);
			break; case 'Q' : after = FALSE; quit(-1);
			break; case 'i' : after = FALSE; inventory(pack, 0);
			break; case 'I' : after = FALSE; picky_inven();
			break; case 'd' : drop();
			break; case 'q' : quaff();
			break; case 'r' : read_scroll();
			break; case 'e' : eat();
			break; case 'w' : wield();
			break; case 'W' : wear();
			break; case 'T' : take_off();
			break; case 'P' : ring_on();
			break; case 'R' : ring_off();
			break; case 'c' : call();
			break; case '>' : after = FALSE; d_level();
			break; case '<' : after = FALSE; u_level();
			break; case '?' : after = FALSE; help();
			break; case '/' : after = FALSE; identify();
			break; case 's' : search();
			break; case 'z' : do_zap(FALSE);
			break; case 'p':
				if (get_dir())
					do_zap(TRUE);
				else
					after = FALSE;
			break; case 'v': msg("Rogue version %s. (mctesq was here)", release);
			break; case CTRL('L') : after = FALSE; clearok(curscr,TRUE); draw();
			break; case CTRL('R') : after = FALSE; msg(huh);
			break; case 'S' : 
				after = FALSE;
				if (save_game()) {
					wmove(cw, LINES-1, 0); 
					wclrtoeol(cw);
					draw();
					endwin();
					exit(0);
				}
			break; case ' ': case '.':
				;    /* Rest command */
			break; case CTRL('P') :
				after = FALSE;
				if (!wizard) {
					if (wizard = passwd()) {
						msg("You suddenly feel as smart as Ken Arnold (seed: %d)", dnum);
						wizard = waswizard = TRUE;
					} else {
						msg("Sorry");
					}
				}
			break; case ESCAPE:
				door_stop = FALSE;
				count = 0;
				after = FALSE;
			break; default:
				after = FALSE;

				if (wizard) {
					switch (ch) {
					break; case '@' : msg("@ %d,%d", hero.y, hero.x);
					break; case 'C' : create_obj();
					break; case CTRL('I'): inventory(lvl_obj, 0);
					break; case CTRL('W'): whatis();
					break; case CTRL('D'): level++; new_level();
					break; case CTRL('U'): level--; new_level();
					// REFACTOR break; case CTRL('F'): show_win(stdscr, "--More (level map)--");
					// REFACTOR break; case CTRL('X'): show_win(mw, "--More (monsters)--");
					break; case CTRL('T'): teleport();
					break; case CTRL('E'): msg("food left: %d", food_left);
					break; case CTRL('A'): msg("%d things in your pack", inpack);
					break; case CTRL('C'): add_pass();
					break; case CTRL('N'): {
						struct linked_list *item;
						if ((item = get_item("charge", STICK)) != NULL)
							((struct object *) ldata(item))->o_charges = 10000;
					}
					break; case CTRL('H'): {
						int i;
						struct linked_list *item;
						struct object *obj;

						for (i = 0; i < 9; i++)
							raise_level();

						/*
						 * Give the rogue a sword (+1,+1)
						 */
						item = new_item(sizeof *obj);
						obj = (struct object *) ldata(item);
						obj->o_type = WEAPON;
						obj->o_which = TWOSWORD;
						init_weapon(obj, SWORD);
						obj->o_hplus = 1;
						obj->o_dplus = 1;
						add_pack(item, TRUE);
						cur_weapon = obj;

						/*
						 * And his suit of armor
						 */
						item = new_item(sizeof *obj);
						obj = (struct object *) ldata(item);
						obj->o_type = ARMOR;
						obj->o_which = PLATE_MAIL;
						obj->o_ac = -5;
						obj->o_flags |= ISKNOW;
						cur_armor = obj;
						add_pack(item, TRUE);
					}
					break; default:
						msg("Illegal command '%s'.", unctrl(ch));
						count = 0;
					}
				} else {
					msg("Unknown command '%s'.", unctrl(ch));
					count = 0;
				}
			}

			/*
			 * turn off flags if no longer needed
			 */
			if (!running)
				door_stop = FALSE;
		}

		/*
		 * If he ran into something to take, let him pick it up.
		 */
		if (take != 0)
			pick_up(take);
		if (!running)
			door_stop = FALSE;

		--ntimes;
	}

	return after;
}

/*
 * quit:
 *		Have player make certain, then exit.
 */
void
quit(int p)
{
	/*
	 * Reset the signal in case we got here via an interrupt
	 */
	if (signal(SIGINT, quit) != quit)
		mpos = 0;
	msg("Suicide is not the answer. Really quit?");
	draw();

	if (readchar() == 'y') {
		clear();
		move(LINES-1, 0);
		draw();
		score(purse, 1);
		exit(0);
	} else {
		signal(SIGINT, quit);
		wmove(cw, 0, 0);
		wclrtoeol(cw);
		status();
		draw();
		mpos = 0;
		count = 0;
	}
}

/*
 * search:
 *		Player gropes about him to find hidden things.
 */
void
search()
{
	register int x, y;
	register char ch;

	/*
	 * Look all around the hero, if there is something hidden there,
	 * give him a chance to find it.  If its found, display it.
	 */
	if (on(player, ISBLIND))
		return;
	for (x = hero.x - 1; x <= hero.x + 1; x++)
		for (y = hero.y - 1; y <= hero.y + 1; y++) {
			ch = dungeonat(y, x);
			switch (ch) {
				break; case SECRETDOOR:
					if (rnd(100) < 20) {
						mvaddch(y, x, DOOR);
						count = 0;
					}
				break; case TRAP: {
					struct trap *tp;

					if (mvwinch(cw, y, x) == TRAP)
						break;

					if (rnd(100) > 50)
						break;

					tp = trap_at(y, x);
					tp->tr_flags |= ISFOUND;
					mvwaddch(cw, y, x, TRAP);
					count = 0;
					running = FALSE;
					msg(tr_name(tp->tr_type));
				}
			}
		}
}

/*
 * help:
 *		Show the whole help mess
 */
void
help(void)
{
	struct h_list *strp = helpstr;
	char helpch;
	int cnt;

	mpos = 0;
	/*
	 * Here we print help for everything.
	 * Then wait before we return to command mode
	 */
	wclear(hw);
	cnt = 0;
	while (strp->h_ch)
	{
		mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
		waddstr(hw, strp->h_desc);
		cnt++;
		strp++;
	}
	wmove(hw, LINES-1, 0);
	wprintw(hw, "--Press space to continue--");
	wrefresh(hw);
	wait_for(' ');
	wclear(hw);
	wrefresh(hw);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status();
	touchwin(cw);
}

/*
 * identify:
 *		Tell the player what a certain thing is.
 */
void
identify(void)
{
	char ch, *str;

	msg("What do you want identified? ");
	ch = readchar();
	mpos = 0;
	if (ch == ESCAPE) {
		msg("");
		return;
	}

	if (isalpha(ch) && isupper(ch)) {
		str = monsters[ch-'A'].m_name;
	} else {
		switch(ch) {
		break; case '|': case '-':
			str = "wall of a room";
		break; case GOLD: str = "gold";
		break; case STAIRS : str = "passage leading down";
		break; case DOOR: str = "door";
		break; case FLOOR: str = "room floor";
		break; case PLAYER: str = "you";
		break; case PASSAGE: str = "passage";
		break; case TRAP: str = "trap";
		break; case POTION: str = "potion";
		break; case SCROLL: str = "scroll";
		break; case FOOD: str = "food";
		break; case WEAPON: str = "weapon";
		break; case ' ' : str = "solid rock";
		break; case ARMOR: str = "armor";
		break; case AMULET: str = "The Amulet of Yendor";
		break; case RING: str = "ring";
		break; case STICK: str = "wand or staff";
		break; default:
			str = "unknown character";
		}
	}
	msg("'%s' : %s", unctrl(ch), str);
}

/*
 * d_level:
 *		He wants to go down a level
 */
void
d_level()
{
	if (dungeonat_c(&hero) != STAIRS) {
		msg("You pound furiously on the floor.");
	} else {
		++level;
		new_level();
	}
}

/*
 * u_level:
 *		He wants to go up a level
 */
void
u_level(void)
{
	if (dungeonat_c(&hero) == STAIRS) {
		if (amulet) {
			--level;
			if (level == 0)
				total_winner();
			new_level(); // REFACTOR
			msg("You feel a wrenching sensation in your gut.");
			return;
		}
	}

	msg("You gaze in despair at the ceiling.");
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
void
call(void)
{
	struct object *obj;
	struct linked_list *item;
	char **guess, *elsewise;
	bool *know;

	item = get_item("call", CALLABLE);

	/*
	 * Make certain that it is somethings that we want to wear
	 */
	if (item == NULL)
		return;

	obj = (struct object *) ldata(item);
	switch (obj->o_type) {
	break; case RING:
		guess = r_guess;
		know = r_know;
		elsewise = (r_guess[obj->o_which] != NULL ?
					r_guess[obj->o_which] : r_stones[obj->o_which]);
	break; case POTION:
		guess = p_guess;
		know = p_know;
		elsewise = (p_guess[obj->o_which] != NULL ?
					p_guess[obj->o_which] : p_colors[obj->o_which]);
	break; case SCROLL:
		guess = s_guess;
		know = s_know;
		elsewise = (s_guess[obj->o_which] != NULL ?
					s_guess[obj->o_which] : s_names[obj->o_which]);
	break; case STICK:
		guess = ws_guess;
		know = ws_know;
		elsewise = (ws_guess[obj->o_which] != NULL ?
					ws_guess[obj->o_which] : ws_made[obj->o_which]);
	otherwise:
		msg("You can't call that anything");
		return;
	}

	if (know[obj->o_which]) {
		msg("That has already been identified");
		return;
	}

	msg("Was called \"%s\"", elsewise);
	msg("What do you want to call it? ");

	if (guess[obj->o_which] != NULL)
		free(guess[obj->o_which]);

	strcpy(prbuf, elsewise);
	if (get_str(prbuf, cw) == NORM) {
		guess[obj->o_which] = malloc((unsigned int) strlen(prbuf) + 1);
		strcpy(guess[obj->o_which], prbuf);
	}
}
