/*
 * new_level:
 *		Dig and draw a new level
 *
 * @(#)new_level.c		3.7 (Berkeley) 6/2/81
 */

#include <curses.h>
#include "dungeon.h"
#include "rogue.h"

void
new_level()
{
	int rm;
	char ch;
	struct Coord stairs;

	if (level > max_level)
		max_level = level;

	status();

	/*
	 * Free up the monsters on the last level
	 */
	free_list(mlist);
	do_rooms();			/* Draw rooms */
	do_passages();			/* Draw passages */
	++no_food;
	put_things();			/* Place objects (if any) */

	/*
	 * Place the staircase down.
	 */
	do {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &stairs);
	} until (dungeonat_c(&stairs) == FLOOR);

	dungeonset_c(&stairs, STAIRS);

	/*
	 * Place the traps
	 */
	if (rnd(10) < level) {
		ntraps = MAX(rnd(level/4)+1, MAXTRAPS);
		for (size_t i = 0; i < ntraps; ++i) {
			do {
				rm = rnd_room();
				rnd_pos(&rooms[rm], &stairs);
			} until (dungeonat_c(&stairs) == FLOOR);

			char traptypes[] = { TRAPDOOR, BEARTRAP, SLEEPTRAP,
				ARROWTRAP, TELTRAP, DARTTRAP };
			ch = traptypes[rnd(ARRAY_LEN(traptypes))];

			dungeonset_c(&stairs, TRAP);

			traps[i].tr_type = ch;
			traps[i].tr_flags = 0;
			traps[i].tr_pos = stairs;
		}
	}

	do {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &hero);
	} until(dungeonat_c(&hero) == FLOOR);

	light(&hero);
	dungeonset_c(&hero, PLAYER);

	ui_draw();
}

/*
 * Pick a room that is really there
 */
int
rnd_room()
{
	int rm;
	do
		rm = rnd(MAXROOMS);
	while (rooms[rm].r_flags & ISGONE);
	return rm;
}

/*
 * put_things:
 *		put potions and scrolls on this level
 */
void
put_things(void)
{
	int i;
	struct linked_list *item;
	struct object *cur;
	int rm;
	struct Coord tp;

	/*
	 * Throw away stuff left on the previous level (if anything)
	 */
	free_list(lvl_obj);

	/*
	 * Once you have found the amulet, the only way to get new stuff is
	 * go down into the dungeon.
	 */
	if (amulet && level < max_level)
		return;

	/*
	 * Do MAXOBJ attempts to put things on a level
	 */
	for (i = 0; i < MAXOBJ; i++) {
		if (rnd(100) < 35) {
			/*
			 * Pick a new object and link it in the list
			 */
			item = new_thing();
			attach(lvl_obj, item);
			cur = (struct object *) ldata(item);

			/*
			 * Put it somewhere
			 */
			rm = rnd_room();
			do
				rnd_pos(&rooms[rm], &tp);
			until (dungeonat_c(&tp) == FLOOR);
			dungeonset_c(&tp, cur->o_type);
			cur->o_pos = tp;
		}
	}

	/*
	 * If he is really deep in the dungeon and he hasn't found the
	 * amulet yet, put it somewhere on the ground
	 */
	if (level > 25 && !amulet) {
		item = new_item(sizeof *cur);
		attach(lvl_obj, item);
		cur = (struct object *) ldata(item);
		cur->o_hplus = cur->o_dplus = 0;
		strcpy(cur->o_damage,"0d0");
		strcpy(cur->o_hurldmg, "0d0");
		cur->o_ac = 11;
		cur->o_type = AMULET;

		/*
		 * Put it somewhere
		 */
		rm = rnd_room();
		do
			rnd_pos(&rooms[rm], &tp);
		until (dungeonat_c(&tp) == FLOOR);
		dungeonset_c(&tp, cur->o_type);
		cur->o_pos = tp;
	}
}
