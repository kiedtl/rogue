#ifndef DUNGEON_H
#define DUNGEON_H

#include <curses.h>
#include <stdint.h>

typedef struct Coord {
    int x, y;
} coord;

struct Tile {
	size_t type;
};

#define MAXLEVELS 64
extern size_t max_level;		/* Deepest player has gone */
extern size_t level;
extern struct Tile dungeon[MAXLEVELS][40][100];

void dungeonset_c(struct Coord *c, size_t t);
char dungeonat_c(struct Coord *c);
char dungeonat(size_t y, size_t x);
void dungeoncurs_c(struct Coord *c);

/* TODO: move ui code into ui.h/ui.c */

extern WINDOW *cw;       /* Window that the player sees */
extern WINDOW *hw;       /* Used for the help command */
extern WINDOW *mw;       /* Used to store mosnters */
void ui_init(void);
void ui_shutdown(void);
void ui_draw(void);

/* for backwards-compat */
void draw(void);

#endif
