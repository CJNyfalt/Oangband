/* File: store.c */

/* Store comments, racial penalties, what an item will cost, store
 * inventory management, what a store will buy (what stores will sell
 * is in init2.c, and store owners in tables.c), interacting with the
 * stores (entering, haggling, buying, selling, leaving, getting kicked
 * out, etc.).  Shuffle store owners.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

/* Some local constants */
#define STORE_TURNOVER	9		/* Normal shop turnover, per day */
#define STORE_OBJ_LEVEL	5		/* Magic Level for normal stores */


/* Randomly select one of the entries in an array */
#define ONE_OF(x)	x[randint0(N_ELEMENTS(x))]


/*** Flavour text stuff ***/

#define MAX_COMMENT_1	6

static const char * comment_1[MAX_COMMENT_1] =
{
	"Okay.",
	"Fine.",
	"Accepted!",
	"Agreed!",
	"Done!",
	"Taken!"
};

#define MAX_COMMENT_2A	2

static const char * comment_2a[MAX_COMMENT_2A] =
{
	"You try my patience.  %s is final.",
	"My patience grows thin.  %s is final."
};

#define MAX_COMMENT_2B	12

static const char * comment_2b[MAX_COMMENT_2B] =
{
	"I can take no less than %s gold pieces.",
	"I will accept no less than %s gold pieces.",
	"Ha!  No less than %s gold pieces.",
	"You knave!  No less than %s gold pieces.",
	"That's a pittance!  I want %s gold pieces.",
	"That's an insult!  I want %s gold pieces.",
	"As if!  How about %s gold pieces?",
	"My arse!  How about %s gold pieces?",
	"May the fleas of 1000 orcs molest you!  Try %s gold pieces.",
	"May your most favourite parts go moldy!  Try %s gold pieces.",
	"May Morgoth find you tasty!  Perhaps %s gold pieces?",
	"Your mother was an Ogre!  Perhaps %s gold pieces?"
};

#define MAX_COMMENT_3A	2

static const char * comment_3a[MAX_COMMENT_3A] =
{
	"You try my patience.  %s is final.",
	"My patience grows thin.  %s is final."
};


#define MAX_COMMENT_3B	12

static const char * comment_3b[MAX_COMMENT_3B] =
{
	"Perhaps %s gold pieces?",
	"How about %s gold pieces?",
	"I will pay no more than %s gold pieces.",
	"I can afford no more than %s gold pieces.",
	"Be reasonable.  How about %s gold pieces?",
	"I'll buy it as scrap for %s gold pieces.",
	"That is too much!  How about %s gold pieces?",
	"That looks war surplus!  Say %s gold pieces?",
	"Never!  %s is more like it.",
	"That's an insult!  %s is more like it.",
	"%s gold pieces and be thankful for it!",
	"%s gold pieces and not a copper more!"
};

#define MAX_COMMENT_4A	4

static const char * comment_4a[MAX_COMMENT_4A] =
{
	"Enough!  You have abused me once too often!",
	"Arghhh!  I have had enough abuse for one day!",
	"That does it!  You shall waste my time no more!",
	"This is getting nowhere!  I'm going to Londis!"
};

#define MAX_COMMENT_4B	4

static const char * comment_4b[MAX_COMMENT_4B] =
{
	"Leave my store!",
	"Get out of my sight!",
	"Begone, you scoundrel!",
	"Out, out, out!"
};

#define MAX_COMMENT_5	8

static const char * comment_5[MAX_COMMENT_5] =
{
	"Try again.",
	"Ridiculous!",
	"You will have to do better than that!",
	"Do you wish to do business or not?",
	"You've got to be kidding!",
	"You'd better be kidding!",
	"You try my patience.",
	"Hmmm, nice weather we're having."
};

#define MAX_COMMENT_6	4

static const char * comment_6[MAX_COMMENT_6] =
{
	"I must have heard you wrong.",
	"I'm sorry, I missed that.",
	"I'm sorry, what was that?",
	"Sorry, what was that again?"
};



/*
 * Successful haggle.
 */
static void say_comment_1(void)
{
	msg_print(comment_1[randint0(MAX_COMMENT_1)]);
}


/*
 * Continue haggling (player is buying)
 */
static void say_comment_2(s32b value, int annoyed)
{
	char tmp_val[80];

	/* Prepare a string to insert */
	sprintf(tmp_val, "%ld", (long)value);

	/* Final offer */
	if (annoyed > 0)
	{
		/* Formatted message */
		msg(comment_2a[randint0(MAX_COMMENT_2A)], tmp_val);
	}

	/* Normal offer */
	else
	{
		/* Formatted message */
		msg(comment_2b[randint0(MAX_COMMENT_2B)], tmp_val);
	}
}


/*
 * Continue haggling (player is selling)
 */
static void say_comment_3(s32b value, int annoyed)
{
	char tmp_val[80];

	/* Prepare a string to insert */
	sprintf(tmp_val, "%ld", (long)value);

	/* Final offer */
	if (annoyed > 0)
	{
		/* Formatted message */
		msg(comment_3a[randint0(MAX_COMMENT_3A)], tmp_val);
	}

	/* Normal offer */
	else
	{
		/* Formatted message */
		msg(comment_3b[randint0(MAX_COMMENT_3B)], tmp_val);
	}
}


/*
 * You must leave my store
 */
static void say_comment_4(void)
{
	msg_print(comment_4a[randint0(MAX_COMMENT_4A)]);
	msg_print(comment_4b[randint0(MAX_COMMENT_4B)]);
}


/*
 * You are insulting me
 */
static void say_comment_5(void)
{
	msg_print(comment_5[randint0(MAX_COMMENT_5)]);
}


/*
 * You are making no sense.
 */
static void say_comment_6(void)
{
	msg_print(comment_6[randint0(MAX_COMMENT_6)]);
}



/*
 * Messages for reacting to purchase prices.
 */

static const char *comment_worthless[] =
{
	"Arrgghh!",
	"You bastard!",
	"You hear someone sobbing...",
	"The shopkeeper howls in agony!"
};

static const char *comment_bad[] =
{
	"Pamawitz!",
	"You fiend!",
	"The shopkeeper curses at you.",
	"The shopkeeper glares at you."
};

static const char *comment_good[] =
{
	"Cool!",
	"You've made my day!",
	"The shopkeeper giggles.",
	"The shopkeeper laughs loudly."
};

static const char *comment_great[] =
{
	"Yipee!",
	"I think I'll retire!",
	"The shopkeeper jumps for joy.",
	"The shopkeeper smiles gleefully."
};


/*
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
	/* Item was worthless, but we bought it */
	if ((value <= 0) && (price > value))
		msgt(MSG_STORE1, "%s", ONE_OF(comment_worthless));

	/* Item was cheaper than we thought, and we paid more than necessary */
	else if ((value < guess) && (price > value))
		msgt(MSG_STORE2, "%s", ONE_OF(comment_bad));

	/* Item was a good bargain, and we got away with it */
	else if ((value > guess) && (value < (4 * guess)) && (price < value))
		msgt(MSG_STORE3, "%s", ONE_OF(comment_good));

	/* Item was a great bargain, and we got away with it */
	else if ((value > guess) && (price < value))
		msgt(MSG_STORE4, "%s", ONE_OF(comment_great));
}





/*
 * We store the current "store number" here so everyone can access it
 */
static int store_num = (MAX_STORES - 1);

/*
 * We store the current "store page" here so everyone can access it
 */
static int store_top = 0;

/*
 * We store the number of items per page here so everyone can access it
 */
static int store_per = 12;

/*
 * We store the current "store pointer" here so everyone can access it
 */
static store_type *st_ptr = NULL;

/*
 * We store the current "owner type" here so everyone can access it
 */
static owner_type *ot_ptr = NULL;


/*** Check if a store will buy an object ***/

/*
 * Determine if the current store will purchase the given object
 *
 * Note that a shop-keeper must refuse to buy "worthless" objects
 */
static bool store_will_buy(object_type *o_ptr)
{
	/* Hack -- The Home is simple */
	if (store_num == STORE_HOME) return (TRUE);

	/* Switch on the store */
	switch (store_num)
	{
		/* General Store */
		case STORE_GENERAL:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_FOOD:
				case TV_LITE:
				case TV_FLASK:
				case TV_SPIKE:
				case TV_SHOT:
				case TV_ARROW:
				case TV_BOLT:
				case TV_DIGGING:
				case TV_CLOAK:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Armoury */
		case STORE_ARMOR:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_BOOTS:
				case TV_GLOVES:
				case TV_CROWN:
				case TV_HELM:
				case TV_SHIELD:
				case TV_CLOAK:
				case TV_SOFT_ARMOR:
				case TV_HARD_ARMOR:
				case TV_DRAG_ARMOR:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Weapon Shop */
		case STORE_WEAPON:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SHOT:
				case TV_BOLT:
				case TV_ARROW:
				case TV_BOW:
				case TV_DIGGING:
				case TV_HAFTED:
				case TV_POLEARM:
				case TV_SWORD:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Temple */
		case STORE_TEMPLE:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SCROLL:
				case TV_POTION:
				case TV_HAFTED:
				break;

				/* Hack -- The temple will buy known blessed swords and
				 * polearms.
				 */
				case TV_SWORD:
				case TV_POLEARM:
				{
					u32b f1, f2, f3;

					/* Extract the item flags */
					object_flags(o_ptr, &f1, &f2, &f3);

					if (f3 & (TR3_BLESSED) && object_known_p(o_ptr))
						break;
					else return (FALSE);
				}

				default:
				return (FALSE);
			}
			break;
		}

		/* Alchemist */
		case STORE_ALCHEMY:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SCROLL:
				case TV_POTION:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* Magic Shop */
		case STORE_MAGIC:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_AMULET:
				case TV_RING:
				case TV_STAFF:
				case TV_WAND:
				case TV_ROD:
				break;
				default:
				return (FALSE);
			}
			break;
		}

		/* The Bookseller */
		case STORE_BOOK:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_MAGIC_BOOK:
				case TV_PRAYER_BOOK:
				case TV_DRUID_BOOK:
				case TV_NECRO_BOOK:
				break;
				default:
				return (FALSE);
			}
			break;
		}
	}

	/* Ignore "worthless" items XXX XXX XXX */
	if (object_value(o_ptr) <= 0) return (FALSE);

	/* Assume okay */
	return (TRUE);
}



