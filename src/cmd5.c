/* File: cmd5.c */

/* Warrior probing.  Selection, browsing, learning, and casting of spells
 * and prayers.  Includes definitions of all spells and prayers.  Shape-
 * shifting and making Athelas.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * Warriors will eventually learn to pseudo-probe monsters.  If they use
 * the browse command, give ability information. -LM-
 */
static void warrior_probe_desc(void)
{
	/* Save screen */
	screen_save();

	/* Erase the screen */
	Term_clear();

	/* Title in light blue. */
	c_roff(TERM_L_BLUE, "Warrior Pseudo-Probing Ability:", 5, 75);
	roff("\n", 0, 0); roff("\n", 0, 0);

	/* Print out information text. */
	roff("     Warriors learn to probe monsters at level 35.  This costs nothing except a full turn.  When you reach this level, type 'm', and target the monster you would like to learn more about.  This reveals the monster's race, approximate HPs, and basic resistances.  Be warned:  the information given is not always complete...", 5, 75);
	roff("\n", 0, 0); roff("\n", 0, 0); roff("\n", 0, 0);

	/* The "exit" sign. */
	roff("    (Press any key to continue.)", 5, 75);
	roff("\n", 0, 0);

	/* Wait for it. */
	(void)inkey();

	/* Load screen */
	screen_load();
}

/*
 * Warriors will eventually learn to pseudo-probe monsters.  This allows
 * them to better choose between slays and brands.  They select a target,
 * and receive (slightly incomplete) infomation about racial type,
 * basic resistances, and HPs. -LM-
 */
static void pseudo_probe(void)
{
	char m_name[80];

	/* Acquire the target monster */
	int m_idx = p_ptr->target_who;
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	int approx_hp;
	int approx_mana=0;


	/* If no target monster, fail. */
	if (p_ptr->target_who < 1)
	{
		msg_print("You must actually target a monster.");
		return;
	}

	else
	{
		/* Get "the monster" or "something" */
		monster_desc(m_name, m_ptr, 0x04);

		/* Approximate monster HPs */
		approx_hp = m_ptr->hp - randint0(m_ptr->hp / 4) +
			randint0(m_ptr->hp / 4);

		/* Approximate monster HPs */
		if (r_ptr->mana)
			approx_mana = m_ptr->mana - randint0(m_ptr->mana / 4) +
			  randint0(m_ptr->mana / 4);

		/* Describe the monster */
		if (!(r_ptr->mana))
			msg("%^s has about %d hit points.", m_name, approx_hp);
		else
			msg("%^s has about %d hit points and about %d mana.", m_name, approx_hp, approx_mana);

		/* Learn some flags.  Chance of omissions. */
		if ((r_ptr->flags3 & (RF3_ANIMAL)) && (randint0(20) != 1))
			l_ptr->flags3 |= (RF3_ANIMAL);
		if ((r_ptr->flags3 & (RF3_EVIL)) && (randint0(10) != 1))
			l_ptr->flags3 |= (RF3_EVIL);
		if ((r_ptr->flags3 & (RF3_UNDEAD)) && (randint0(20) != 1))
			l_ptr->flags3 |= (RF3_UNDEAD);
		if ((r_ptr->flags3 & (RF3_DEMON)) && (randint0(20) != 1))
			l_ptr->flags3 |= (RF3_DEMON);
		if ((r_ptr->flags3 & (RF3_ORC)) && (randint0(20) != 1))
			l_ptr->flags3 |= (RF3_ORC);
		if ((r_ptr->flags3 & (RF3_TROLL)) && (randint0(20) != 1))
			l_ptr->flags3 |= (RF3_TROLL);
		if ((r_ptr->flags3 & (RF3_GIANT)) && (randint0(10) != 1))
			l_ptr->flags3 |= (RF3_GIANT);
		if ((r_ptr->flags3 & (RF3_DRAGON)) && (randint0(20) != 1))
			l_ptr->flags3 |= (RF3_DRAGON);
		if ((r_ptr->flags3 & (RF3_IM_ACID)) && (randint0(5) != 1))
			l_ptr->flags3 |= (RF3_IM_ACID);
		if ((r_ptr->flags3 & (RF3_IM_ELEC)) && (randint0(5) != 1))
			l_ptr->flags3 |= (RF3_IM_ELEC);
		if ((r_ptr->flags3 & (RF3_IM_FIRE)) && (randint0(5) != 1))
			l_ptr->flags3 |= (RF3_IM_FIRE);
		if ((r_ptr->flags3 & (RF3_IM_COLD)) && (randint0(5) != 1))
			l_ptr->flags3 |= (RF3_IM_COLD);
		if ((r_ptr->flags3 & (RF3_IM_POIS)) && (randint0(5) != 1))
			l_ptr->flags3 |= (RF3_IM_POIS);

		/* Confirm success. */
		msg_print("You feel you know more about this monster...");

		/* Update monster recall window */
		if (p_ptr->monster_race_idx == m_ptr->r_idx)
		{
			/* Window stuff */
			p_ptr->window |= (PW_MONSTER);
		}
	}
}



/*
 * Alter player's shape.  Taken from Sangband.
 */
void shapechange(s16b shape)
{
	char *shapedesc = "";

	/* Wonder Twin powers -- Activate! */
	p_ptr->schange = (byte) shape;
	p_ptr->update |= PU_BONUS;

	switch (shape)
	{
		case SHAPE_MOUSE:
			shapedesc = "mouse";
			break;
		case SHAPE_FERRET:
			shapedesc = "ferret";
			break;
		case SHAPE_HOUND:
			shapedesc = "hound";
			break;
		case SHAPE_GAZELLE:
			shapedesc = "gazelle";
			break;
		case SHAPE_LION:
			shapedesc = "lion";
			break;
		case SHAPE_ENT:
			shapedesc = "elder ent";
			break;
		case SHAPE_BAT:
			shapedesc = "bat";
			break;
		case SHAPE_WEREWOLF:
			shapedesc = "werewolf";
			break;
		case SHAPE_VAMPIRE:
			shapedesc = "vampire";
			break;
		case SHAPE_WYRM:
			shapedesc = "wyrm";
			break;
		case SHAPE_BEAR:
			shapedesc = "bear";
			break;
		default:
			msg_print("You return to your normal form.");
			break;
	}

	if (shape)
	{
		msg("You assume the form of a %s.", shapedesc);
		msg_print("Your equipment merges into your body.");
	}

	/* Recalculate mana. */
	p_ptr->update |= (PU_MANA);

	/* Show or hide shapechange on main window. */
	p_ptr->redraw |= (PR_SHAPE);
}


/*
 * Choose a paladin elemental attack. -LM-
 */
static void choose_ele_attack(void)
{
	int num;

	char choice;

	/* Save screen */
	screen_save();

	prt("", 14, 0);
	prt("        Choose a temporary elemental brand ", 1, 14);

	num = (p_ptr->lev - 20) / 7;

	c_prt(TERM_RED,                  "        a) Fire Brand", 2, 14);

	if (num >= 2) c_prt(TERM_L_WHITE,"        b) Cold Brand", 3, 14);
	else prt("", 3, 14);

	if (num >= 3) c_prt(TERM_L_DARK, "        c) Acid Brand", 4, 14);
	else prt("", 4, 14);

	if (num >= 4) c_prt(TERM_BLUE,   "        d) Elec Brand", 5, 14);
	else prt("", 5, 14);

	prt("", 6, 14);
	prt("", 7, 14);
	prt("", 8, 14);
	prt("", 9, 14);

	choice = inkey();

	if ((choice == 'a') || (choice == 'A'))
		set_ele_attack(ATTACK_FIRE, 200);
	else if (((choice == 'b') || (choice == 'B')) && (num >= 2))
		set_ele_attack(ATTACK_COLD, 200);
	else if (((choice == 'c') || (choice == 'C')) && (num >= 3))
		set_ele_attack(ATTACK_ACID, 200);
	else if (((choice == 'd') || (choice == 'D')) && (num >= 4))
		set_ele_attack(ATTACK_ELEC, 200);
	else msg_print("You cancel the temporary branding.");

	/* Load screen */
	screen_load();
}


/*
 * Hack -- The Athelas-creation code. -LM-
 */
void create_athelas(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	object_type *i_ptr;
	object_type object_type_body;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Hack -- Make some Athelas, identify it, and drop it near the player. */
	object_prep(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_ATHELAS));

	/* Prevent money-making. */
	i_ptr->discount = 80;

	object_aware(i_ptr);
	object_known(i_ptr);
	drop_near(i_ptr, -1, py, px);
}


/*
 * Controlled teleportation.  -LM-
 * Idea from PsiAngband, through Zangband.
 *
 * Return true if teleport occurred, false if canceled.
 */
bool dimen_door(void)
{
	int ny;
	int nx;
	bool okay;
	bool old_expand_look = expand_look;

	expand_look = TRUE;
	okay = target_set_interactive(TARGET_LOOK | TARGET_GRID);
	expand_look = old_expand_look;
	if (!okay) return(FALSE);

	/* grab the target coords. */
	ny = p_ptr->target_row;
	nx = p_ptr->target_col;

	/* Test for empty floor, forbid vaults or too large a
	 * distance, and insure that this spell is never certain.
	 */
	if (!cave_empty_bold(ny,nx) || (cave_info[ny][nx] & CAVE_ICKY) ||
		(distance(ny,nx,p_ptr->py,p_ptr->px) > 25) ||
		(randint0(p_ptr->lev) == 0))
	{
		msg_print("You fail to exit the astral plane correctly!");
		p_ptr->energy -= 50;
		teleport_player(15, FALSE);
		handle_stuff();
	}

	/* Controlled teleport. */
	else teleport_player_to(ny,nx,TRUE);

	return(TRUE);
}


/*
 * Rebalance Weapon.  This is a rather powerful spell, because it can be
 * used with any non-artifact throwing weapon, including ego-items.  It is
 * therefore high-level, and curses weapons on failure.  Do not give Assas-
 * sins "Break Curse". -LM-
 */
