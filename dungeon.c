#include <stdint.h>
#include <curses.h>
#include <stdlib.h>

#include "rogue.h"
#include "dungeon.h"

size_t max_level = 0;
size_t level = 1;
struct Tile dungeon[MAXLEVELS][40][100];

void
dungeonset_c(struct Coord *c, size_t t)
{
	dungeonset(c->y, c->x, t);
}

void
dungeonset(size_t y, size_t x, size_t t)
{
	dungeon[level][y][x].type = t;
}

char
dungeonat_c(struct Coord *c)
{
	return dungeonat(c->y, c->x);
}

char
dungeonat(size_t y, size_t x)
{
	return (char)dungeon[level][y][x].type;
}

void
dungeoncurs_c(struct Coord *c)
{
	wmove(cw, c->y, c->x);
}

WINDOW *cw, *hw, *mw;

void
ui_init(void)
{
	initscr();
	crmode();
	noecho();
	curs_set(0); /* invisible cursor */

	if (COLS < 70 || LINES < 22) {
		ui_shutdown();
		printf("min terminal size: 70 lines, 22 cols\n");
		exit(EXIT_FAILURE);
	}

	cw = newwin(LINES, COLS, 0, 0);
	mw = newwin(LINES, COLS, 0, 0);
	hw = newwin(LINES, COLS, 0, 0);
}

void
ui_shutdown(void)
{
	endwin();
}

void
ui_draw(void)
{
	wclear(cw);
	clear();

	size_t maxx = COLS  -  8;
	size_t maxy = LINES - 30;

	ssize_t startx = hero.x - (maxx / 2);
	ssize_t   endx = hero.x + (maxx / 2);
	ssize_t starty = hero.y - (maxy / 2);
	ssize_t   endy = hero.y + (maxy / 2);

	/* xctr, yctr hold screen position,
	 * x and y counters hold dungeon position */

	size_t xctr = 0, yctr = 0;
	for (ssize_t y = starty; y < endy; ++y, ++yctr) {
		for (size_t x = startx; x < endx; ++x, ++xctr) {
			if (y < 0 || x < 0) {
				mvwaddch(cw, yctr, xctr, ' ');
				continue;
			}

			if (y == hero.y && x == hero.x)
				wattron(cw, A_REVERSE);

			char c = (char)dungeon[level][y][x].type;
			mvwaddch(cw, yctr, xctr, c ? c : ' ');

			if (y == hero.y && x == hero.x)
				wattroff(cw, A_REVERSE);
		}

		xctr = 0;
	}

	wrefresh(cw);
}

void draw(void) { ui_draw(); }