/*** Basics: pricing, generation, etc. ***/

/*
 * Determine the price of an object (qty one) in a store.  Altered in
 * Oangband.
 *
 * This function takes into account the player's charisma, and the
 * shop-keepers friendliness, and the shop-keeper's base greed, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * Charisma adjustments run from 80 to 150.
 * Racial adjustments run from 85 to 130.
 * Greed adjustments run from 102 to 210.
 */
static s32b price_item(object_type *o_ptr, int greed, bool flip)
{
	int racial_factor, charisma_factor;
	s32b offer_adjustment;
	s32b price;

	/* Get the value of one of the items */
	price = object_value(o_ptr);

	/* Worthless items */
	if (price <= 0) return (0L);

	/* Compute the racial factor */
	racial_factor = g_info[(ot_ptr->owner_race * MAX_P_IDX) + p_ptr->prace];

	/* Paranoia */
	if (!racial_factor) racial_factor=100;

	/* Add in the charisma factor */
	charisma_factor = adj_chr_gold[p_ptr->stat_ind[A_CHR]];

	/* Shop is buying */
	if (flip)
	{
		/* Calculate the offer adjustment (x100). */
		offer_adjustment = 100000000L / (racial_factor *
					 charisma_factor * greed);

		/* Never get "silly" */
		if (offer_adjustment > 100L) offer_adjustment = 100L;
	}

	/* Shop is selling */
	else
	{
		/* Calculate the offer adjustment (x100). */
		offer_adjustment = 1L * (long)racial_factor * charisma_factor *
					 greed / 10000L;

		/* Never get "silly" */
		if (offer_adjustment < 100L) offer_adjustment = 100L;
	}

	/* Compute the final price (with rounding) */
	price = (price * offer_adjustment + 50L) / 100L;

	/* Note -- Never become "free" */
	if (price <= 0L) return (1L);

	/* Return the price */
	return (price);
}


/*
 * Special "mass production" computation.
 */
static int mass_roll(int times, int max)
{
	int i, t = 0;

	assert(max > 1);

	for (i = 0; i < times; i++)
		t += randint0(max);

	return (t);
}


/*
 * Certain "cheap" objects should be created in "piles"
 * Some objects can be sold at a "discount" (in small piles)
 */
static void mass_produce(object_type *o_ptr)
{
	int size = 1;
	int discount = 0;

	int discount_probability, max_purse, temp;

	s32b cost = object_value(o_ptr);


	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Food, Flasks, and Lites */
		case TV_FOOD:
		case TV_FLASK:
		case TV_LITE:
		{
			if (cost < 6L) size += mass_roll(3, 5);
			if (cost < 21L) size += mass_roll(3, 5);
			break;
		}

		/* The Black Market sells potions and scrolls in *quantity*.  This
		 * allows certain utility objects to be actually worth using. -LM-
		 */
		case TV_POTION:
		case TV_SCROLL:
		{
			if ((store_num == STORE_BLACKM) && (randint1(2) == 1))
			{
				if (cost < 400L) size += mass_roll(15,3);
			}
			else
			{
				if (cost < 61L) size += mass_roll(3, 5);
				if (cost < 241L) size += mass_roll(1, 5);
			}
			break;
		}

		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_DRUID_BOOK:
		case TV_NECRO_BOOK:
		{
			if (cost < 51L) size += mass_roll(2, 3);
			if (cost < 501L) size += mass_roll(1, 3);
			break;
		}

		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SHIELD:
		case TV_GLOVES:
		case TV_BOOTS:
		case TV_CLOAK:
		case TV_HELM:
		case TV_CROWN:
		case TV_SWORD:
		case TV_POLEARM:
		case TV_HAFTED:
		case TV_DIGGING:
		case TV_BOW:
		{
			if (o_ptr->name2) break;
			if (cost < 11L) size += mass_roll(3, 5);
			if (cost < 101L) size += mass_roll(2, 5);
			break;
		}

		case TV_SPIKE:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			if (cost < 6L) size += mass_roll(2, 5);
			if (cost < 51L) size += mass_roll(2, 5);
			if (cost < 501L) size += mass_roll(12, 5);
			break;
		}

		/*  Because many rods (and a few wands and staffs) are useful mainly
		 * in quantity, the Black Market will occasionally have a bunch of
		 * one kind. -LM- */
		case TV_ROD:
		case TV_WAND:
		case TV_STAFF:
		{
			if ((store_num == STORE_BLACKM) && (randint1(3) == 1))
			{
				if (cost < 1601L) size += mass_roll(1, 5);
				else if (cost < 3201L) size += mass_roll(1, 3);
			}
			/* Ensure that mass-produced rods and wands get the correct pvals. */
			if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
			{
				o_ptr->pval *= size;
			}
			break;
		}
	}


	/* Determine the discount probability modifier.   The purpose of this is to
	 * make shopkeepers with low purses offer more sales, as one might expect.
	 * -LM-
	 */

	/* Hack -- determine the maximum possible purse available at any one store. */
	if (store_num == STORE_GENERAL) max_purse = 450;
	if (store_num == STORE_ALCHEMY) max_purse = 15000;
	else max_purse = 30000;

	/* This to prevent integer math ruining accuracy. */
	temp = ot_ptr->max_cost / 10;

	/* Determine discount probability (inflated by 10) but stay reasonable. */
	discount_probability = max_purse / temp;
	if (discount_probability > 50) discount_probability = 50;


	/* Pick a discount.  Despite the changes in the formulas, average discount
	 * frequency hasn't changed much. */
	if (cost < 5)
	{
		discount = 0;
	}
	else if (randint0(400 / discount_probability) == 0)
	{
		discount = 25;
	}
	else if (randint0(2000 / discount_probability) == 0)
	{
		discount = 50;
	}
	else if (randint0(4000 / discount_probability) == 0)
	{
		discount = 75;
	}
	else if (randint0(6000 / discount_probability) == 0)
	{
		discount = 90;
	}


	/* Save the discount */
	o_ptr->discount = discount;

	/* Save the total pile size */
	o_ptr->number = size - (size * discount / 100);

	/* Hack -- rod/wands share the timeout/charges */
	if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
	{
		o_ptr->pval = o_ptr->number * k_info[o_ptr->k_idx].pval;
	}
}



/*
 * Convert a store item index into a one character label
 *
 * We use labels "a"-"l" for page 1, and labels "m"-"x" for page 2.
 */
static s16b store_to_label(int i)
{
	/* Assume legal */
	return (I2A(i));
}


/*
 * Convert a one character label into a store item index.
 *
 * Return "-1" if the label does not indicate a real store item.
 */
static s16b label_to_store(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i >= st_ptr->stock_num)) return (-1);

	/* Return the index */
	return (i);
}



/*
 * Determine if a store object can "absorb" another object
 *
 * See "object_similar()" for the same function for the "player"
 */
static bool store_object_similar(object_type *o_ptr, object_type *j_ptr)
{
	/* Hack -- Identical items cannot be stacked */
	if (o_ptr == j_ptr) return (0);

	/* Different objects cannot be stacked */
	if (o_ptr->k_idx != j_ptr->k_idx) return (0);

	/* Different charges (etc) cannot be stacked, unless wands or rods. */
	if ((o_ptr->pval != j_ptr->pval) && (o_ptr->tval != TV_WAND) && (o_ptr->tval != TV_ROD)) return (0);

	/* Require many identical values */
	if (o_ptr->to_h	 !=  j_ptr->to_h) return (0);
	if (o_ptr->to_d	 !=  j_ptr->to_d) return (0);
	if (o_ptr->to_a	 !=  j_ptr->to_a) return (0);

	/* Require identical "artifact" names */
	if (o_ptr->name1 != j_ptr->name1) return (0);

	/* Require identical "ego-item" names */
	if (o_ptr->name2 != j_ptr->name2) return (0);

	/* Hack -- Never stack "powerful" items */
	if (o_ptr->xtra1 || j_ptr->xtra1) return (0);

	/* Hack -- Never stack recharging items */
	if (o_ptr->timeout || j_ptr->timeout) return (0);

	/* Require many identical values */
	if (o_ptr->ac	 !=  j_ptr->ac)	  return (0);
	if (o_ptr->dd	 !=  j_ptr->dd)	  return (0);
	if (o_ptr->ds	 !=  j_ptr->ds)	  return (0);

	/* Hack -- Never stack chests */
	if (o_ptr->tval == TV_CHEST) return (0);

	/* Require matching discounts */
	if (o_ptr->discount != j_ptr->discount) return (0);

	/* They match, so they must be similar */
	return (TRUE);
}


/*
 * Allow a store object to absorb another object
 */
static void store_object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Combine quantity, lose excess items */
	o_ptr->number = (total > 99) ? 99 : total;

	/* Hack -- if rods are stacking, add the pvals (maximum timeouts) together.  */
	if (o_ptr->tval == TV_ROD)
	{
		o_ptr->pval += j_ptr->pval;
	}

	/* Hack -- if wands are stacking, combine the charges. */
	if (o_ptr->tval == TV_WAND)
	{
		o_ptr->pval += j_ptr->pval;
	}
}


/*
 * Check to see if the shop will be carrying too many objects
 *
 * Note that the shop, just like a player, will not accept things
 * it cannot hold.  Before, one could "nuke" objects this way, by
 * adding them to a pile which was already full.
 */