static void rebalance_weapon(void)
{
	u32b f1, f2, f3;

	object_type *o_ptr;
	char o_name[120];

	/* Select the wielded melee weapon */
	o_ptr = &inventory[INVEN_WIELD];

	/* Nothing to rebalance */
	if (!o_ptr->k_idx)
	{
		msg_print("You are not wielding any melee weapon.");
		return;
	}
	/* Artifacts not allowed. */
	if (o_ptr->name1)
	{
		msg_print("Artifacts cannot be rebalanced.");
		return;
	}

	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Not a throwing weapon. */
	if (!(f1 & (TR1_THROWING)))
	{
		msg_print("The melee weapon you are wielding is not designed for throwing.");
		return;
	}

	/* 20% chance to curse weapon. */
	else if (randint1(5) == 1)
	{
		/* Description */
		object_desc(o_name, o_ptr, FALSE, 0);

		/* Light curse and lower to_h and to_d by 2 to 5 each. */
		o_ptr->ident |= (IDENT_CURSED);
		o_ptr->to_h -= (s16b) (2 + randint0(4));
		o_ptr->to_d -= (s16b) (2 + randint0(4));

		/* Describe */
		msg("Oh no!  A dreadful black aura surrounds your %s!", o_name);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
	}

	/* Rebalance. */
	else
	{
		/* Grant perfect balance. */
		o_ptr->xtra1 = OBJECT_XTRA_TYPE_BALANCE;
		o_ptr->xtra2 = (byte) randint0(OBJECT_XTRA_SIZE_BALANCE);

		/* Description */
		object_desc(o_name, o_ptr, FALSE, 0);

		/* Describe */
		msg("Your %s gleams steel blue!", o_name);

		/* Prevent money-making. */
		o_ptr->discount = 80;
	}
}

/*
 * Calculate level boost for Channeling ability.
 */
int get_channeling_boost(void)
{
	long max_channeling = 45 + (2 * p_ptr->lev);
	long channeling = 0L;
	int boost;

	if (!(check_ability(SP_CHANNELING))) return 0;

	if (p_ptr->msp > 0) channeling = (max_channeling * p_ptr->csp * p_ptr->csp) / (p_ptr->msp * p_ptr->msp);
	boost = ((int) channeling + 5) / 10;

	return (boost);
}

/*
 * Calculate the total current spell level boost.
 */
int get_spell_level_boost(void)
{
	int boost = 0;
	if (check_ability(SP_HEIGHTEN_MAGIC)) boost += 1 + ((p_ptr->heighten_power + 5)/ 10);
	if (check_ability(SP_CHANNELING)) boost += get_channeling_boost();

	return boost;
}


/*
 * Allow user to choose a spell/prayer from the given book.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 *
 * The "prompt" should be "cast", "recite", "study", or "browse".
 * The "known" should be TRUE for cast/pray, FALSE for study
 */
static int get_spell(int *sn, const char * prompt, int tval, int sval)
{
	int i;

	int spell = -1;

	int first_spell, after_last_spell;

	int ver;

	int spell_level_boost;

	bool flag, redraw, okay;
	char choice;

	magic_type *s_ptr;

	char out_val[160];

	const char * p = "";

	const char * h = "";


#ifdef ALLOW_REPEAT /* TNB */

	/* Get the spell, if available */
	if (repeat_pull(sn))
	{
		/* Find the array index of the spellbook's first spell. */
		first_spell = mp_ptr->book_start_index[sval];

		/* Find the first spell in the next book. */
		after_last_spell = mp_ptr->book_start_index[sval+1];

		/* Verify the spell is in this book */
		if (((*sn) >= first_spell) && ((*sn) < after_last_spell))
		{
			/* Verify the spell is okay */
			if (spell_okay(*sn))
			{
				/* Success */
				return (TRUE);
			}
		}

		/* Invalid repeat - reset it */
		else repeat_clear();
	}

#endif /* ALLOW_REPEAT */

	/* Calculate effective level boost */
	spell_level_boost = get_spell_level_boost();

	/* Determine the magic description, for color. */
	if (mp_ptr->spell_book == TV_MAGIC_BOOK) p = "spell";
	if (mp_ptr->spell_book == TV_PRAYER_BOOK) p = "prayer";
	if (mp_ptr->spell_book == TV_DRUID_BOOK) p = "druidic lore";
	if (mp_ptr->spell_book == TV_NECRO_BOOK) p = "ritual";

	/* Power enhancement. */
	if (spell_level_boost > 16) h = "(**Empowered**) ";
	else if (spell_level_boost > 10) h = "(Empowered!!) ";
	else if (spell_level_boost > 5) h = "(Empowered) ";
	else h = "";

	/* Assume no usable spells */
	okay = FALSE;

	/* Assume no spells available */
	(*sn) = -2;


	/* Find the array index of the spellbook's first spell. */
	first_spell = mp_ptr->book_start_index[sval];

	/* Find the first spell in the next book. */
	after_last_spell = mp_ptr->book_start_index[sval+1];

	/* Check for "okay" spells */
	for (i = first_spell; i < after_last_spell; i++)
	{
		/* Look for "okay" spells */
		if (spell_okay(i)) okay = TRUE;
	}

	/* No "okay" spells */
	if (!okay) return (FALSE);

	/* Assume cancelled */
	*sn = (-1);

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;


	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit) %s%^s which %s? ",
		p, I2A(0), I2A(after_last_spell - first_spell - 1), h, prompt, p);

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice))
	{
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?'))
		{
			/* Hide the list */
			if (redraw)
			{
				/* Load screen */
				screen_load();

				/* Hide list */
				redraw = FALSE;
			}

			/* Show the list */
			else
			{
				/* Show list */
				redraw = TRUE;

				/* Save screen */
				screen_save();

				/* Display a list of spells */
				print_spells(tval, sval, 1, 14);
			}

			/* Ask again */
			continue;
		}


		/* Note verify */
		ver = (isupper(choice));

		/* Lowercase */
		choice = tolower(choice);

		/* Extract request */
		i = (islower(choice) ? A2I(choice) : -1);

		/* Totally Illegal */
		if ((i < 0) || (i >= after_last_spell - first_spell))
		{
			bell("Illegal spell choice!");
			continue;
		}

		/* Convert spellbook number to spell index. */
		spell = i + first_spell;

		/* Require "okay" spells */
		if (!spell_okay(spell))
		{
			bell("Illegal spell choice!");
			msg("You may not %s that %s.", prompt, p);
			continue;
		}

		/* Verify it */
		if (ver)
		{
			char tmp_val[160];

			/* Access the spell */
			s_ptr = &mp_ptr->info[spell];

			/* Prompt */
			strnfmt(tmp_val, 78, "%^s %s (%d mana, %d%% fail)? ",
				prompt, spell_names[s_ptr->index],
				s_ptr->smana, spell_chance(spell));

			/* Belay that order */
			if (!get_check(tmp_val)) continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}


	/* Restore the screen */
	if (redraw)
	{
		/* Load screen */
		screen_load();
	}


	/* Abort if needed */
	if (!flag) return (FALSE);

	/* Save the choice */
	(*sn) = spell;

#ifdef ALLOW_REPEAT /* TNB */
	repeat_push(*sn);

#endif /* ALLOW_REPEAT */

	/* Success */
	return (TRUE);
}


/*
 * Peruse the spells/prayers in a Book, showing "spell tips" as
 * requested. -LM-
 *
 * Note that browsing is allowed while confused or blind,
 * and in the dark, primarily to allow browsing in stores.
 */
void do_cmd_browse(void)
{
	int item, spell, lines;

	object_type *o_ptr;

	magic_type *s_ptr;

	const char * q = "";
	const char * s = "";

	/* Forbid illiterates to read spellbooks. */
	if (!mp_ptr->spell_book)
	{
		/* Warriors will eventually learn to pseudo-probe monsters. */
		if (check_ability(SP_PROBE)) warrior_probe_desc();

		else msg_print("You cannot read books!");
		return;
	}

	/* Restrict choices to "useful" books */
	item_tester_tval = mp_ptr->spell_book;

	/* Get a realm-flavored description. */
	if (mp_ptr->spell_book == TV_MAGIC_BOOK)
	{
		q = "Browse which magic book? ";
		s = "You have no magic books that you can read.";
	}
	if (mp_ptr->spell_book == TV_PRAYER_BOOK)
	{
		q = "Browse which holy book? ";
		s = "You have no holy books that you can read.";
	}
	if (mp_ptr->spell_book == TV_DRUID_BOOK)
	{
		q = "Browse which stone of lore? ";
		s = "You have no stones that you can read.";
	}
	if (mp_ptr->spell_book == TV_NECRO_BOOK)
	{
		q = "Browse which tome? ";
		s = "You have no tomes that you can read.";
	}

	/* Get an item */
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);

	/* Hack -- Handle stuff */
	handle_stuff();


	/* Save screen */
	screen_save();

	/* Display the spells */
	print_spells(o_ptr->tval, o_ptr->sval, 1, 14);

	/* Prompt for a command */
	put_str("(Browsing) Choose a spell, or ESC: ", 0, 0);


	/* Hack - Determine how far from the top of the screen the spell list
	 * extends by counting spells, and adding space for name, etc.
	 */
	lines = mp_ptr->book_start_index[o_ptr->sval + 1] -
		mp_ptr->book_start_index[o_ptr->sval] + 3;


	/* Keep browsing spells.  Exit browsing on cancel. */
	while(TRUE)
	{
		/* Ask for a spell, allow cancel */
		if (!get_spell(&spell, "browse", o_ptr->tval, o_ptr->sval))
		{
			/* If cancelled, leave immediately. */
			if (spell == -1) break;

			/* Notify that there's nothing to see, and wait. */
			c_put_str(TERM_SLATE, "No spells to browse     ", 0, 11);

			/* Any key cancels if no spells are available. */
			if (inkey()) break;
		}

		/* Clear lines, position cursor (really should use strlen here) */
		Term_erase(14, lines + 2, 255);
		Term_erase(14, lines + 1, 255);
		Term_erase(14, lines, 255);

		/* Access the spell */
		s_ptr = &mp_ptr->info[spell];

		/* Display that spell's information. */
		c_roff(TERM_L_BLUE, spell_tips[s_ptr->index], 16, 0);
	}

	/* Load screen */
	screen_load();
}




