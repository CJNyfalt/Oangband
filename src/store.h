#ifndef INCLUDED_STORE_H
#define INCLUDED_STORE_H

#include "types.h"

#define STORE_INVEN_MAX		24    /* Max number of discrete objs in inven */
#define STORE_TURNS		1000  /* Number of turns between turnovers */
#define STORE_SHUFFLE		20    /* 1/Chance (per day) of an owner changing */
#define STORE_MIN_KEEP  6       /* Min slots to "always" keep full (>0) */
#define STORE_MAX_KEEP  18      /* Max slots to "always" keep full (<STORE_INVEN_MAX) */
#define STORE_CHOICES	32		/* Number of items to choose stock from */




/*
 * Total number of stores (see "store.c", etc)
 */
#define MAX_STORES	9

/*
 * Store number of the player's home (see "store.c", etc)
 * Taken from Sangband.
 */
#define STORE_HOME 		7

/*
 * Store number of the Black Market
 */
#define STORE_BLACKM 		6



typedef struct owner_type owner_type;
typedef struct store_type store_type;


/*
 * A store owner
 */
struct owner_type
{
	u32b owner_name;		/* Name */
        u32b unused;		/* Unused */

	s16b max_cost;		/* Purse limit */

	int max_inflate;		/* Initial Inflation */
	int min_inflate;		/* Final Offer Inflation */

	byte haggle_per;		/* Haggle unit */

	byte insult_max;		/* Insult limit */

	byte owner_race;		/* Owner race */

};




/*
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
struct store_type
{
	byte owner;				/* Owner index */
	byte extra;				/* Unused for now */

	s16b insult_cur;		/* Insult counter */

	s16b good_buy;			/* Number of "good" buys */
	s16b bad_buy;			/* Number of "bad" buys */

	s32b store_open;		/* Closed until this turn */

	s32b store_wrap;		/* Unused for now */

	s16b table_num;			/* Table -- Number of entries */
	s16b table_size;		/* Table -- Total Size of Array */
	s16b *table;			/* Table -- Legal item kinds */

	s16b stock_num;			/* Stock -- Number of entries */
	s16b stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */
};





extern void display_home_inventory_remote(void);
extern void do_cmd_store(void);
extern void store_shuffle(int which);
extern void store_maint(int which);
extern void store_init(int which);




#endif /* INCLUDED_STORE_H */