static bool store_check_num(struct store_type *store, object_type *o_ptr)
{
	int i;
	object_type *j_ptr;

	/* Free space is always usable */
	if (store->stock_num < store->stock_size) return TRUE;

	/* The "home" acts like the player */
	if (store->sidx == STORE_HOME)
	{
		/* Check all the objects */
		for (i = 0; i < store->stock_num; i++)
		{
			/* Get the existing object */
			j_ptr = &store->stock[i];

			/* Can the new object be combined with the old one? */
			if (object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* Normal stores do special stuff */
	else
	{
		/* Check all the objects */
		for (i = 0; i < store->stock_num; i++)
		{
			/* Get the existing object */
			j_ptr = &store->stock[i];

			/* Can the new object be combined with the old one? */
			if (store_object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* But there was no room at the inn... */
	return (FALSE);
}



/*
 * Add an object to the inventory of the Home.
 *
 * In all cases, return the slot (or -1) where the object was placed.
 *
 * Note that this is a hacked up version of "inven_carry()".
 *
 * Also note that it may not correctly "adapt" to "knowledge" becoming
 * known, the player may have to pick stuff up and drop it again.
 */
static int home_carry(object_type *o_ptr)
{
	int i, slot;
	s32b value, j_value;
	object_type *j_ptr;


	/* Check each existing object (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing object */
		j_ptr = &st_ptr->stock[slot];

		/* The home acts just like the player */
		if (object_similar(j_ptr, o_ptr))
		{
			/* Save the new number of items */
			object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


	/* Determine the "value" of the object */
	value = object_value(o_ptr);

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get that object */
		j_ptr = &st_ptr->stock[slot];

		/* Hack -- readable books always come first */
		if ((o_ptr->tval == mp_ptr->spell_book) &&
		    (j_ptr->tval != mp_ptr->spell_book)) break;
		if ((j_ptr->tval == mp_ptr->spell_book) &&
		    (o_ptr->tval != mp_ptr->spell_book)) continue;

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Can happen in the home */
		if (!object_aware_p(o_ptr)) continue;
		if (!object_aware_p(j_ptr)) break;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Objects in the home can be unknown */
		if (!object_known_p(o_ptr)) continue;
		if (!object_known_p(j_ptr)) break;

		/* Objects sort by decreasing value */
		j_value = object_value(j_ptr);
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		/* Hack -- slide the objects */
		object_copy(&st_ptr->stock[i], &st_ptr->stock[i-1]);
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Hack -- Insert the new object */
	object_copy(&st_ptr->stock[slot], o_ptr);

	/* Return the location */
	return (slot);
}


/*
 * Add an object to a real stores inventory.
 *
 * If the object is "worthless", it is thrown away (except in the home).
 *
 * If the object cannot be combined with an object already in the inventory,
 * make a new slot for it, and calculate its "per item" price.  Note that
 * this price will be negative, since the price will not be "fixed" yet.
 * Adding an object to a "fixed" price stack will not change the fixed price.
 *
 * In all cases, return the slot (or -1) where the object was placed
 */
static int store_carry(struct store_type *store, object_type *o_ptr)
{
	int i, slot;
	s32b value, j_value;
	object_type *j_ptr;


	/* Evaluate the object */
	value = object_value(o_ptr);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0) return (-1);

	/* Erase the feeling */
	o_ptr->feel = FEEL_NONE;

	/* Erase the inscription */
	o_ptr->note = 0;

	/* Check each existing object (try to combine) */
	for (slot = 0; slot < store->stock_num; slot++)
	{
		/* Get the existing object */
		j_ptr = &store->stock[slot];

		/* Can the existing items be incremented? */
		if (store_object_similar(j_ptr, o_ptr))
		{
			/* Hack -- extra items disappear */
			store_object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (store->stock_num >= store->stock_size) {
		return (-1);
	}

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < store->stock_num; slot++)
	{
		/* Get that object */
		j_ptr = &store->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Evaluate that slot */
		j_value = object_value(j_ptr);

		/* Objects sort by decreasing value */
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = store->stock_num; i > slot; i--)
	{
		/* Hack -- slide the objects */
		object_copy(&store->stock[i], &store->stock[i-1]);
	}

	/* More stuff now */
	store->stock_num++;

	/* Hack -- Insert the new object */
	object_copy(&store->stock[slot], o_ptr);

	/* Return the location */
	return (slot);
}


/*
 * Increase, by a 'num', the number of an item 'item' in store 'st'.
 * This can result in zero items.
 */
static void store_item_increase(struct store_type *store, int item, int num)
{
	int cnt;
	object_type *o_ptr;

	/* Get the object */
	o_ptr = &store->stock[item];

	/* Verify the number */
	cnt = o_ptr->number + num;
	if (cnt > 255) cnt = 255;
	else if (cnt < 0) cnt = 0;

	/* Save the new number */
	o_ptr->number = cnt;
}


/*
 * Remove a slot if it is empty, in store 'st'.
 */
static void store_item_optimize(struct store_type *store, int item)
{
	int j;
	object_type *o_ptr;

	/* Get the object */
	o_ptr = &store->stock[item];

	/* Must exist */
	if (!o_ptr->k_idx) return;

	/* Must have no items */
	if (o_ptr->number) return;

	/* One less object */
	store->stock_num--;

	/* Slide everyone */
	for (j = item; j < store->stock_num; j++)
	{
		store->stock[j] = store->stock[j + 1];
	}

	/* Nuke the final slot */
	object_wipe(&store->stock[j]);
}



/*
 * Delete an object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 */
static void store_delete_index(struct store_type *store, int what)
{
	int num;
	object_type *o_ptr;

	/* Paranoia */
	if (store->stock_num <= 0) return;

	/* Get the object */
	o_ptr = &store->stock[what];

	/* Determine how many objects are in the slot */
	num = o_ptr->number;

	/* Hack -- sometimes, only destroy half the objects */
	if (randint0(100) < 50) num = (num + 1) / 2;

	/* Hack -- sometimes, only destroy a single object, if not a missile. */
	if (randint0(100) < 50)
	{
		if ((o_ptr->tval != TV_BOLT) && (o_ptr->tval != TV_ARROW) && (o_ptr->tval != TV_SHOT)) num = 1;
	}

	/* Hack -- decrement the maximum timeouts and total charges of rods and wands. */
	if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
	{
		o_ptr->pval -= num * o_ptr->pval / o_ptr->number;

	}

	/* Delete the item */
	store_item_increase(store, what, -num);
	store_item_optimize(store, what);
}



/*
 * Delete a random object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 *
 * This function is used when store maintainance occurs, and is designed to
 * imitate non-PC purchasers making purchases from the store.
 */
static void store_delete_random(struct store_type *store)
{
	int what;

	if (store->stock_num <= 0) return;

	/* Pick a random slot */
	what = randint0(store->stock_num);

	store_delete_index(store, what);
}



/*
 * This function will keep 'crap' out of the black market.
 * Crap is defined as any object that is "available" elsewhere
 * Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
 */
static bool black_market_crap(object_type *o_ptr)
{
	int i, j;

	/* Ego items are never crap */
	if (o_ptr->name2) return (FALSE);

	/* Good items are never crap */
	if (o_ptr->to_a > 0) return (FALSE);
	if (o_ptr->to_h > 0) return (FALSE);
	if (o_ptr->to_d > 0) return (FALSE);

	/* Check the other stores */
	for (i = 0; i < MAX_STORES; i++)
	{
		/* Ignore the Home and the Black Market itself. */
		if (i == STORE_HOME) continue;
		if (i == STORE_BLACKM) continue;

		/* Check every object in the store */
		for (j = 0; j < stores[i].stock_num; j++)
		{
			object_type *j_ptr = &stores[i].stock[j];

			/* Duplicate object "type", assume crappy */
			if (o_ptr->k_idx == j_ptr->k_idx) return (TRUE);
		}
	}

	/* Assume okay */
	return (FALSE);
}



/*
 * Get a choice from the store allocation table, in tables.c
 */
static int store_get_choice(struct store_type *store)
{
	/* Choose a random entry from the store's table */
	int r = randint0(store->table_num);

	/* Return it */
	return store->table[r];
}


/*
 * Creates a random object and gives it to a store
 * This algorithm needs to be rethought.  A lot.
 * Currently, "normal" stores use a pre-built array.
 *
 * Note -- the "level" given to "obj_get_num()" is a "favored"
 * level, that is, there is a much higher chance of getting
 * items with a level approaching that of the given level...
 *
 * Should we check for "permission" to have the given object?
 */
static void store_create_random(struct store_type *store)
{
	int k_idx, tries, level;

	object_type *i_ptr;
	object_type object_type_body;

	/* Paranoia -- no room left */
	if (store->stock_num >= store->stock_size) return;


	/* Hack -- consider up to four items */
	for (tries = 0; tries < 4; tries++)
	{
		/* Black Market */
		if (store->sidx == STORE_BLACKM)
		{
			/* Pick a level for object/magic.  Now depends partly
			 * on player level.
			 */
			level = 5 + p_ptr->lev / 2 + randint0(30);

			/* Random object kind (biased towards given level) */
			k_idx = get_obj_num(level);

			/* Handle failure */
			if (!k_idx) continue;
		}

		/* Normal Store */
		else
		{
			/* Hack -- Pick an object kind to sell */
			k_idx = store_get_choice(store);

			/* Hack -- fake level for apply_magic() */
			level = rand_range(1, STORE_OBJ_LEVEL);
		}

		/*** Generate the item ***/

		/* Get local object */
		i_ptr = &object_type_body;

		/* Create a new object of the chosen kind */
		object_prep(i_ptr, k_idx);

		/* Apply some "low-level" magic (no artifacts) */
		apply_magic(i_ptr, level, FALSE, FALSE, FALSE);

		/* Hack -- Charge lite's */
		if (i_ptr->tval == TV_LITE)
		{
			if (i_ptr->sval == SV_LITE_TORCH) i_ptr->pval = FUEL_TORCH / 2;
			if (i_ptr->sval == SV_LITE_LANTERN) i_ptr->pval = FUEL_LAMP / 2;
		}

		/* The object is "known" */
		object_known(i_ptr);

		/* Mega-Hack -- no chests in stores */
		if (i_ptr->tval == TV_CHEST) continue;

		/* Prune the black market */
		if (store->sidx == STORE_BLACKM)
		{
			/* Hack -- No "crappy" items */
			if (black_market_crap(i_ptr)) continue;

			/* Hack -- No "cheap" items */
			if (object_value(i_ptr) < 10) continue;

			/* No "worthless" items */
			/* if (object_value(i_ptr) <= 0) continue; */
		}

		/* Prune normal stores */
		else
		{
			/* No "worthless" items */
			if (object_value(i_ptr) <= 0) continue;
		}


		/* Mass produce and/or apply discount */
		mass_produce(i_ptr);

		/* Attempt to carry the object */
		(void)store_carry(store, i_ptr);

		/* Definitely done */
		break;
	}
}



/*
 * Eliminate need to bargain if player has haggled well in the past
 */
static bool noneedtobargain(s32b minprice)
{
	s32b good = st_ptr->good_buy;
	s32b bad = st_ptr->bad_buy;

	/* Cheap items are "boring" */
	if (minprice < 10L) return (TRUE);

	/* Perfect haggling */
	if (good == MAX_SHORT) return (TRUE);

	/* Reward good haggles, punish bad haggles, notice price */
	if (good > ((3 * bad) + (5 + (minprice/50)))) return (TRUE);

	/* Return the flag */
	return (FALSE);
}


/*
 * Maintain the inventory at the stores.
 */
void store_maint(int which)
{
	int j;

	unsigned int stock;
	int old_rating = rating;

	/* Activate that store */
	store_type *store = &stores[which];

	/* Ignore home */
	if (store->sidx == STORE_HOME)
		return;

	/* Save the store index */
	store_num = which;

	/* Activate the owner */
	ot_ptr = &b_info[(store_num * MAX_B_IDX) + store->owner];


	/* Store keeper forgives the player */
	store->insult_cur = 0;

	/* Prune the black market */
	if (store->sidx == STORE_BLACKM)
	{
		/* Destroy crappy black market items */
		for (j = store->stock_num - 1; j >= 0; j--)
		{
			object_type *o_ptr = &store->stock[j];

			/* Destroy crappy items */
			if (black_market_crap(o_ptr))
			{
				/* Destroy the object */
				store_item_increase(store, j, 0 - o_ptr->number);
				store_item_optimize(store, j);
			}
		}
	}

	/*** "Sell" various items */

	/* Sell a few items */
	stock = store->stock_num;
	stock -= randint1(STORE_TURNOVER);

	/* Keep stock between specified min and max slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* Hack -- prevent "underflow" */
	if (stock < 0) stock = 0;

	/* Destroy objects until only "j" slots are left */
	while (store->stock_num > stock) store_delete_random(store);


	/*** "Buy in" various items */

	/* Buy a few items */
	stock = store->stock_num;
	stock += randint1(STORE_TURNOVER);

	/* Keep stock between specified min and max slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* Hack -- prevent "overflow" */
	if (stock >= store->stock_size) stock = store->stock_size - 1;

	/* Acquire some new items */
	while (store->stock_num < stock)
		store_create_random(store);


	/* Hack -- Restore the rating */
	rating = old_rating;
}


/*
 * Initialize the stores
 */
void store_init(int which)
{
	int k;


	/* Save the store index */
	store_num = which;

	/* Activate that store */
	st_ptr = &stores[store_num];


	/* Pick an owner */
	st_ptr->owner = (byte)randint0(MAX_B_IDX);

	/* Activate the new owner */
	ot_ptr = &b_info[(store_num * MAX_B_IDX) + st_ptr->owner];


	/* Initialize the store */
	st_ptr->store_open = 0;
	st_ptr->insult_cur = 0;
	st_ptr->good_buy = 0;
	st_ptr->bad_buy = 0;

	/* Nothing in stock */
	st_ptr->stock_num = 0;

	/* Clear any old items */
	for (k = 0; k < st_ptr->stock_size; k++)
	{
		object_wipe(&st_ptr->stock[k]);
	}
}



/*
 * Shuffle one of the stores.
 */
void store_shuffle(int which)
{
	int i, j;


	/* Ignore home */
	if (which == STORE_HOME) return;


	/* Save the store index */
	store_num = which;

	/* Activate that store */
	st_ptr = &stores[store_num];

	/* Pick a new owner */
	for (j = st_ptr->owner; j == st_ptr->owner; )
	{
		st_ptr->owner = (byte)randint0(MAX_B_IDX);
	}

	/* Activate the new owner */
	ot_ptr = &b_info[(store_num * MAX_B_IDX) + st_ptr->owner];

	/* Reset the owner data */
	st_ptr->insult_cur = 0;
	st_ptr->store_open = 0;
	st_ptr->good_buy = 0;
	st_ptr->bad_buy = 0;


	/* Hack -- discount all the items */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &st_ptr->stock[i];

		/* Sell all old items for "half price" */
		o_ptr->discount = 50;

		/* Clear the "fixed price" flag */
		o_ptr->ident &= ~(IDENT_FIXED);

		/* Inscribe the object as "on sale" */
		o_ptr->note = quark_add("on sale");
	}
}

/*** Display code ***/

/*
 * Redisplay a single store entry
 */
static void display_entry(int item)
{
	int y;
	object_type *o_ptr;
	s32b x;

	char o_name[120];
	char out_val[160];

	int maxwid = 75;


	/* Must be on current "page" to get displayed */
	if (!((item >= store_top) && (item < store_top + store_per))) return;


	/* Get the object */
	o_ptr = &st_ptr->stock[item];

	/* Get the row */
	y = (item % store_per) + 6;

	/* Label it */
	sprintf(out_val, "%c) ", store_to_label(item));

	/* Describe an object in the home */
	if (store_num == STORE_HOME)
	{
		byte attr;

		/* Print label, clear the line --(-- */
		if (o_ptr->ident & (IDENT_MENTAL))
		{
			c_prt(TERM_L_BLUE, out_val, y, 0);
		}
		else if ((object_known_p(o_ptr)) || (object_aware_p(o_ptr)))
		{
			prt(out_val, y, 0);
		}
		else
		{
			c_prt(TERM_L_WHITE, out_val, y, 0);
		}

		/* Available width */
		maxwid = 75;

		/* Leave room for weights, if necessary -DRS- */
		if (show_weights) maxwid -= 10;

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);
		o_name[maxwid] = '\0';

		/* Acquire inventory color.  Apply spellbook hack. */
		attr = proc_list_color_hack(o_ptr);

		/* Display the object */
		c_put_str(attr, o_name, y, 3);

		/* Show weights */
		if (show_weights)
		{
			/* Only show the weight of a single object */
			int wgt = o_ptr->weight;

			if (use_metric) sprintf(out_val, "%3d.%d kg", make_metric(wgt) / 10, make_metric(wgt) % 10);
			else sprintf(out_val, "%3d.%d lb", wgt / 10, wgt % 10);

			put_str(out_val, y, 67);
		}
	}

	/* Describe an object (fully) in a store */
	else
	{
		byte attr;

		/* Clear the line --(-- */
		prt(out_val, y, 0);

		/* Must leave room for the "price" */
		maxwid = 65;

		/* Leave room for weights, if necessary -DRS- */
		if (show_weights) maxwid -= 7;

		/* Describe the object (fully) */
		object_desc_store(o_name, o_ptr, TRUE, 3);
		o_name[maxwid] = '\0';

		/* Acquire inventory color.  Apply spellbook hack. */
		attr = proc_list_color_hack(o_ptr);

		/* Display the object */
		c_put_str(attr, o_name, y, 3);

		/* Show weights */
		if (show_weights)
		{
			/* Only show the weight of a single object */
			int wgt = o_ptr->weight;
			if (use_metric) sprintf(out_val, "%3d.%d kg", make_metric(wgt) / 10, make_metric(wgt) % 10);
			else sprintf(out_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(out_val, y, 61);
		}

		/* Display a "fixed" cost */
		if (o_ptr->ident & (IDENT_FIXED))
		{
			/* Extract the "minimum" price */
			x = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

			/* Actually draw the price (not fixed) */
			sprintf(out_val, "%9ld F", (long)x);
			put_str(out_val, y, 67);
		}

		/* Display a "taxed" cost */
		else
		{
			/* Extract the "minimum" price */
			x = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

			/* Hack -- Apply Sales Tax if needed */
			if (!noneedtobargain(x)) x += x / 10;

			/* Actually draw the price (with tax) */
			if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1)) sprintf(out_val, "%9ld avg", (long)x);
			else sprintf(out_val, "%9ld  ", (long)x);

			put_str(out_val, y, 67);
		}
	}
}


/*
 * Display a store's inventory
 *
 * All prices are listed as "per individual object"
 */
static void display_inventory(void)
{
	int i, k;

	/* Display the next store_per items */
	for (k = 0; k < store_per; k++)
	{
		/* Stop when we run out of items */
		if (store_top + k >= st_ptr->stock_num) break;

		/* Display that line */
		display_entry(store_top + k);
	}

	/* Erase the extra lines and the "more" prompt */
	for (i = k; i < store_per + 1; i++) prt("", i + 6, 0);

	/* Assume "no current page" */
	put_str("        ", 5, 20);

	/* Visual reminder of "more items" */
	if (st_ptr->stock_num > store_per)
	{
		/* Show "more" reminder (after the last object ) */
		prt("-more-", k + 6, 3);

		/* Indicate the "current page" */
		put_str(format("(Page %d)", store_top/store_per + 1), 5, 20);
	}
}


/*
 */
void display_home_inventory_remote(void)
{
	store_top = 0;
	store_num = STORE_HOME;
	st_ptr = &stores[store_num];

	/* Clear screen */
	Term_clear();

	/* Put the owner name */
	put_str("Your Home", 3, 30);

	/* No inventory */
	if (st_ptr->stock_num == 0)
	{
		put_str("There are no items in your home.", 5, 3);
	}

	/* There is an inventory */
	else
	{
		/* Label the object descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		if (show_weights)
		{
			put_str("Weight", 5, 70);
		}
	}

	while (TRUE)
	{
		int i, j, k;

		/* Display the next store_per items */
		for (j = 0; j < store_per; j++)
		{
			/* Stop when we run out of items */
			if (store_top + j >= st_ptr->stock_num) break;

			/* Display that line */
			display_entry(store_top + j);
		}

		/* Erase the extra lines and the "more" prompt */
		for (i = j; i < store_per + 1; i++) prt("", i + 6, 0);

		/* Assume "no current page" */
		if (st_ptr->stock_num > 0) put_str("        ", 5, 20);

		/* Visual reminder of "more items" */
		if (st_ptr->stock_num > store_per)
		{
			/* Show "more" reminder (after the last object ) */
			prt("-more-", j + 6, 3);

			/* Indicate the "current page" */
			put_str(format("(Page %d)", store_top / store_per + 1), 5, 20);
		}

		/* Get user directive */
		k = inkey();

		/* Exit */
		if (k == 'q') break;
		if (k == ESCAPE) break;

		/* View the rest of the list */
		else if (st_ptr->stock_num > store_per)
		{
			if (store_top < store_per) store_top = store_per;
			else store_top = 0;
		}
	}
}


/*
 * Display players gold
 */
static void store_prt_gold(void)
{
	char out_val[64];

	prt("Gold Remaining: ", 19, 53);

	sprintf(out_val, "%9ld", (long)p_ptr->au);
	prt(out_val, 19, 67);
}


/*
 * Display store (after clearing screen)
 */
static void display_store(void)
{
	char buf[80];


	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (store_num == STORE_HOME)
	{
		/* Put the owner name */
		put_str("Your Home", 3, 30);

		/* Label the object descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		if (show_weights)
		{
			put_str("Weight", 5, 70);
		}
	}

	/* Normal stores */
	else
	{

		const char * store_name = (f_name + f_info[FEAT_SHOP_HEAD + store_num].name);
		const char * owner_name = &(b_name[ot_ptr->owner_name]);
		const char * race_name = rp_name + rp_info[ot_ptr->owner_race].name;

		/* Put the owner name and race */
		sprintf(buf, "%s (%s)", owner_name, race_name);
		put_str(buf, 3, 10);

		/* Show the max price in the store (above prices) */
		sprintf(buf, "%s (%ld)", store_name, (long)(ot_ptr->max_cost));
		prt(buf, 3, 50);

		/* Label the object descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		if (show_weights)
		{
			put_str("Weight", 5, 60);
		}

		/* Label the asking price (in stores) */
		put_str("Price", 5, 71);

	}

	/* Display the current gold */
	store_prt_gold();

	/* Draw in the inventory */
	display_inventory();
}



/*
 * Get the index of a store object
 *
 * Return TRUE if an object was selected
 */
static bool get_stock(int *com_val, const char * pmt)
{
	int item;

	char which;

	char buf[160];

	char o_name[120];

	char out_val[160];

	object_type *o_ptr;

#ifdef ALLOW_REPEAT /* TNB */

	/* Get the item index */
	if (repeat_pull(com_val))
	{
		/* Verify the item */
		if ((*com_val >= 0) && (*com_val <= (st_ptr->stock_num - 1)))
		{
			/* Success */
			return (TRUE);
		}

		/* Invalid repeat - reset it */
		else repeat_clear();
	}
#endif /* ALLOW_REPEAT */

	/* Assume failure */
	*com_val = (-1);

	/* Build the prompt */
	sprintf(buf, "(Items %c-%c, ESC to exit) %s",
		store_to_label(0), store_to_label(st_ptr->stock_num - 1),
		pmt);

	/* Ask until done */
	while (TRUE)
	{
		int ver;

		/* Escape */
		if (!get_com(buf, &which)) return (FALSE);

		/* Extract "query" setting */
		ver = isupper(which);
		which = tolower(which);

		/* Convert response to item */
		item = label_to_store(which);

		/* Oops */
		if (item < 0)
		{
			/* Oops */
			bell("Illegal store object choice!");

			continue;
		}

		/* No verification */
		if (!ver) break;

		/* Object */
		o_ptr = &st_ptr->stock[item];

		/* Home */
		if (store_num == STORE_HOME)
		{
			/* Describe */
			object_desc(o_name, o_ptr, TRUE, 3);
		}

		/* Shop */
		else
		{
			/* Describe */
			object_desc_store(o_name, o_ptr, TRUE, 3);
		}

		/* Prompt */
		sprintf(out_val, "Try %s? ", o_name);

		/* Query */
		if (!get_check(out_val)) return (FALSE);

		/* Done */
		break;
	}

	/* Save item */
	(*com_val) = item;

#ifdef ALLOW_REPEAT /* TNB */

	repeat_push(*com_val);

#endif /* ALLOW_REPEAT */

	/* Success */
	return (TRUE);
}


/*
 * Increase the insult counter and get angry if necessary
 */
static int increase_insults(void)
{
	/* Increase insults */
	st_ptr->insult_cur++;

	/* Become insulted */
	if (st_ptr->insult_cur > ot_ptr->insult_max)
	{
		/* Complain */
		say_comment_4();

		/* Reset insults */
		st_ptr->insult_cur = 0;
		st_ptr->good_buy = 0;
		st_ptr->bad_buy = 0;

		/* Open tomorrow */
		st_ptr->store_open = turn + 25000 + randint1(25000);

		/* Closed */
		return (TRUE);
	}

	/* Not closed */
	return (FALSE);
}


/*
 * Decrease the insult counter
 */
static void decrease_insults(void)
{
	/* Decrease insults */
	if (st_ptr->insult_cur) st_ptr->insult_cur--;
}


/*
 * The shop-keeper has been insulted
 */
static int haggle_insults(void)
{
	/* Increase insults */
	if (increase_insults()) return (TRUE);

	/* Display and flush insult */
	say_comment_5();

	/* Still okay */
	return (FALSE);
}


/*
 * Mega-Hack -- Enable "increments"
 */
static bool allow_inc = FALSE;

/*
 * Mega-Hack -- Last "increment" during haggling
 */
static s32b last_inc = 0L;


/*
 * Get a haggle
 */
static int get_haggle(const char * pmt, s32b *poffer, s32b price, int final)
{
	char buf[128];


	/* Clear old increment if necessary */
	if (!allow_inc) last_inc = 0L;


	/* Final offer */
	if (final)
	{
		sprintf(buf, "%s [accept] ", pmt);
	}

	/* Old (negative) increment, and not final */
	else if (last_inc < 0)
	{
		sprintf(buf, "%s [-%ld] ", pmt, (long)(ABS(last_inc)));
	}

	/* Old (positive) increment, and not final */
	else if (last_inc > 0)
	{
		sprintf(buf, "%s [+%ld] ", pmt, (long)(ABS(last_inc)));
	}

	/* Normal haggle */
	else
	{
		sprintf(buf, "%s ", pmt);
	}


	/* Paranoia XXX XXX XXX */
	message_flush();


	/* Ask until done */
	while (TRUE)
	{
		const char * p;

		char out_val[81];

		/* Default */
		strcpy(out_val, "");

		/* Ask the user for a response */
		if (!get_string(buf, out_val, 81)) return (FALSE);

		/* Skip leading spaces */
		for (p = out_val; *p == ' '; p++) /* loop */;

		/* Empty response */
		if (*p == '\0')
		{
			/* Accept current price */
			if (final)
			{
				*poffer = price;
				last_inc = 0L;
				break;
			}

			/* Use previous increment */
			if (allow_inc && last_inc)
			{
				*poffer += last_inc;
				break;
			}
		}

		/* Normal response */
		else
		{
			s32b i;

			/* Extract a number */
			i = atol(p);

			/* Handle "incremental" number */
			if ((*p == '+' || *p == '-'))
			{
				/* Allow increments */
				if (allow_inc)
				{
					/* Use the given "increment" */
					*poffer += i;
					last_inc = i;
					break;
				}
			}

			/* Handle normal number */
			else
			{
				/* Use the given "number" */
				*poffer = i;
				last_inc = 0L;
				break;
			}
		}

		/* Warning */
		msg_print("Invalid response.");
		message_flush();
	}

	/* Success */
	return (TRUE);
}


/*
 * Receive an offer (from the player)
 *
 * Return TRUE if offer is NOT okay
 */
static bool receive_offer(const char * pmt, s32b *poffer,
			  s32b last_offer, int factor,
			  s32b price, int final)
{
	/* Haggle till done */
	while (TRUE)
	{
		/* Get a haggle (or cancel) */
		if (!get_haggle(pmt, poffer, price, final)) return (TRUE);

		/* Acceptable offer */
		if (((*poffer) * factor) >= (last_offer * factor)) break;

		/* Insult, and check for kicked out */
		if (haggle_insults()) return (TRUE);

		/* Reject offer (correctly) */
		(*poffer) = last_offer;
	}

	/* Success */
	return (FALSE);
}


/*
 * Haggling routine XXX XXX
 *
 * Return TRUE if purchase is NOT successful
 */
static bool purchase_haggle(object_type *o_ptr, s32b *price)
{
	s32b cur_ask, final_ask;
	s32b last_offer, offer;
	s32b x1, x2, x3;
	s32b min_per, max_per;
	int flag, loop_flag, noneed;
	int annoyed = 0, final = FALSE;

	bool ignore = FALSE;

	bool cancel = FALSE;

	const char * pmt = "Asking";

	char out_val[160];


	*price = 0;


	/* Extract the starting offer and final offer */
	cur_ask = price_item(o_ptr, ot_ptr->max_inflate, FALSE);
	final_ask = price_item(o_ptr, ot_ptr->min_inflate, FALSE);

	/* Determine if haggling is necessary */
	noneed = noneedtobargain(final_ask);

	/* Already haggled */
	if (o_ptr->ident & (IDENT_FIXED))
	{
		/* Message summary */
		msg_print("You instantly agree upon the price.");
		message_flush();
	}

	/* No need to haggle */
	else if (noneed)
	{
		/* Message summary */
		msg_print("You eventually agree upon the price.");
		message_flush();
	}

	/* Auto-haggle */
	else
	{
		/* Message summary */
		msg_print("You quickly agree upon the price.");
		message_flush();

		/* Ignore haggling */
		ignore = TRUE;

		/* Apply sales tax */
		final_ask += (final_ask / 10);
	}

	/* Jump to final price */
	cur_ask = final_ask;

	/* Go to final offer */
	pmt = "Final Offer";
	final = TRUE;


	/* Haggle for the whole pile */
	cur_ask *= o_ptr->number;
	final_ask *= o_ptr->number;


	/* Haggle parameters */
	min_per = ot_ptr->haggle_per;
	max_per = min_per * 3;

	/* Mega-Hack -- artificial "last offer" value */
	last_offer = object_value(o_ptr) * o_ptr->number;
	last_offer = last_offer * (200 - (int)(ot_ptr->max_inflate)) / 100L;
	if (last_offer <= 0) last_offer = 1;

	/* No offer yet */
	offer = 0;

	/* No incremental haggling yet */
	allow_inc = FALSE;

	/* Haggle until done */
	for (flag = FALSE; !flag; )
	{
		loop_flag = TRUE;

		while (!flag && loop_flag)
		{
			sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
			put_str(out_val, 1, 0);
			cancel = receive_offer("What do you offer? ",
					       &offer, last_offer, 1, cur_ask, final);

			if (cancel)
			{
				flag = TRUE;
			}
			else if (offer > cur_ask)
			{
				say_comment_6();
				offer = last_offer;
			}
			else if (offer == cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}
			else
			{
				loop_flag = FALSE;
			}
		}

		if (!flag)
		{
			x1 = 100 * (offer - last_offer) / (cur_ask - last_offer);
			if (x1 < min_per)
			{
				if (haggle_insults())
				{
					flag = TRUE;
					cancel = TRUE;
				}
			}
			else if (x1 > max_per)
			{
				x1 = x1 * 3 / 4;
				if (x1 < max_per) x1 = max_per;
			}
			x2 = rand_range(x1-2, x1+2);
			x3 = ((cur_ask - offer) * x2 / 100L) + 1;
			/* don't let the price go up */
			if (x3 < 0) x3 = 0;
			cur_ask -= x3;

			/* Too little */
			if (cur_ask < final_ask)
			{
				final = TRUE;
				cur_ask = final_ask;
				pmt = "Final Offer";
				annoyed++;
				if (annoyed > 3)
				{
					(void)(increase_insults());
					cancel = TRUE;
					flag = TRUE;
				}
			}
			else if (offer >= cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}

			if (!flag)
			{
				last_offer = offer;
				allow_inc = TRUE;
				prt("", 1, 0);
				sprintf(out_val, "Your last offer: %ld",
					      (long)last_offer);
				put_str(out_val, 1, 39);
				say_comment_2(cur_ask, annoyed);
			}
		}
	}

	/* Mark price as fixed */
	if (!ignore && (*price == final_ask))
	{
		/* Mark as fixed price */
		o_ptr->ident |= (IDENT_FIXED);
	}

	/* Cancel */
	if (cancel) return (TRUE);

	/* Do not cancel */
	return (FALSE);
}


/*
 * Haggling routine XXX XXX
 *
 * Return TRUE if purchase is NOT successful
 */
static bool sell_haggle(object_type *o_ptr, s32b *price)
{
	s32b purse, cur_ask, final_ask;
	s32b last_offer = 0, offer = 0;
	s32b x1, x2, x3;
	s32b min_per, max_per;

	int flag, loop_flag, noneed;
	int annoyed = 0, final = FALSE;

	bool ignore = FALSE;

	bool cancel = FALSE;

	const char * pmt = "Offer";

	char out_val[160];


	*price = 0;

	/* Objects that the player makes himself are heavily discounted.  Let
	 * him know this. -LM-
	 */
	if (o_ptr->discount == 80)
		msg_print("This item looks homemade.  I cannot offer much for it.");

	/* Obtain the starting offer and the final offer */
	cur_ask = price_item(o_ptr, ot_ptr->max_inflate, TRUE);
	final_ask = price_item(o_ptr, ot_ptr->min_inflate, TRUE);

	/* Determine if haggling is necessary */
	noneed = noneedtobargain(final_ask);

	/* Get the owner's payout limit */
	purse = (s32b)(ot_ptr->max_cost);

	/* No reason to haggle */
	if (final_ask >= purse)
	{
		/* Message */
		msg_print("You instantly agree upon the price.");
		message_flush();

		/* Ignore haggling */
		ignore = TRUE;

		/* Offer full purse */
		final_ask = purse;
	}

	/* No need to haggle */
	else if (noneed)
	{
		/* Message */
		msg_print("You eventually agree upon the price.");
		message_flush();
	}

	/* No haggle option */
	else
	{
		/* Message summary */
		msg_print("You quickly agree upon the price.");
		message_flush();

		/* Ignore haggling */
		ignore = TRUE;

		/* Apply Sales Tax */
		final_ask -= final_ask / 10;
	}

	/* Final price */
	cur_ask = final_ask;

	/* Final offer */
	final = TRUE;
	pmt = "Final Offer";

	/* Haggle for the whole pile */
	cur_ask *= o_ptr->number;
	final_ask *= o_ptr->number;


	/* Display commands XXX XXX XXX */

	/* Haggling parameters */
	min_per = ot_ptr->haggle_per;
	max_per = min_per * 3;

	/* Mega-Hack -- artificial "last offer" value */
	last_offer = object_value(o_ptr) * o_ptr->number;
	last_offer = last_offer * ot_ptr->max_inflate / 100L;

	/* No offer yet */
	offer = 0;

	/* No incremental haggling yet */
	allow_inc = FALSE;

	/* Haggle */
	for (flag = FALSE; !flag; )
	{
		while (1)
		{
			loop_flag = TRUE;

			sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
			put_str(out_val, 1, 0);
			cancel = receive_offer("What price do you ask? ",
					       &offer, last_offer, -1, cur_ask, final);

			if (cancel)
			{
				flag = TRUE;
			}
			else if (offer < cur_ask)
			{
				say_comment_6();
				/* rejected, reset offer for incremental haggling */
				offer = last_offer;
			}
			else if (offer == cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}
			else
			{
				loop_flag = FALSE;
			}

			/* Stop */
			if (flag || !loop_flag) break;
		}

		if (!flag)
		{
			x1 = 100 * (last_offer - offer) / (last_offer - cur_ask);
			if (x1 < min_per)
			{
				if (haggle_insults())
				{
					flag = TRUE;
					cancel = TRUE;
				}
			}
			else if (x1 > max_per)
			{
				x1 = x1 * 3 / 4;
				if (x1 < max_per) x1 = max_per;
			}
			x2 = rand_range(x1-2, x1+2);
			x3 = ((offer - cur_ask) * x2 / 100L) + 1;
			/* don't let the price go down */
			if (x3 < 0) x3 = 0;
			cur_ask += x3;

			if (cur_ask > final_ask)
			{
				cur_ask = final_ask;
				final = TRUE;
				pmt = "Final Offer";
				annoyed++;
				if (annoyed > 3)
				{
					flag = TRUE;
					(void)(increase_insults());
				}
			}
			else if (offer <= cur_ask)
			{
				flag = TRUE;
				*price = offer;
			}

			if (!flag)
			{
				last_offer = offer;
				allow_inc = TRUE;
				prt("", 1, 0);
				sprintf(out_val,
					      "Your last bid %ld", (long)last_offer);
				put_str(out_val, 1, 39);
				say_comment_3(cur_ask, annoyed);
			}
		}
	}

	/* Cancel */
	if (cancel) return (TRUE);

	/* Do not cancel */
	return (FALSE);
}





/*
 * Buy an object from a store
 */
static void store_purchase(void)
{
	int n;
	int amt, choice;
	int item, item_new;

	s32b price;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	char o_name[120];

	char out_val[160];


	/* Empty? */
	if (st_ptr->stock_num <= 0)
	{
		if (store_num == STORE_HOME)
		{
			msg_print("Your home is empty.");
		}
		else
		{
			msg_print("I am currently out of stock.");
		}
		return;
	}


	/* Prompt */
	if (store_num == STORE_HOME)
	{
		sprintf(out_val, "Which item do you want to take? ");
	}
	else
	{
		sprintf(out_val, "Which item are you interested in? ");
	}

	/* Get the object number to be bought */
	if (!get_stock(&item, out_val)) return;

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);

	/* Allow user abort */
	if (amt <= 0) return;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Get desired object */
	object_copy(i_ptr, o_ptr);

	/* Hack -- If a rod or wand, allocate total maximum timeouts or charges
	 * between those purchased and left on the shelf.
	 */
	reduce_charges(i_ptr, i_ptr->number - amt);

	/* Modify quantity */
	i_ptr->number = amt;

	/* Hack -- require room in pack */
	if (!inven_carry_okay(i_ptr))
	{
		msg_print("You cannot carry that many items.");
		return;
	}

	/* Attempt to buy it */
	if (store_num != STORE_HOME)
	{
		/* Describe the object (fully) */
		object_desc_store(o_name, i_ptr, TRUE, 3);

		/* Message */
		msg("Buying %s (%c).",
			   o_name, store_to_label(item));
		message_flush();

		/* Haggle for a final price */
		choice = purchase_haggle(i_ptr, &price);

		/* Hack -- Got kicked out */
		if (st_ptr->store_open >= turn) return;

		/* Hack -- Maintain fixed prices */
		if (i_ptr->ident & (IDENT_FIXED))
		{
			/* Mark as fixed price */
			o_ptr->ident |= (IDENT_FIXED);
		}

		/* Player wants it */
		if (choice == 0)
		{
			/* Player can afford it */
			if (p_ptr->au >= price)
			{
				/* Say "okay" */
				say_comment_1();

				/* Be happy */
				decrease_insults();

				/* Spend the money */
				p_ptr->au -= price;

				/* Update the display */
				store_prt_gold();

				/* Buying an object makes you aware of it */
				object_aware(i_ptr);

				/* Combine / Reorder the pack (later) */
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);

				/* Clear the "fixed" flag from the object */
				i_ptr->ident &= ~(IDENT_FIXED);

				/* Describe the transaction */
				object_desc(o_name, i_ptr, TRUE, 3);

				/* Message */
				msg("You bought %s (%c) for %ld gold.",
					   o_name, store_to_label(item),
					   (long)price);

				/* Erase the feeling */
				i_ptr->feel = FEEL_NONE;

				/* Erase the inscription */
				i_ptr->note = 0;

				/* Give it to the player */
				item_new = inven_carry(i_ptr);

				/* Describe the final result */
				object_desc(o_name, &inventory[item_new], TRUE, 3);

				/* Message */
				msg("You have %s (%c).",
					   o_name, index_to_label(item_new));

				/* Now, reduce the original stack's pval. */
				if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
				{
					o_ptr->pval -= i_ptr->pval;
				}


				/* Handle stuff */
				handle_stuff();

				/* Note how many slots the store used to have */
				n = st_ptr->stock_num;

				/* Remove the bought objects from the store */
				store_item_increase(st_ptr, item, -amt);
				store_item_optimize(st_ptr, item);

				/* Store is empty */
				if (st_ptr->stock_num == 0)
				{
					int i;

					/* Shuffle.  Made retiring more likely. */
					if (randint0(STORE_SHUFFLE) < 5)
					{
						/* Message */
						msg_print("The shopkeeper retires.");

						/* Shuffle the store */
						store_shuffle(store_num);
					}

					/* Maintain */
					else
					{
						/* Message */
						msg_print("The shopkeeper brings out some new stock.");
					}

					/* New inventory */
					for (i = 0; i < 10; ++i)
					{
						/* Maintain the store */
						store_maint(store_num);
					}

					/* Start over */
					store_top = 0;

					/* Redraw everything */
					display_inventory();
				}

				/* The object is gone */
				else if (st_ptr->stock_num != n)
				{
					/* Only one screen left */
					if (st_ptr->stock_num <= store_per)
					{
						store_top = 0;
					}

					/* Redraw everything */
					display_inventory();
				}

				/* The object is still here */
				else
				{
					/* Redraw the object */
					display_entry(item);
				}
			}

			/* Player cannot afford it */
			else
			{
				/* Simple message (no insult) */
				msg_print("You do not have enough gold.");
			}
		}
	}

	/* Home is much easier */
	else
	{
		/* Distribute charges of wands or rods */
		distribute_charges(o_ptr, i_ptr, amt);

		/* Give it to the player */
		item_new = inven_carry(i_ptr);

		/* Describe just the result */
		object_desc(o_name, &inventory[item_new], TRUE, 3);

		/* Message */
		msg("You have %s (%c).", o_name, index_to_label(item_new));

		/* Handle stuff */
		handle_stuff();

		/* Take note if we take the last one */
		n = st_ptr->stock_num;

		/* Remove the items from the home */
		store_item_increase(st_ptr, item, -amt);
		store_item_optimize(st_ptr, item);

		/* The object is gone */
		if (st_ptr->stock_num != n)
		{
			/* Only one screen left */
			if (st_ptr->stock_num <= store_per)
			{
				store_top = 0;
			}

			/* Redraw everything */
			display_inventory();
		}

		/* The object is still here */
		else
		{
			/* Redraw the object */
			display_entry(item);
		}
	}

	/* Not kicked out */
	return;
}


/*
 * Sell an object to the store (or home)
 */
static void store_sell(void)
{
	int choice;
	int item, item_pos;
	int amt;

	s32b price, value, dummy;

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	const char * q, *s;

	char o_name[120];


	/* Home */
	q = "Drop which item? ";

	/* Real store */
	if (store_num != STORE_HOME)
	{
		/* New prompt */
		q = "Sell which item? ";

		/* Only allow items the store will buy */
		item_tester_hook = store_will_buy;
	}

	/* Start off in equipment mode. */
	p_ptr->command_wrk = (USE_INVEN);

	/* Get an item */
	s = "You have nothing that I want.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}


	/* Hack -- Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);

	/* Allow user abort */
	if (amt <= 0) return;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Get a copy of the object */
	object_copy(i_ptr, o_ptr);

	/* Modify quantity */
	i_ptr->number = amt;

	/* Hack -- If a rod or wand, allocate total maximum
	 * timeouts or charges to those being sold.
	 */
	if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
	{
		i_ptr->pval = o_ptr->pval * amt / o_ptr->number;
	}

	/* Get a full description */
	object_desc(o_name, i_ptr, TRUE, 3);

	/* Is there room in the store (or the home?) */
	if (!store_check_num(st_ptr, i_ptr))
	{
		if (store_num == STORE_HOME)
		{
			msg_print("Your home is full.");
		}
		else
		{
			msg_print("I have not the room in my store to keep it.");
		}
		return;
	}


	/* Real store */
	if (store_num != STORE_HOME)
	{
		/* Describe the transaction */
		msg("Selling %s (%c).", o_name, index_to_label(item));
		message_flush();

		/* Haggle for it */
		choice = sell_haggle(i_ptr, &price);

		/* Kicked out */
		if (st_ptr->store_open >= turn) return;

		/* Sold... */
		if (choice == 0)
		{
			/* Say "okay" */
			say_comment_1();

			/* Be happy */
			decrease_insults();

			/* Get some money */
			p_ptr->au += price;

			/* Update the display */
			store_prt_gold();

			/* Get the "apparent" value */
			dummy = object_value(i_ptr) * i_ptr->number;

			/* Identify original object */
			object_aware(o_ptr);
			object_known(o_ptr);

			/* Erase the feeling */
			i_ptr->feel = FEEL_NONE;

			/* Remove any inscription */
			i_ptr->note = 0;

			/* Combine / Reorder the pack (later) */
			p_ptr->notice |= (PN_COMBINE | PN_REORDER);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);

			/* Get local object */
			i_ptr = &object_type_body;

			/* Get a copy of the object */
			object_copy(i_ptr, o_ptr);

			/* Modify quantity */
			i_ptr->number = amt;

			/* Hack -- If a rod or wand, let the shopkeeper know just
			 * how many charges he really paid for.
			 */
			if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_WAND))
			{
				i_ptr->pval = o_ptr->pval * amt / o_ptr->number;
			}

			/* Get the "actual" value */
			value = object_value(i_ptr) * i_ptr->number;

			/* Get the description all over again */
			object_desc(o_name, i_ptr, TRUE, 3);

			/* Describe the result (in message buffer) */
			msg("You sold %s (%c) for %ld gold.",
				   o_name, index_to_label(item), (long)price);

			/* Analyze the prices (and comment verbally) */
			purchase_analyze(price, value, dummy);


			/* Hack -- Allocate charges between those wands or rods sold
			 * and retained, unless all are being sold.
			 */
			distribute_charges(o_ptr, i_ptr, amt);

			/* Take the object from the player */
			inven_item_increase(item, -amt);
			inven_item_describe(item);
			inven_item_optimize(item);

			if (item == INVEN_WIELD)
				p_ptr->shield_on_back = FALSE;


			/* Handle stuff */
			handle_stuff();

#if 0
			/* Take note if we add a new item */
			n = st_ptr->stock_num;
#endif

			/* The store gets that (known) object */
			item_pos = store_carry(st_ptr, i_ptr);

			/* Update the display */
			if (item_pos >= 0)
			{
				/* Redisplay wares */
				display_inventory();
			}
		}
	}

	/* Player is at home */
	else
	{
		/* Distribute charges of wands/rods */
		distribute_charges(o_ptr, i_ptr, amt);

		/* Describe */
		msg("You drop %s (%c).", o_name, index_to_label(item));


		/* Take it from the players inventory */
		inven_item_increase(item, -amt);
		inven_item_describe(item);
		inven_item_optimize(item);

		if (item == INVEN_WIELD)
			p_ptr->shield_on_back = FALSE;


		/* Handle stuff */
		handle_stuff();

#if 0
		/* Take note if we add a new item */
		n = st_ptr->stock_num;
#endif

		/* Let the home carry it */
		item_pos = home_carry(i_ptr);

		/* Update store display */
		if (item_pos >= 0)
		{
			/* Redisplay wares */
			display_inventory();
		}
	}
}