/*
 * Cast a spell or pray a prayer.
 */
void do_cmd_cast_or_pray(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int item, spell, dir;
	int chance, beam;
	s16b shape = 0;

	bool failed = FALSE;

	int plev = p_ptr->lev;

	object_type *o_ptr;

	magic_type *s_ptr;

	const char * p = "";
	const char * r = "";
	const char * t = "";

	const char * q = "";
	const char * s = "";


	/* Require spell ability. */
	if (!mp_ptr->spell_book)
	{
		/* Warriors will eventually learn to pseudo-probe monsters. */
		if (check_ability(SP_PROBE))
		{
			if (p_ptr->lev < 35)
				msg_print("You do not know how to probe monsters yet.");
			else if ((p_ptr->confused) || (p_ptr->image))
				msg_print("You feel awfully confused.");
			else
			{
				/* Get a target. */
				msg_print("Target a monster to probe.");
				if (!get_aim_dir(&dir)) return;

				/* Low-level probe spell. */
				pseudo_probe();

				/* Take a turn */
				p_ptr->energy_use = 100;
			}
		}
		else msg_print("You know no magical realm.");

		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Determine magic description. */
	if (mp_ptr->spell_book == TV_MAGIC_BOOK) p = "spell";
	if (mp_ptr->spell_book == TV_PRAYER_BOOK) p = "prayer";
	if (mp_ptr->spell_book == TV_DRUID_BOOK) p = "druidic lore";
	if (mp_ptr->spell_book == TV_NECRO_BOOK) p = "ritual";

	/* Determine spellbook description. */
	if (mp_ptr->spell_book == TV_MAGIC_BOOK) r = "magic book";
	if (mp_ptr->spell_book == TV_PRAYER_BOOK) r = "holy book";
	if (mp_ptr->spell_book == TV_DRUID_BOOK) r = "stone";
	if (mp_ptr->spell_book == TV_NECRO_BOOK) r = "tome";

	/* Determine method description. */
	if (mp_ptr->spell_book == TV_MAGIC_BOOK) t = "cast";
	if (mp_ptr->spell_book == TV_PRAYER_BOOK) t = "pray";
	if (mp_ptr->spell_book == TV_DRUID_BOOK) t = "use";
	if (mp_ptr->spell_book == TV_NECRO_BOOK) t = "perform";


	/* Restrict choices to spell books of the player's realm. */
	item_tester_tval = mp_ptr->spell_book;


	/* Get a realm-flavored description. */
	if (mp_ptr->spell_book == TV_MAGIC_BOOK)
	{
		q = "Use which magic book? ";
		s = "You have no magic books that you can use.";
	}
	if (mp_ptr->spell_book == TV_PRAYER_BOOK)
	{
		q = "Use which holy book? ";
		s = "You have no holy books that you can use.";
	}
	if (mp_ptr->spell_book == TV_DRUID_BOOK)
	{
		q = "Use which stone of lore? ";
		s = "You have no stones that you can use.";
	}
	if (mp_ptr->spell_book == TV_NECRO_BOOK)
	{
		q = "Use which tome? ";
		s = "You have no tomes that you can use.";
	}

	/* Get an item */
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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

	/* Track the object kind */
	object_kind_track(o_ptr->k_idx);

	/* Hack -- Handle stuff */
	handle_stuff();


	/* Ask for a spell */
	if (!get_spell(&spell, t, o_ptr->tval, o_ptr->sval))
	{
		if (spell == -2)
		{
			msg("You don't know any %s%s in that %s.", p,
				(mp_ptr->spell_book == TV_DRUID_BOOK) ? "" : "s", r);
		}
		return;
	}


	/* Access the spell */
	s_ptr = &mp_ptr->info[spell];


	/* Verify "dangerous" spells */
	if (s_ptr->smana > p_ptr->csp)
	{
		/* Warning */
		msg("You do not have enough mana to %s this %s.",
			t, p);

		/* Verify */
		if (!get_check("Attempt it anyway? ")) return;
	}


	/* Spell failure chance */
	chance = spell_chance(spell);

	/* Specialty Ability */
	plev += get_spell_level_boost();

	/* Failed spell */
	if (randint0(100) < chance)
	{
		failed = TRUE;

		if (flush_failure) flush();
		if (mp_ptr->spell_book == TV_MAGIC_BOOK)
			msg_print("You failed to get the spell off!");
		if (mp_ptr->spell_book == TV_PRAYER_BOOK)
			msg_print("You lost your concentration!");
		if (mp_ptr->spell_book == TV_DRUID_BOOK)
			msg_print("You lost your concentration!");
		if (mp_ptr->spell_book == TV_NECRO_BOOK)
			msg_print("You perform the ritual incorrectly!");
	}

	/* Process spell */
	else
	{
		/* Hack -- higher chance of "beam" instead of "bolt" for mages
		 * and necros.
		 */
		beam = ((check_ability(SP_BEAM)) ? plev : (plev / 2));



		/* Spell Effects.  Spells are mostly listed by realm, each using a
		 * block of 64 spell slots, but there are a certain number of spells
		 * that are used by more than one realm (this allows more neat class-
		 * specific magics)
		 */
		switch (s_ptr->index)
		{
			/* Sorcerous Spells */

			case 0:	/* Magic Missile */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt(GF_MANA, dir,
						  damroll(2, 4 + plev / 10));
				break;
			}
			case 1:	/* Detect Monsters */
			{
				(void)detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 2:	/* Phase Door */
			{
				teleport_player(10,TRUE);
				break;
			}
			case 3:	/* Light Area */
			{
				(void)lite_area(damroll(2, 1 + (plev / 5)), (plev / 10) + 1);
				break;
			}
			case 4:	/* Combat Poison */
			{
				(void)set_poisoned(p_ptr->poisoned / 2);
				break;
			}
			case 5:	/* Cure Light Wounds */
			{
				(void)hp_player(damroll(2, plev / 4 + 5));
				(void)set_cut(p_ptr->cut - 15);
				break;
			}
			case 6:	/* Rogue Spell: Detect Treasure */
			{
				/* Hack - 'show' effected region only with
				 * the first detect */
				(void)detect_treasure(DETECT_RAD_DEFAULT, TRUE);
				(void)detect_objects_gold(DETECT_RAD_DEFAULT, FALSE);
				break;
			}
			case 7:	/* Rogue Spell:	 Detect Objects */
			{
				(void)detect_objects_normal(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 8:	/* Stinking Cloud */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_POIS, dir, 5 + plev / 3, 2, FALSE);
				break;
			}
			case 9:	/* Confuse Monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)confuse_monster(dir, plev + 10);
				break;
			}
			case 10:	/* Lightning Bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam, GF_ELEC, dir,
						  damroll(2+((plev-5)/5), 8));
				break;
			}
			case 11:	/* Door Destruction */
			{
				(void)destroy_doors_touch();
				break;
			}
			case 12:	/* Sleep Monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)sleep_monster(dir, plev + 10);
				break;
			}
			case 13:	/* Cure Poison */
			{
				(void)set_poisoned(0);
				break;
			}
			case 14:	/* Teleport Self */
			{
				teleport_player(50 + plev * 2, TRUE);
				break;
			}
			case 15:	/* Spear of Light */
			{
				if (!get_aim_dir(&dir)) return;
				msg_print("A line of blue shimmering light appears.");
				lite_line(dir);
				break;
			}
			case 16:	/* Recharge Item I */
			{
				if (!recharge(85)) return;
				break;
			}
			case 17:	/* Cone of Cold */
			{
				if (!get_aim_dir(&dir)) return;
				fire_arc(GF_COLD, dir, 20 + plev, 3 + plev / 10, 45);
				break;
			}
			case 18:	/* Satisfy Hunger */
			{
				(void)set_food(PY_FOOD_MAX - 1);
				break;
			}
			case 19:	/* Magic Disarm */
			{
				if (!get_aim_dir(&dir)) return;
				(void)disarm_trap(dir);
				break;
			}
			case 20:	/* Polymorph Other */
			{
				if (!get_aim_dir(&dir)) return;
				(void)poly_monster(dir);
				break;
			}
			case 21:	/* Identify */
			{
				if (!ident_spell()) return;
				break;
			}
			case 22:	/* Sleep Monsters */
			{
				(void)sleep_monsters(plev + 10);
				break;
			}
			case 23:	/* Fire Bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam, GF_FIRE, dir,
						  damroll(7+((plev-5)/5), 8));
				break;
			}
			case 24:	/* Slow Monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)slow_monster(dir, plev + 10);
				break;
			}
			case 25:	/* Tap magical energy */
			{
				if (!tap_magical_energy()) return;
				break;
			}
			case 26:	/* Frost Ball */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_COLD, dir, 30 + plev, 2, FALSE);
				break;
			}
			case 27:	/* Recharge Item II */
			{
				if (!recharge(150)) return;
				break;
			}
			case 28:	/* Teleport Other */
			{
				if (!get_aim_dir(&dir)) return;
				(void)teleport_monster(dir, 45 + (plev/2));
				break;
			}
			case 29:	/* Haste Self */
			{
				if (!p_ptr->fast)
				{
					(void)set_fast(randint1(20) + plev);
				}
				else
				{
					(void)set_fast(p_ptr->fast + randint1(5));
				}
				break;
			}
			case 30:	/* Fire Ball */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_FIRE, dir, 55 + plev, 2, FALSE);
				break;
			}
			case 31:	/* Hold Monsters */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_HOLD, dir, 0, 2, FALSE);
				break;
			}
			case 32:	/* Word of Destruction */
			{
				destroy_area(py, px, 15, TRUE);
				break;
			}
			case 33:	/* Resist Fire */
			{
				(void)set_oppose_fire(p_ptr->oppose_fire + randint1(plev) + plev);
				break;
			}
			case 34:	/* Resist Cold */
			{
				(void)set_oppose_cold(p_ptr->oppose_cold + randint1(plev) + plev);
				break;
			}
			case 35:	/* Resist Acid */
			{
				(void)set_oppose_acid(p_ptr->oppose_acid + randint1(plev) + plev);
				break;
			}

			case 36:	/* Resist Poison */
			{
				(void)set_oppose_pois(p_ptr->oppose_pois + randint1(plev) + plev);
				break;
			}
			case 37:	/* Resistance */
			{
				(void)set_oppose_acid(p_ptr->oppose_acid + randint1(20) + 20);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint1(20) + 20);
				(void)set_oppose_fire(p_ptr->oppose_fire + randint1(20) + 20);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint1(20) + 20);
				(void)set_oppose_pois(p_ptr->oppose_pois + randint1(20) + 20);
				break;
			}
			case 38:	/* Door Creation */
			{
				(void)door_creation();
				break;
			}
			case 39:	/* Stair Creation */
			{
				(void)stair_creation();
				break;
			}
			case 40:	/* Teleport Level */
			{
				(void)teleport_player_level(TRUE);
				break;
			}
			case 41:	/* Word of Recall */
			{
#if 1
				word_recall(randint0(20) + 15);
#else
				if (!p_ptr->word_recall)
				{
					p_ptr->word_recall = randint0(20) + 15;
					msg_print("The air about you becomes charged...");
				}
				else
				{
					p_ptr->word_recall = 0;
					msg_print("A tension leaves the air around you...");
				}
				p_ptr->redraw |= PR_STATUS;
#endif
				break;
			}
			case 42: /* Dimension Door. */
			{
				msg_print("Choose a location to teleport to.");
				message_flush();
				if (!dimen_door()) return;
				break;
			}
			case 43:	/* Detect Evil */
			{
				(void)detect_monsters_evil(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 44:	/* Detect Enchantment */
			{
				(void)detect_objects_magic(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 45:	/* Earthquake */
			{
				earthquake(py, px, 10, FALSE);
				break;
			}
			case 46:	/* Beguiling */
			{
				(void)slow_monsters(5 * plev / 3);
				(void)confu_monsters(5 * plev / 3);
				(void)sleep_monsters(5 * plev / 3);
				break;
			}
			case 47:	/* Starburst */
			{
				fire_sphere(GF_LITE, 0,
					  5 * plev / 2, plev / 12, 20);
				break;
			}
			case 48:	/* Clear Mind */
			{
				if (p_ptr->csp < p_ptr->msp)
				{
					p_ptr->csp += 1 + plev / 12;
					p_ptr->csp_frac = 0;
					if (p_ptr->csp > p_ptr->msp) (p_ptr->csp = p_ptr->msp);
					msg_print("You feel your head clear a little.");
					p_ptr->redraw |= (PR_MANA);
					p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
				}
				break;
			}
			case 49:	/* Shield */
			{
				if (!p_ptr->shield)
				{
					(void)set_shield(p_ptr->shield + randint1(20) + 30);
				}
				else
				{
					(void)set_shield(p_ptr->shield + randint1(10) + 15);
				}
				break;
			}
			case 50:	/* Recharge Item III */
			{
				recharge(220);
				break;
			}
			case 51:	/* Essence of Speed */
			{
				if (!p_ptr->fast)
				{
					(void)set_fast(randint1(30) + 10 + plev);
				}
				else
				{
					(void)set_fast(p_ptr->fast + randint1(10));
				}
				break;
			}
			case 52:	/* Strengthen Defenses */
			{
				if (!p_ptr->magicdef)
				{
					(void)set_extra_defences(40);
				}
				else
				{
					(void)set_extra_defences(p_ptr->magicdef + randint1(20));
				}

				break;
			}
			case 53:	/* Acid Bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam, GF_ACID, dir,
						  damroll(5+((plev-5)/5), 8));
				break;
			}
			case 54:	/* Cloud Kill */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_POIS, dir, 10 + plev, 3, FALSE);
				break;
			}
			case 55:	/* Acid Ball */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_ACID, dir, 2 * plev, 2, FALSE);
				break;
			}
			case 56:	/* Ice Storm */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_COLD, dir, 3 * plev, 3, FALSE);
				break;
			}
			case 57:	/* Meteor Swarm */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_METEOR, dir, 80 + (plev * 2), 1, FALSE);
				break;
			}
			case 58:	/* Mana Storm */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_MANA, dir, 120 + (plev * 2), 4, FALSE);
				break;
			}
			case 59:	/* Rogue Spell: Hit and Run */
			{
				p_ptr->special_attack |= (ATTACK_FLEE);

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATUS);

				break;
			}
			case 60:	/* Rogue Spell: Day of Misrule */
			{
				const char * p = (p_ptr->psex == SEX_FEMALE ? "Daughters" : "Sons");
				msg("%s of Night rejoice!  It's the Day of Misrule!", p);
				(void)set_fast(randint1(30) + 30);
				(void)set_shero(p_ptr->shero + randint1(30) + 30);
				break;
			}


			/* Holy Prayers */

			case 64: /* Detect Evil */
			{
				(void)detect_monsters_evil(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 65: /* Cure Light Wounds */
			{
				(void)hp_player(damroll(2, plev / 4 + 5));
				(void)set_cut(p_ptr->cut - 10);
				break;
			}
			case 66: /* Bless */
			{
				if (!p_ptr->blessed)
				{
					(void)set_blessed(p_ptr->blessed + randint1(12) + 12);
				}
				else
				{
					(void)set_blessed(p_ptr->blessed + randint1(4) + 4);
				}
				break;
			}
			case 67: /* Remove Fear */
			{
				(void)set_afraid(0);
				break;
			}
			case 68: /* Call Light */
			{
				(void)lite_area(damroll(2, 1 + (plev / 3)), (plev / 10) + 1);
				break;
			}
			case 69: /* Find Traps */
			{
				(void)detect_traps(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 70: /* Detect Doors/Stairs */
			{
				/* Hack - 'show' effected region only with
				 * the first detect */
				(void)detect_doors(DETECT_RAD_DEFAULT, TRUE);
				(void)detect_stairs(DETECT_RAD_DEFAULT, FALSE);
				break;
			}
			case 71: /* Slow Poison */
			{
				(void)set_poisoned(p_ptr->poisoned / 2);
				break;
			}
			case 72: /* Cure Serious Wounds */
			{
				(void)hp_player(damroll(4, plev / 4 + 6));
				(void)set_cut((p_ptr->cut / 2) - 5);
				break;
			}
			case 73: /* Scare Monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)fear_monster(dir, (3 * plev / 2) + 10);
				break;
			}
			case 74: /* Portal */
			{
				teleport_player(2 * plev, TRUE);
				break;
			}
			case 75: /* Chant */
			{
				if (!p_ptr->blessed)
				{
					(void)set_blessed(p_ptr->blessed + randint1(24) + 24);
				}
				else
				{
					(void)set_blessed(p_ptr->blessed + randint1(8) + 8);
				}
				break;
			}
			case 76: /* Sanctuary */
			{
				(void)sleep_monsters_touch(plev + 25);
				break;
			}
			case 77: /* Satisfy Hunger */
			{
				(void)set_food(PY_FOOD_MAX - 1);
				break;
			}
			case 78: /* Remove Curse */
			{
				if (remove_curse()) msg_print ("You feel kindly hands aiding you.");
				break;
			}
			case 79: /* Resist Heat and Cold */
			{
				(void)set_oppose_fire(p_ptr->oppose_fire + randint1(10) + plev / 2);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint1(10) + plev / 2);
				break;
			}
			case 80: /* Neutralize Poison */
			{
				(void)set_poisoned(0);
				break;
			}
			case 81: /* Orb of Draining */
			{
				if (!get_aim_dir(&dir)) return;
				fire_sphere(GF_HOLY_ORB, dir, (damroll(3, 6) + plev / 4 +
					(plev / ((check_ability(SP_STRONG_MAGIC)) ? 2 : 4))),
					((plev < 30) ? 1 : 2), 30);
				break;
			}
			case 82: /* Sense Invisible */
			{
				(void)set_tim_invis(p_ptr->tim_invis + randint1(24) + plev);
				break;
			}
			case 83: /* Protection from Evil */
			{
				if (!p_ptr->protevil)
				{
					(void)set_protevil(p_ptr->protevil +
						randint1(24) + 3 * plev / 2);
				}
				else
				{
					(void)set_protevil(p_ptr->protevil + randint1(30));
				}
				break;
			}
			case 84: /* Cure Mortal Wounds */
			{
				(void)hp_player(damroll(9, plev / 3 + 12));
				(void)set_stun(0);
				(void)set_cut(0);
				break;
			}
			case 85: /* Earthquake */
			{
				earthquake(py, px, 10, FALSE);
				break;
			}
			case 86: /* Sense Surroundings. */
			{
				/* Extended area for high-level Rangers. */
				if ((check_ability(SP_LORE)) && (plev > 34))
					map_area(0, 0, TRUE);
				else	map_area(0, 0, FALSE);
				break;
			}
			case 87: /* Turn Undead */
			{
				(void)turn_undead((3 * plev / 2) + 10);
				break;
			}
			case 88: /* Prayer */
			{
				if (!p_ptr->blessed)
				{
					(void)set_blessed(p_ptr->blessed + randint1(48) + 48);
				}
				else
				{
					(void)set_blessed(p_ptr->blessed + randint1(12) + 12);
				}
				break;
			}
			case 89: /* Dispel Undead */
			{
				(void)dispel_undead(randint1(plev * 3));
				break;
			}
			case 90: /* Heal */
			{
				(void)hp_player(300);
				(void)set_stun(0);
				(void)set_cut(0);
				break;
			}
			case 91: /* Dispel Evil */
			{
				(void)dispel_evil(randint1(plev * 3));
				break;
			}
			case 92: /* Sacred Shield */
			{
				if (!p_ptr->shield)
				{
					(void)set_shield(p_ptr->shield + randint1(20) +
						plev / 2);
				}
				else
				{
					(void)set_shield(p_ptr->shield + randint1(10) +
						plev / 4);
				}
				break;
			}
			case 93: /* Glyph of Warding */
			{
				(void)warding_glyph();
				break;
			}
			case 94: /* Holy Word */
			{
				(void)dispel_evil(randint1(plev * 4));
				(void)hp_player(300);
				(void)set_afraid(0);
				(void)set_poisoned(p_ptr->poisoned - 200);
				(void)set_stun(0);
				(void)set_cut(0);
				break;
			}
			case 95: /* Blink */
			{
				teleport_player(10, TRUE);
				break;
			}
			case 96: /* Teleport Self */
			{
				teleport_player(plev * 4, TRUE);
				break;
			}
			case 97: /* Teleport Other */
			{
				if (!get_aim_dir(&dir)) return;
				(void)teleport_monster(dir, 45 + (plev/3));
				break;
			}
			case 98: /* Teleport Level */
			{
				(void)teleport_player_level(TRUE);
				break;
			}
			case 99: /* Word of Recall */
			{
#if 1
				word_recall(randint0(20) + 15);
#else
				if (p_ptr->word_recall == 0)
				{
					p_ptr->word_recall = randint0(20) + 15;
					msg_print("The air about you becomes charged...");
				}
				else
				{
					p_ptr->word_recall = 0;
					msg_print("A tension leaves the air around you...");
				}
				p_ptr->redraw |= PR_STATUS;
#endif
				break;
			}
			case 100: /* Alter Reality */
			{
				msg_print("The world changes!");

				/* Leaving */
				p_ptr->leaving = TRUE;

				break;
			}
			case 101: /* Detect Monsters */
			{
				(void)detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 102: /* Detection */
			{
				(void)detect_all(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 103: /* Probing */
			{
				(void)probing();
				break;
			}
			case 104: /* Perception */
			{
				if (!ident_spell()) return;
				break;
			}
			case 105: /* Clairvoyance */
			{
				wiz_lite(FALSE);
				break;
			}
			case 106: /* Banishment */
			{
				if (banish_evil(80))
				{
					msg_print("The power of your god banishes evil!");
				}
				break;
			}
			case 107: /* Healing */
			{
				(void)hp_player(500);
				(void)set_stun(0);
				(void)set_cut(0);
				break;
			}
			case 108: /* Sacred Knowledge */
			{
				(void)identify_fully();
				break;
			}
			case 109: /* Restoration */
			{
				(void)do_res_stat(A_STR);
				(void)do_res_stat(A_INT);
				(void)do_res_stat(A_WIS);
				(void)do_res_stat(A_DEX);
				(void)do_res_stat(A_CON);
				(void)do_res_stat(A_CHR);
				break;
			}
			case 110: /* Remembrance */
			{
				(void)restore_level();
				break;
			}
			case 111: /* Unbarring Ways */
			{
				(void)destroy_doors_touch();
				break;
			}
			case 112: /* Recharging */
			{
				if (!recharge(140)) return;
				break;
			}
			case 113: /* Dispel Curse */
			{
				if (remove_all_curse())
				msg_print("A beneficent force surrounds you for a moment.");
				break;
			}
			case 114: /* Disarm Trap */
			{
				if (!get_aim_dir(&dir)) return;
				(void)disarm_trap(dir);
				break;
			}
			case 115: /* Holding */
			{
				if (!get_aim_dir(&dir)) return;

				/* Spell will hold any monster or door in one square. */
				fire_ball(GF_HOLD, dir, 0, 0, FALSE);

				break;
			}
			case 116: /* Enchant Weapon or Armour */
			{
				char answer;

				/* Query */
				msg_print("Would you like to enchant a 'W'eapon or 'A'rmour (w/a)");

				/* Interact and enchant. */
				while(1)
				{
					answer = inkey();
					if ((answer == 'W') || (answer == 'w'))
					{
						(void)enchant_spell(randint0(4) + 1,
						randint0(4) + 1, 0);
						break;
					}
					else if ((answer == 'A') || (answer == 'a'))
					{
						(void)enchant_spell(0, 0, randint0(3) + 2);
						break;
					}
					else if (answer == ESCAPE) return;
					else msg_print("Please type 'w' to enhant a weapon, 'a' to enchant armour, or ESC to cancel");
				}

				break;
			}
			case 117: /* Ball of Light */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_LITE, dir, plev * 2, 3, FALSE);
				break;
			}
			case 118: /* Holy Lance */
			{
				if (!get_aim_dir(&dir)) return;
				fire_beam(GF_HOLY_ORB, dir, 3 * plev / 2);
				break;
			}
			case 119: /* Word of Destruction */
			{
				destroy_area(py, px, 15, TRUE);
				break;
			}

			case 120: /* Annihilation */
			{
				if (!get_aim_dir(&dir)) return;
				drain_life(dir, plev * 3 + randint1(100));
				break;
			}
			case 121: /* Call on Varda */
			{
				msg_print("Gilthoniel A Elbereth!");
				fire_sphere(GF_LITE, 0,
					  plev * 5, plev / 7 + 2, 20);
				(void)fear_monsters(plev * 2);
				(void)hp_player(500);
				break;
			}

			case 122: /* Paladin Prayer: Elemental Infusion */
			{
				choose_ele_attack();
				break;
			}
			case 123: /* Paladin Prayer: Sanctify for Battle */
			{
				/* Can't have black breath and holy attack (doesn't happen anyway) */
				if (p_ptr->special_attack & ATTACK_BLKBRTH) p_ptr->special_attack &= ~ ATTACK_BLKBRTH;

				if (!(p_ptr->special_attack & ATTACK_HOLY))
					msg_print("Your blows will strike with Holy might!");
				p_ptr->special_attack |= (ATTACK_HOLY);

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATUS);

				break;
			}
			case 124: /* Paladin Prayer: Horn of Wrath */
			{
				(void)hp_player(20);
				if (!p_ptr->hero)
				{
					(void)set_hero(p_ptr->hero + randint1(20) + 20);
				}
				else
				{
					(void)set_hero(p_ptr->hero + randint1(10) + 10);
				}
				(void)set_afraid(0);

				(void)fear_monsters(plev);
				break;
			}
			case 125: /* Purifying Strike */
			{
				int type;
				if (!get_aim_dir(&dir)) return;

				if (randint0(4) == 0) type = GF_LITE;
				else if (randint0(3) == 0) type = GF_HOLY_ORB;
				else if (randint0(2) == 0) type = GF_FIRE;
				else type = GF_MANA;

				fire_ball(type, dir, 25 + plev + randint1(plev), 0, FALSE);
				break;
			}


			/* Nature Magics */

			case 128: /* detect life */
			{
				(void)detect_monsters_living(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 129:  /* call light */
			{
				(void)lite_area(damroll(2, 1 + (plev / 4)), (plev / 10) + 1);
				break;
			}
			case 130: /* foraging */
			{
				(void)set_food(PY_FOOD_MAX - 1);
				break;
			}
			case 131:  /* blink */
			{
				teleport_player(10, TRUE);
				break;
			}
			case 132:  /* combat poison */
			{
				(void)set_poisoned(p_ptr->poisoned / 2);
				break;
			}
			case 133:  /* lightning spark */
			{
				if (!get_aim_dir(&dir)) return;
				fire_arc(GF_ELEC, dir, damroll(2 + (plev/8), 6),
					(1 + plev / 5), 0);
				break;
			}
			case 134:  /* door destruction */
			{
				(void)destroy_doors_touch();
				break;
			}
			case 135:  /* turn stone to mud */
			{
				if (!get_aim_dir(&dir)) return;
				(void)wall_to_mud(dir);
				break;
			}
			case 136:  /* ray of sunlight */
			{
				if (!get_aim_dir(&dir)) return;
				msg_print("A ray of golden yellow light appears.");
				lite_line(dir);
				break;
			}
			case 137:  /* Cure poison */
			{
				(void)set_poisoned(0);
				break;
			}
			case 138:  /* frost bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam - 10, GF_COLD, dir,
						  damroll(2 + (plev/5), 8));
				break;
			}
			case 139:  /* sleep creature */
			{
				if (!get_aim_dir(&dir)) return;
				(void)sleep_monster(dir, plev + 10);
				break;
			}
			case 140:  /* frighten creature */
			{
				if (!get_aim_dir(&dir)) return;
				(void)fear_monster(dir, plev + 10);
				break;
			}
			case 141:  /* detect trap/doors */
			{
				(void)detect_traps(DETECT_RAD_DEFAULT, FALSE);
				(void)detect_doors(DETECT_RAD_DEFAULT, FALSE);
				(void)detect_stairs(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 142:  /* snuff small life */
			{
				(void)dispel_small_monsters(2 + plev / 5);
				break;
			}
			case 143:  /* fire bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam - 10, GF_FIRE, dir,
						  damroll(3 + (plev/5), 8));
				break;
			}
			case 144:  /* heroism */
			{
				(void)hp_player(20);
				if (!p_ptr->hero)
				{
					(void)set_hero(p_ptr->hero + randint1(20) + 20);
				}
				else
				{
					(void)set_hero(p_ptr->hero + randint1(10) + 10);
				}
				(void)set_afraid(0);
				break;
			}
			case 145:  /* remove curse */
			{
				if (remove_curse()) msg_print("You feel tender hands aiding you.");
				break;
			}
			case 146:  /* acid bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam - 10, GF_ACID, dir,
						  damroll(5 + (plev/5), 8));
				break;
			}
			case 147:  /* teleport monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)teleport_monster(dir, 45 + (plev/3));
				break;
			}
			case 148:  /* poison bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam - 10, GF_POIS, dir,
						  damroll(7 + (plev/4), 8));
				break;
			}
			case 149:  /* resist poison */
			{
				(void)set_oppose_pois(p_ptr->oppose_pois + randint1(20) + 20);
				break;
			}
			case 150:  /* earthquake */
			{
				earthquake(py, px, 10, FALSE);
				break;
			}
			case 151:  /* resist fire & cold */
			{
				(void)set_oppose_fire(p_ptr->oppose_fire + randint1(20) + 20);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint1(20) + 20);
				break;
			}
			case 152:  /* detect all */
			{
				(void)detect_all(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 153:  /* natural vitality */
			{
				(void)set_poisoned((3 * p_ptr->poisoned / 4) - 5);
				(void)hp_player(damroll(2, plev / 5));
				(void)set_cut(p_ptr->cut - plev / 2);
				break;
			}
			case 154:  /* resist acid & lightning */
			{
				(void)set_oppose_acid(p_ptr->oppose_acid + randint1(20) + 20);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint1(20) + 20);
				break;
			}
			case 155:  /* wither foe */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt(GF_MANA, dir,
					damroll(plev / 7, 8));
				(void)confuse_monster(dir, plev + 10);
				(void)slow_monster(dir, plev + 10);

				break;
			}
			case 156:  /* disarm trap */
			{
				if (!get_aim_dir(&dir)) return;
				(void)disarm_trap(dir);
				break;
			}
			case 157:  /* identify */
			{
				if (!ident_spell()) return;
				break;
			}
			case 158:  /* create athelas */
			{
				(void)create_athelas();
				break;
			}
			case 159:  /* raging storm */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_ELEC, dir, plev + randint1(60 + plev * 2),
					(1 + plev / 15), FALSE);
				break;
			}
			case 160:  /* thunderclap */
			{
				msg_print("Boom!");
				fire_sphere(GF_SOUND, 0,
					  plev + randint1(40 + plev * 2), plev / 8, 20);
				break;
			}
			case 161:  /* become mouse */
			{
				shape = SHAPE_MOUSE;
				break;
			}
			case 162:  /* become ferret */
			{
				shape = SHAPE_FERRET;
				break;
			}
			case 163:  /* become hound */
			{
				shape = SHAPE_HOUND;
				break;
			}
			case 164:  /* become gazelle */
			{
				shape = SHAPE_GAZELLE;
				break;
			}
			case 165:  /* become lion */
			{
				shape = SHAPE_LION;
				break;
			}
			case 166:  /* detect evil */
			{
				(void)detect_monsters_evil(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 167:  /* song of frightening */
			{
				(void)fear_monsters(3 * plev / 2 + 10);
				break;
			}
			case 168:  /* sense surroundings */
			{
				map_area(0, 0, FALSE);
				break;
			}
			case 169:  /* sight beyond sight */
			{
				/* Hack - 'show' effected region only with
				 * the first detect */
				(void)detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
				(void)detect_monsters_invis(DETECT_RAD_DEFAULT, FALSE);
				(void)detect_traps(DETECT_RAD_DEFAULT, FALSE);
				(void)detect_doors(DETECT_RAD_DEFAULT, FALSE);
				(void)detect_stairs(DETECT_RAD_DEFAULT, FALSE);
				if (!p_ptr->tim_invis)
				{
					(void)set_tim_invis(p_ptr->tim_invis +
						randint1(24) + 24);
				}
				else
				{
					(void)set_tim_invis(p_ptr->tim_invis +
						randint1(12) + 12);
				}
				break;
			}
			case 170:  /* herbal healing */
			{
				(void)hp_player(damroll(25 + plev / 2, 12));
				(void)set_cut(0);
				(void)set_poisoned(p_ptr->poisoned - 200);
				break;
			}
			case 171:  /* blizzard */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_COLD, dir, plev + randint1(50 + plev * 2),
					1 + plev / 12, FALSE);
				break;
			}
			case 172:  /* trigger tsunami */
			{
				msg_print("You hurl mighty waves at your foes!");
				fire_sphere(GF_WATER, 0,
					  30 + ((4 * plev) / 5) + randint1(30 + plev * 2), plev / 7, 20);
				break;
			}
			case 173:  /* volcanic eruption */
			{
				msg_print("The earth convulses and erupts in fire!");
				fire_sphere(GF_FIRE, 0, 3 * plev / 2 + randint1(50 + plev * 3), 1 + plev / 15, 20);
				earthquake(py, px, plev / 5, TRUE);
				break;
			}
			case 174:  /* molten lightning */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_PLASMA, dir,
					35 + (2 * plev) + randint1(90 + plev * 4), 1, FALSE);
				break;
			}
			case 175:  /* starburst. */
			{
				msg_print("Light bright beyond enduring dazzles your foes!");
				fire_sphere(GF_LITE, 0,
					  40 + (3 * plev / 2) + randint1(plev * 3), plev / 10, 20);
				break;
			}
			case 176:  /* song of lulling */
			{
				msg_print("Your tranquil music enchants those nearby.");

				(void)slow_monsters(5 * plev / 3);
				(void)sleep_monsters(5 * plev / 3);
				break;
			}
			case 177:  /* song of protection */
			{
				msg_print("Your song creates a mystic shield.");
				if (!p_ptr->shield)
				{
					(void)set_shield(p_ptr->shield + randint1(30) +
						plev / 2);
				}
				else
				{
					(void)set_shield(p_ptr->shield + randint1(15) +
						plev / 4);
				}
				break;
			}
			case 178:  /* song of dispelling */
			{
				msg_print("An unbearable discord tortures your foes!");

				(void)dispel_monsters(randint1(plev * 2));
				(void)dispel_evil(randint1(plev * 2));
				break;
			}
			case 179:  /* song of warding */
			{
				msg_print("Your song creates a place of sanctuary.");

				(void)warding_glyph();
				break;
			}
			case 180:  /* song of renewal */
			{
				msg_print("Amidst the gloom, you envoke light and beauty; your body regains its natural vitality.");

				(void)do_res_stat(A_STR);
				(void)do_res_stat(A_INT);
				(void)do_res_stat(A_WIS);
				(void)do_res_stat(A_DEX);
				(void)do_res_stat(A_CON);
				(void)do_res_stat(A_CHR);
				(void)restore_level();
				break;
			}
			case 181:  /* time blast */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(plev * 2, GF_TIME, dir,
						  damroll(plev / 6, 8));
				break;
			}
			case 182:  /* essence of speed */
			{
				if (!p_ptr->fast)
				{
					(void)set_fast(randint1(10) + plev / 2);
				}
				else
				{
					(void)set_fast(p_ptr->fast + randint1(5));
				}
				break;
			}
			case 183:  /* infusion */
			{
				if (!recharge(125)) return;
				break;
			}
			case 184:  /* become ent */
			{
				shape = SHAPE_ENT;
				break;
			}
			case 185:  /* regain life */
			{
				(void)restore_level();
				break;
			}
			case 186:  /* intervention of Yavanna */
			{
				(void)dispel_evil(100);
				(void)hp_player(500);
				(void)set_blessed(p_ptr->blessed + randint1(100) + 100);
				(void)set_afraid(0);
				(void)set_poisoned(0);
				(void)set_stun(0);
				(void)set_cut(0);
				break;
			}
			case 187:  /* Ranger Spell:  Creature Knowledge */
			{
				msg_print("Target the monster you wish to learn about.");
				if (!get_aim_dir(&dir)) return;
				pseudo_probe();
				break;
			}


			/* Necromantic Spells */

			case 192: /* magic bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt(GF_MANA, dir, damroll(2, 5 + plev / 7));
				break;
			}
			case 193: /* detect evil */
			{
				(void)detect_monsters_evil(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 194: /* enhanced infravision */
			{
				set_tim_infra(p_ptr->tim_infra + 70 + randint1(70));
				break;
			}
			case 195: /* break curse */
			{
				if (remove_curse())
					msg_print("You feel mighty hands aiding you.");
				break;
			}
			case 196: /* slow monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)slow_monster(dir, plev + 5);
				break;
			}
			case 197: /* sleep monster */
			{
				if (!get_aim_dir(&dir)) return;
				(void)sleep_monster(dir, plev + 5);
				break;
			}
			case 198: /* horrify */
			{
				if (!get_aim_dir(&dir)) return;
				(void)fear_monster(dir, plev + 15);
				break;
			}
			case 199: /* become bat */
			{
				take_hit(damroll(2, 4), "shapeshifting stress");
				shape = SHAPE_BAT;
				break;
			}
			case 200: /* door destruction */
			{
				(void)destroy_doors_touch();
				break;
			}
			case 201: /* dark bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam - 10, GF_DARK, dir,
						  damroll((3 + plev / 7), 8));
				break;
			}
			case 202: /* noxious fumes */
			{
				fire_sphere(GF_POIS, 0, 10 + plev, 2 + plev / 12, 40);

				pois_hit(5);
				break;
			}
			case 203: /* turn undead */
			{
				(void)turn_undead(3 * plev / 2);
				break;
			}
			case 204: /* turn evil */
			{
				(void)turn_evil(5 * plev / 4);
				break;
			}
			case 205: /* cure poison */
			{
				(void)set_poisoned(0);
				break;
			}
			case 206: /* dispel undead */
			{
				(void)dispel_undead(plev + 15 + randint1(3 * plev / 2));
				break;
			}
			case 207: /* dispel evil */
			{
				(void)dispel_evil(plev + randint1(plev));
				break;
			}
			case 208: /* see invisible */
			{
				if (!p_ptr->tim_invis)
				{
					set_tim_invis(p_ptr->tim_invis + 20 +
						randint1(plev / 2));
				}
				else
				{
					set_tim_invis(p_ptr->tim_invis + 10 +
						randint1(plev / 4));
				}
				break;
			}
			case 209: /* shadow shifting */
			{
				take_hit(damroll(1, 4), "shadow dislocation");
				teleport_player(16, TRUE);
				break;
			}
			case 210: /* detect traps */
			{
				(void)detect_traps(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 211: /* detect doors/stairs */
			{
				/* Hack - 'show' effected region only with
				 * the first detect */
				(void)detect_doors(DETECT_RAD_DEFAULT, TRUE);
				(void)detect_stairs(DETECT_RAD_DEFAULT, FALSE);
				break;
			}
			case 212: /* sleep monsters */
			{
				(void)sleep_monsters(plev + 5);
				break;
			}
			case 213: /* slow monsters */
			{
				(void)slow_monsters(plev + 5);
				break;
			}
			case 214: /* detect magic */
			{
				(void)detect_objects_magic(DETECT_RAD_DEFAULT, TRUE);
				break;
			}
			case 215: /* death bolt */
			{
				if (!get_aim_dir(&dir)) return;
				take_hit(damroll(1, 6), "the dark arts");
				fire_bolt_or_beam(beam, GF_SPIRIT, dir,
				       damroll(2 + plev / 3, 8));
				break;
			}
			case 216: /* resist poison */
			{
				(void)set_oppose_pois(p_ptr->oppose_pois + randint1(20) + plev / 2);
				break;
			}
			case 217: /* Exorcise Demons */
			{
				(void)dispel_demons(2 * plev + randint1(2 * plev));
				break;
			}
			case 218: /* dark spear */
			{
				if (!get_aim_dir(&dir)) return;
				fire_beam(GF_DARK, dir, 12 + plev);
				break;
			}
			case 219: /* mana bolt */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt_or_beam(beam, GF_MANA, dir,
				       damroll(1 + plev / 2, 8));
				break;
			}
			case 220: /* genocide */
			{
				(void)genocide();
				break;
			}
			case 221: /* dark ball */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_DARK, dir, 50 + plev * 2, 2, FALSE);
				break;
			}
			case 222: /* stench of death */
			{
				take_hit(damroll(2, 8), "the stench of Death");
				(void)dispel_living(50 + randint1(plev));
				confu_monsters(plev + 10);
				if (get_aim_dir(&dir))
				{
					fire_sphere(GF_POIS, dir, plev * 2, 5 + plev / 11, 40);
				}
				break;
			}
			case 223: /* probing */
			{
				(void)probing();
				break;
			}
			case 224: /* shadow mapping */
			{
				map_area(0, 0, FALSE);
				break;
			}
			case 225: /* identify */
			{
				if (!ident_spell()) return;
				break;
			}
			case 226: /* shadow warping */
			{
				take_hit(damroll(2, 6), "shadow dislocation");
				teleport_player(plev * 3, TRUE);
				break;
			}
			case 228: /* resist acid and cold */
			{
				(void)set_oppose_acid(p_ptr->oppose_pois + randint1(20) + 20);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint1(20) + 20);
				break;
			}
			case 229: /* heal any wound */
			{
				(void)set_cut(0);
				(void)set_stun(0);
				break;
			}
			case 230: /* protection from evil */
			{
				(void)set_protevil(p_ptr->protevil + plev / 2 + randint1(plev));
				break;
			}
			case 231: /* black blessing */
			{
				(void)set_blessed(p_ptr->blessed + randint1(66));
				break;
			}
			case 232: /* banish evil */
			{
				(void)banish_evil(80);
				break;
			}
			case 233: /* shadow barrier */
			{
				if (!p_ptr->shield)
				{
					(void)set_shield(p_ptr->shield + randint1(20) + 10);
				}
				else
				{
					(void)set_shield(p_ptr->shield + randint1(10) + 5);
				}
				break;
			}
			case 234: /* detect all monsters */
			{
				/* Hack - 'show' affected region only with
				 * the first detect */
				(void)detect_monsters_normal(DETECT_RAD_DEFAULT, TRUE);
				(void)detect_monsters_invis(DETECT_RAD_DEFAULT, FALSE);
				break;
			}
			case 235: /* strike at life */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt(GF_NETHER, dir,
				       damroll(3 * plev / 5, 13));
				break;
			}
			case 236: /* orb of death */
			{
				if (!get_aim_dir(&dir)) return;
				take_hit(damroll(2, 8), "Death claiming his wages");
				fire_sphere(GF_SPIRIT, dir, 20 + (4 * plev), 1, 20);
				break;
			}
			case 237: /* dispel life */
			{
				(void)dispel_living(60 + randint1(plev * 2));
				break;
			}
			case 238: /* vampiric drain */
			{
				if (!get_aim_dir(&dir)) return;
				fire_bolt(GF_SPIRIT, dir,
				       damroll(plev / 3, 11));
				(void)hp_player(3 * plev);
				(void)set_food(p_ptr->food + 1000);
				break;
			}
			case 239: /* recharging */
			{
				if (!recharge(140)) return;
				break;
			}
			case 240: /* become werewolf */
			{
				take_hit(damroll(2, 7), "shapeshifting stress");
				shape = SHAPE_WEREWOLF;
				break;
			}
			case 241: /* dispel curse */
			{
				if (remove_all_curse()) msg_print ("You feel mighty hands aiding you.");
				break;
			}
			case 242: /* become vampire */
			{
				take_hit(damroll(3, 6), "shapeshifting stress");
				shape = SHAPE_VAMPIRE;
				break;
			}
			case 243: /* haste self */
			{
				if (!p_ptr->fast)
				{
					(void)set_fast(10 + randint1(20));
				}
				else
				{
					(void)set_fast(p_ptr->fast + randint1(5));
				}
				break;
			}
			case 244: /* prepare black breath */
			{
				/* Can't have black breath and holy attack (doesn't happen anyway) */
				if (p_ptr->special_attack & ATTACK_HOLY) p_ptr->special_attack &= ~ ATTACK_HOLY;

				if (!(p_ptr->special_attack & ATTACK_BLKBRTH))
				   msg_print("Your hands start to radiate Night.");
				p_ptr->special_attack |= (ATTACK_BLKBRTH);

				/* Redraw the state */
				p_ptr->redraw |= (PR_STATUS);

				break;
			}
			case 245: /* word of destruction */
			{
				destroy_area(py, px, 15, TRUE);
				break;
			}
			case 246: /* teleport away */
			{
				if (!get_aim_dir(&dir)) return;
				(void)teleport_monster(dir, 45 + (plev/3));
				break;
			}
			case 247: /* smash undead */
			{
				(void)dispel_undead(plev * 3 + randint1(50));
				break;
			}
			case 248: /* bind undead */
			{
				(void)hold_undead();
				break;
			}
			case 249: /* darkness storm */
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_DARK, dir, 11 * plev / 2, plev / 7, FALSE);
				break;
			}
			case 250: /* Necro spell - timed ESP */
			{
				if (!p_ptr->tim_esp)
				{
					(void)set_tim_esp(30 + randint1(40));
				}
				else
				{
					(void)set_tim_esp(p_ptr->tim_esp + randint1(30));
				}
				break;
			}
			case 251:	/* Rogue and Assassin Spell - Slip into the Shadows */
			{
				if (!p_ptr->superstealth)
				{
					(void)set_superstealth(40);
				}
				else
				{
					(void)set_superstealth(p_ptr->superstealth + randint1(20));
				}
				break;
			}
			case 252:	/* Assassin spell: Bloodwrath */
			{
				if (!p_ptr->shero)
				{
					(void)set_shero(40);
				}
				else
				{
					(void)set_shero(p_ptr->shero + randint1(20));
				}

				(void)set_fast(40);

				break;
			}
			case 253:	/* Assassin Spell - Rebalance Weapon */
			{
				rebalance_weapon();
				break;
			}



			/* Archery Spells */
			case 255:  /* Rogue Spell: Stone Volley */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval || (p_ptr->ammo_tval != TV_SHOT))
				{
					msg_print("You must equip a sling to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_MULTI)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 263:  /* Ranger Spell:  Deadeye Shot */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval || !p_ptr->ammo_tval)
				{
					msg_print("You must equip a missile weapon to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_DEADEYE)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 264:  /* Ranger Spell:  Rain of Arrows */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval || (p_ptr->ammo_tval != TV_ARROW))
				{
					msg_print("You must equip a bow to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_MULTI)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 265:  /* Ranger Spell:  Storm Shot */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval ||
				    !((p_ptr->ammo_tval == TV_ARROW) ||
				      (p_ptr->ammo_tval == TV_BOLT)))
				{
					msg_print("You must equip a bow or crossbow to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_STORM)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 266:  /* Ranger Spell:  Dragonslayer */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval ||
				    !((p_ptr->ammo_tval == TV_ARROW) ||
				      (p_ptr->ammo_tval == TV_BOLT)))
				{
					msg_print("You must equip a bow or crossbow to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_SLAY_DRAGON)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 267:  /* Ranger Spell:  Scourging Shot */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval || !p_ptr->ammo_tval)
				{
					msg_print("You must equip a missile weapon to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_SCOURGE)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 271:  /* Assassin Spell:  Vile Dart */
			{
				msg_print("You channel the power of Night.");

				if (!do_cmd_throw(THROW_MODE_BLKBRTH)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}
			case 272: /* poison ammo - for assassins only */
			{
				/* Get the "bow" (if any) */
				o_ptr = &inventory[INVEN_BOW];

				/* Require a usable launcher */
				if (!o_ptr->tval || !p_ptr->ammo_tval)
				{
					msg_print("You must equip a missile weapon to use this spell.");
					return;
				}

				if (!do_cmd_fire(FIRE_MODE_POISON)) return;

				/* recalculate energy cost later */
				p_ptr->energy_use = 0;

				break;
			}

			default:	/* No Spell */
			{
				msg_print("Undefined Spell");
				break;
			}
		}

		/* A spell was cast or a prayer prayed */
		if (!((spell < 32) ?
		       (p_ptr->spell_worked1 & (1L << spell)) :
		       (p_ptr->spell_worked2 & (1L << (spell - 32)))))
		{
			int e = s_ptr->sexp;

			/* The spell or prayer worked */
			if (spell < 32)
			{
				p_ptr->spell_worked1 |= (1L << spell);
			}
			else
			{
				p_ptr->spell_worked2 |= (1L << (spell - 32));
			}

			/* Gain experience */
			gain_exp(e * s_ptr->slevel);
		}
	}

	/*
	 * Energy usage
	 */

	/* Normal spell - 100 energy modified by Fast Casting specialty */
	if (s_ptr->stimecode == CAST_TIME_NORMAL)
	{
		/* Take a turn */
		p_ptr->energy_use = 100;

		/* Reduced for fast casters */
		if (check_ability(SP_FAST_CAST))
		{
			int level_difference = p_ptr->lev - s_ptr->slevel;

			p_ptr->energy_use -= 5 + (level_difference / 2);

			/* Increased bonus for really easy spells */
			if (level_difference > 25) p_ptr->energy_use -= (level_difference - 25);
		}

		/* Give Credit for Heighten Magic */
		if (check_ability(SP_HEIGHTEN_MAGIC)) add_heighten_power(20);

		/* Paranioa */
		if (p_ptr->energy_use < 50) p_ptr->energy_use = 50;
	}

	/* Archery type spells */
	else if (s_ptr->stimecode == CAST_TIME_SHOOT) p_ptr->energy_use = (1000 / p_ptr->num_fire);
	else if (s_ptr->stimecode == CAST_TIME_SHOOT_FAST) p_ptr->energy_use = (500 / p_ptr->num_fire);
	else if (s_ptr->stimecode == CAST_TIME_SHOOT_SLOW) p_ptr->energy_use = (1500 / p_ptr->num_fire);

	/* Throwing type spells */
	else if (s_ptr->stimecode == CAST_TIME_THROW) p_ptr->energy_use = 100;
	else if (s_ptr->stimecode == CAST_TIME_THROW_FAST) p_ptr->energy_use = 50;
	else if (s_ptr->stimecode == CAST_TIME_THROW_SLOW) p_ptr->energy_use = 150;

	/* Unknown casting speed code */
	else p_ptr->energy_use = 100;

	/* Sufficient mana */
	if (s_ptr->smana <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= s_ptr->smana;

		/* Specialty ability Harmony */
		if ((failed == FALSE) & (check_ability(SP_HARMONY)))
		{
		  	int frac, boost;

			/* Percentage of max hp to be regained */
			frac = 3 + (s_ptr->smana / 3);

			/* Cap at 10 % */
			if (frac > 10) frac = 10;

			/* Calculate fractional bonus */
			boost = (frac * p_ptr->mhp) / 100;

			/* Apply bonus */
			(void)hp_player(boost);
		}
	}

	/* Over-exert the player */
	else
	{
		int oops = s_ptr->smana - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		if (mp_ptr->spell_book == TV_NECRO_BOOK)
			msg_print("You collapse after the ritual!");
		else msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(p_ptr->paralyzed + randint1(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (randint0(100) < 50)
		{
			bool perm = (randint0(100) < 25);

			/* Message */
			msg_print("You have damaged your health!");

			/* Reduce constitution */
			(void)dec_stat(A_CON, 15 + randint1(10), perm);
		}
	}

	/* Alter shape, if necessary. */
	if (shape) shapechange(shape);


	/* Redraw mana */
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);

}

/*
 * Check if we have a race, class, or specialty ability -BR-
 */
bool check_ability(int ability)
{
	int i;

	/* Check for innate racial specialty */
	if ((ability >= SP_RACIAL_START) && (ability <= SP_RACIAL_END))
	{
		int offset = ability - SP_RACIAL_START;
		if (rp_ptr->flags_special & (0x00000001 << offset)) return(TRUE);
	}

	/* Check for innate class ability */
	if ((ability >= SP_CLASS_START) && (ability <= SP_CLASS_END))
	{
		int offset = ability - SP_CLASS_START;
		if (cp_ptr->flags_special & (0x00000001 << offset)) return(TRUE);
	}

	/*
	 * Check to see if the queried specialty is
	 * on the allowed part of the list
	 */
	for (i = 0; i < p_ptr->specialties_allowed; i++)
	{
		if (p_ptr->specialty_order[i] == ability) return(TRUE);
	}

	/* Assume false */
	return(FALSE);
}

/*
 * Check if we have a specialty ability -BR-
 */
bool check_ability_specialty(int ability)
{
	int i;

	/*
	 * Check to see if the queried specialty is
	 * on the allowed part of the list
	 */
	for (i = 0; i < p_ptr->specialties_allowed; i++)
	{
		if (p_ptr->specialty_order[i] == ability) return(TRUE);
	}

	/* Assume false */
	return(FALSE);
}

/*
 * Check if we can gain a specialty ability -BR-
 */
bool check_specialty_gain(int specialty)
{
	int i;
	bool allowed = FALSE;

	/* Can't learn it if we already know it */
	for (i = 0; i < MAX_SPECIALTIES; i++)
	{
		if (p_ptr->specialty_order[i] == specialty) return(FALSE);
	}

	/* Is it allowed for this class? */
	for (i = 0; i < CLASS_SPECIALTIES; i++)
	{
		if (cp_ptr->specialties[i] == specialty)
		{
			allowed = TRUE;
			break;
		}
	}

	/* return */
	return (allowed);
}

/*
 * Gain a new specialty ability
 * Adapted from birth.c get_player_choice -BR-
 */
void do_cmd_gain_specialty(void)
{
	int top = 0, cur = 0;
	int i, j, k, dir;
	int choices[255];
	int hgt;
	char c;
	char buf[80];
	bool done_one, done_all, use_cur;

        /* Find the next open entry in "specialty_order[]" */
	for (k = 0; k < MAX_SPECIALTIES; k++)
	{
		/* Stop at the first empty space */
		if (p_ptr->specialty_order[k] == SP_NO_SPECIALTY) break;
	}

	/* Check if specialty array is full */
	if (k >= MAX_SPECIALTIES - 1)
	{
		msg_print("Maximum specialties known.");
		return;
	}

	/* Save screen */
	screen_save();

	/* loop until done with all selections or user exit */
	done_all = FALSE;
	while (!done_all)
	{
		/* Clear screen */
		Term_clear();

		/* Find the learnable specialties */
		for (j=0, i=0; i < 255; i++)
		{
			if (check_specialty_gain(i))
			{
				choices[j]=i;
				j++;
			}
		}

		/* We are out of specialties - should never happen */
		if (!j)
		{
			msg_print("No specialties available.");
			screen_load();
			return;
		}

		/* Find height of selection interface */
		hgt = Term->hgt - 7;
		if (hgt > j) hgt = j;

		/* Prompt choices */
	        sprintf(buf, "(Specialties: %c-%c, ESC=exit) Gain which specialty (%d available)? ",
				 I2A(0), I2A(j-1), p_ptr->new_specialties);
		Term_putstr(5, 0, 66, TERM_WHITE, buf);

		/* No valid selection yet */
		use_cur = FALSE;

		/* Make one choice; loop until done */
		done_one = FALSE;
		while (!done_one)
		{
			/* Redraw the list */
			for (i = 0; ((i + top < j) && (i < hgt)); i++)
			{
				if (i + top < 26)
				{
					sprintf(buf, "%c) %s", I2A(i + top), specialty_names[choices[i + top]]);
				}
				else
				{
					/* ToDo: Fix the ASCII dependency */
					sprintf(buf, "%c) %s", 'A' + (i + top - 26), specialty_names[choices[i + top]]);
				}

				/* Clear */
				Term_erase(5, i + 2, 66);

				/* Display */
				Term_putstr(5, i + 2, 66, TERM_GREEN, buf);
			}

			/* Display that specialty's information. */
			roff("",5,0);
			Term_erase(5, hgt + 5, 255);
			Term_erase(5, hgt + 4, 255);
			Term_erase(5, hgt + 3, 255);
			c_roff(TERM_L_BLUE, specialty_tips[choices[cur]], 5, 0);

			/* Move the cursor */
			put_str("", 2 + cur - top, 5);

			/* get input */
			c = inkey();

			/* Numbers are used for scolling */
			if (isdigit(c))
			{
				/* Get a direction from the key */
				dir = target_dir(c);

				/* Going up? */
				if (dir == 8)
				{
					if (cur != 0)
					{
						/* Move selection */
						cur--;
					}

					if ((top > 0) && ((cur - top) < 4))
					{
						/* Scroll up */
						top--;
					}
				}

				/* Going down? */
				if (dir == 2)
				{
					if (cur != (j - 1))
					{
						/* Move selection */
						cur++;
					}

					if ((top + hgt - 1 < (j - 1)) && ((top + hgt - 1 - cur) < 4))
					{
						/* Scroll down */
						top++;
					}
				}
			}

			/* Letters are used for selection */
			else if (isalpha(c))
			{
				int choice;

				if (islower(c))
				{
					choice = A2I(c);
				}
				else
				{
					choice = c - 'A' + 26;
				}

				/* Validate input */
				if ((choice > -1) && (choice < j))
				{
					cur = choice;

					/* Use current */
					use_cur = TRUE;

					/* Done this selection */
					done_one = TRUE;
				}

				else
				{
					bell("Illegal response to question!");
				}

			}

			else if ((c == '\n') || (c == '\r'))
			{
				/* Use current */
				use_cur = TRUE;

				/* Done this selection */
				done_one = TRUE;
			}

			/* Allow user to exit the fuction */
			else if (c == ESCAPE)
			{
				done_one = TRUE;
				done_all = TRUE;
			}

			/* Invalid input */
			else bell("Illegal response to question!");
		}


		if (use_cur)
		{
			if (get_check("Are you sure? "))
			{
				/* Add new specialty */
				p_ptr->specialty_order[k] = choices[cur];

				/* Increment next available slot */
				k++;

				/* Update specialties available count */
				p_ptr->new_specialties--;
				p_ptr->old_specialties = p_ptr->new_specialties;

				/* In case we have more to learn, go to the head of the list */
				cur = 0;

				/* Update some stuff */
				p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPECIALTY | PU_TORCH);

				/* Redraw Status */
				p_ptr->redraw |= (PR_BASIC | PR_STUDY);
			}

			/* Check if we are completely done */
			if ((p_ptr->new_specialties <= 0) || (k >= 9)) done_all = TRUE;
		}

	}

	/* Load screen */
	screen_load();

	/* exit */
	return;
}


/*
 * Interact with specialty abilities -BR-
 */
void do_cmd_specialty(void)
{
	char answer;

	/* Might want to gain a new ability or browse old ones */
	if (p_ptr->new_specialties > 0)
	{
		/* Query */
		/* Interact and choose. */
		while(get_com("'V'iew your abilities or 'L'earn specialty (l/v/ESC)?", &answer))
		{

			/* New ability */
			if ((answer == 'L') || (answer == 'l'))
			{
				do_cmd_gain_specialty();
				return;
			}

			/* View Current */
			if ((answer == 'V') || (answer == 'v'))
			{
				do_cmd_view_abilities();
				return;
			}

			/* Exit */
			else if (answer == ESCAPE) return;


		}

	}

	/* View existing specialties is the only option */
	else
	{
		do_cmd_view_abilities();
		return;
	}

	return;
}