/*
 * Get an object in a store to inspect.   Original code by -JDL- (from Zangband).
 */
static void store_inspect(void)
{
	int	    item;

	object_type *o_ptr;

	char	    out_val[160];


	/* Empty? */
	if (st_ptr->stock_num <= 0)
	{
		if (store_num == STORE_HOME)
			msg_print("Your home is empty.");
		else
			msg_print("I am currently out of stock.");
		return;
	}

	/* Prompt */
	sprintf(out_val, "Examine which item? ");

	/* Get the item number to be examined */
	if (!get_stock(&item, out_val)) return;

	/* Get the actual item */
	o_ptr = &st_ptr->stock[item];

	/* Examine the item. */
	do_cmd_observe(o_ptr, (store_num == STORE_HOME ? FALSE : TRUE));
}


/*
 * Hack -- set this to leave the store
 */
static bool leave_store = FALSE;


/*
 * Process a command in a store
 *
 * Note that we must allow the use of a few "special" commands
 * in the stores which are not allowed in the dungeon, and we
 * must disable some commands which are allowed in the dungeon
 * but not in the stores, to prevent chaos.
 *
 * Hack -- note the bizarre code to handle the "=" command,
 * which is needed to prevent the "redraw" from affecting
 * the display of the store.  XXX XXX XXX
 */
static void store_process_command(void)
{
#ifdef ALLOW_REPEAT /* TNB */

    /* Handle repeating the last command */
    repeat_check();

#endif /* ALLOW_REPEAT */

	/* Parse the command */
	switch (p_ptr->command_cmd)
	{
			/* Leave */
		case ESCAPE:
		{
			leave_store = TRUE;
			break;
		}

			/* Browse */
		case ' ':
		{
			if (st_ptr->stock_num <= store_per)
			{
				/* Nothing to see */
				msg_print("Entire inventory is shown.");
			}

			else if (store_top == 0)
			{
				/* Page 2 */
				store_top = store_per;

				/* Redisplay wares */
				display_inventory();
			}

			else
			{
				/* Page 1 */
				store_top = 0;

				/* Redisplay wares */
				display_inventory();
			}

			break;
		}

			/* Redraw */
		case KTRL('R'):
		{
			do_cmd_redraw();
			display_store();
			break;
		}

			/* Get (purchase) */
		case 'g':
		{
			store_purchase();
			break;
		}

			/* Drop (Sell) */
		case 'd':
		{
			store_sell();
			break;
		}

			/* Ignore return */
		case '\r':
		{
			break;
		}



			/*** Inventory Commands ***/

			/* Wear/wield equipment */
		case 'w':
		{
			do_cmd_wield();
			break;
		}

			/* Take off equipment */
		case 't':
		{
			do_cmd_takeoff();
			break;
		}

#if 0

			/* Drop an item */
		case 'd':
		{
			do_cmd_drop();
			break;
		}

#endif

			/* Destroy an item */
		case 'k':
		{
			do_cmd_destroy();
			break;
		}

			/* Equipment list */
		case 'e':
		{
			do_cmd_equip();
			break;
		}

			/* Inventory list */
		case 'i':
		{
			do_cmd_inven();
			break;
		}


			/*** Various commands ***/

			/* Identify an object */
		case 'I':
		{
			store_inspect();
			break;
		}

			/* Hack -- toggle windows */
		case KTRL('E'):
		{
			toggle_inven_equip();
			break;
		}



			/*** Use various objects ***/

			/* Browse a book */
		case 'b':
		{
			do_cmd_browse();
			break;
		}

			/* Inscribe an object */
		case '{':
		{
			do_cmd_inscribe();
			break;
		}

			/* Uninscribe an object */
		case '}':
		{
			do_cmd_uninscribe();
			break;
		}



			/*** Help and Such ***/

			/* Help */
		case '?':
		{
			do_cmd_help();
			break;
		}

			/* Identify symbol */
		case '/':
		{
			do_cmd_query_symbol();
			break;
		}

			/* Character description */
		case 'C':
		{
			do_cmd_change_name();
			break;
		}


			/*** System Commands ***/

			/* Single line from a pref file */
		case '"':
		{
			do_cmd_pref();
			break;
		}

			/* Interact with macros */
		case '@':
		{
			do_cmd_macros();
			break;
		}

			/* Interact with visuals */
		case '%':
		{
			do_cmd_visuals();
			break;
		}

			/* Interact with colors */
		case '&':
		{
			do_cmd_colors();
			break;
		}

			/* Interact with options */
		case '=':
		{
			do_cmd_options();
			do_cmd_redraw();
			display_store();
			break;
		}


			/*** Misc Commands ***/

			/* Take notes */
		case ':':
		{
			do_cmd_note();
			break;
		}

			/* Version info */
		case 'V':
		{
			do_cmd_version();
			break;
		}

			/* Repeat level feeling */
		case KTRL('F'):
		{
			do_cmd_feeling();
			break;
		}

			/* Show previous message */
		case KTRL('O'):
		{
			do_cmd_message_one();
			break;
		}

			/* Show previous messages */
		case KTRL('P'):
		{
			do_cmd_messages();
			break;
		}

			/* Check knowledge */
		case '~':
		case '|':
		{
			do_cmd_knowledge();
			break;
		}

			/* Load "screen dump" */
		case '(':
		{
			do_cmd_load_screen();
			break;
		}

			/* Save "screen dump" */
		case ')':
		{
			do_cmd_save_screen();
			break;
		}


			/* Hack -- Unknown command */
		default:
		{
			msg_print("That command does not work in stores.");
			break;
		}
	}
}


/*
 * Enter a store, and interact with it.
 *
 * Note that we use the standard "request_command()" function
 * to get a command, allowing us to use "p_ptr->command_arg" and all
 * command macros and other nifty stuff, but we use the special
 * "shopping" argument, to force certain commands to be converted
 * into other commands, normally, we convert "p" (pray) and "m"
 * (cast magic) into "g" (get), and "s" (search) into "d" (drop).
 */
void do_cmd_store(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int which;

	int tmp_chr;

	/* Verify a store */
	if (!((cave_feat[py][px] >= FEAT_SHOP_HEAD) &&
	      (cave_feat[py][px] <= FEAT_SHOP_TAIL)))
	{
		msg_print("You see no store here.");
		return;
	}

	/* Hack -- Extract the store code */
	which = (cave_feat[py][px] - FEAT_SHOP_HEAD);

	/* Hack -- Check the "locked doors" */
	if (stores[which].store_open >= turn)
	{
		msg_print("The doors are locked.");
		return;
	}


	/* Forget the view */
	forget_view();


	/* Hack -- Increase "icky" depth */
	character_icky++;


	/* No command argument */
	p_ptr->command_arg = 0;

	/* No repeated command */
	p_ptr->command_rep = 0;

	/* No automatic command */
	p_ptr->command_new = 0;


	/* Save the store number */
	store_num = which;

	/* Save the store and owner pointers */
	st_ptr = &stores[store_num];
	ot_ptr = &b_info[(store_num * MAX_B_IDX) + st_ptr->owner];

	/* Start at the beginning */
	store_top = 0;

	/* Display the store */
	display_store();

	/* Do not leave */
	leave_store = FALSE;

	/* Interact with player */
	while (!leave_store)
	{
		/* Hack -- Clear line 1 */
		prt("", 1, 0);

		/* Hack -- Check the charisma */
		tmp_chr = p_ptr->stat_use[A_CHR];

		/* Clear */
		clear_from(21);

		/* Basic commands */
		prt(" ESC) Exit from Building.", 22, 0);

		/* Browse if necessary */
		if (st_ptr->stock_num > 12)
		{
			prt(" SPACE) Next page of stock", 23, 0);
		}

		/* Commands */
		prt(" g) Get/Purchase an item.", 22, 29);
		prt(" d) Drop/Sell an item.", 23, 29);

		prt("   I) Inspect an item.", 22, 55);

		/* Prompt */
		prt("You may: ", 21, 0);

		/* Get a command */
		request_command(TRUE);

		/* Process the command */
		store_process_command();

		/* Notice stuff */
		notice_stuff();

		/* Handle stuff */
		handle_stuff();

		/* Pack Overflow XXX XXX XXX */
		if (inventory[INVEN_PACK].k_idx)
		{
			int item = INVEN_PACK;

			object_type *o_ptr = &inventory[item];

			/* Hack -- Flee from the store */
			if (store_num != STORE_HOME)
			{
				/* Message */
				msg_print("Your pack is so full that you flee the store...");

				/* Leave */
				leave_store = TRUE;
			}

			/* Hack -- Flee from the home */
			else if (!store_check_num(st_ptr, o_ptr))
			{
				/* Message */
				msg_print("Your pack is so full that you flee your home...");

				/* Leave */
				leave_store = TRUE;
			}

			/* Hack -- Drop items into the home */
			else
			{
				int item_pos;

				object_type *i_ptr;
				object_type object_type_body;

				char o_name[120];


				/* Give a message */
				msg_print("Your pack overflows!");

				/* Get local object */
				i_ptr = &object_type_body;

				/* Grab a copy of the object */
				object_copy(i_ptr, o_ptr);

				/* Describe it */
				object_desc(o_name, i_ptr, TRUE, 3);

				/* Message */
				msg("You drop %s (%c).", o_name, index_to_label(item));

				/* Remove it from the players inventory */
				inven_item_increase(item, -255);
				inven_item_describe(item);
				inven_item_optimize(item);

				/* Handle stuff */
				handle_stuff();

#if 0
				/* Take note if we add a new item */
				n = st_ptr->stock_num;
#endif
				/* Let the home carry it */
				item_pos = home_carry(i_ptr);

				/* Redraw the home */
				if (item_pos >= 0)
				{
					/* Redisplay wares */
					display_inventory();
				}
			}
		}

		/* Hack -- Handle charisma changes */
		if (tmp_chr != p_ptr->stat_use[A_CHR])
		{
			/* Redisplay wares */
			display_inventory();
		}

		/* Hack -- get kicked out of the store */
		if (st_ptr->store_open >= turn) leave_store = TRUE;
	}


	/* Free turn XXX XXX XXX */
	p_ptr->energy_use = 0;


	/* Hack -- Cancel automatic command */
	p_ptr->command_new = 0;

	/* Hack -- Cancel "see" mode */
	p_ptr->command_see = FALSE;


	/* Flush messages XXX XXX XXX */
	message_flush();


	/* Hack -- Decrease "icky" depth */
	character_icky--;


	/* Clear the screen */
	Term_clear();


	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



