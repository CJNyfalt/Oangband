/*
 * File: death.c
 * Purpose: Handle the UI bits that happen after the character dies.
 *
 * Copyright (c) 1987 - 2007 Angband contributors
 */
#include "angband.h"

/*
 * Hack - save the time of death
 */
static time_t death_time = (time_t)0;

/*
 * Gets a personalized string for ghosts.  Code originally from get_name. -LM-
 */
static char *get_personalized_string(byte choice)
{
	static char tmp[80];
	byte n, i;


	/* Clear last line */
	clear_from(15);

	/* Prompt and ask */
	if (choice == 1) prt("Enter a message for your character's ghost above, or hit ESCAPE.", 15, 8);
	else if (choice == 2) prt("Enter an addition to your character ghost's description, or hit ESCAPE.", 15, 4);
	else return NULL;

	prt("(79 characters maximum.  Entry will be used as (a) sentence(s).)", 16, 8);

	/* Ask until happy */
	while (1)
	{
		/* Start at beginning of field. */
		move_cursor(14, 0);

		/* Get an input */
		(void)askfor_aux(tmp, 79);

		/* All done */
		break;
	}

	/* Pad the string (to clear junk and allow room for a ending) */
	sprintf(tmp, "%-79.79s", tmp);

	/* Ensure that strings end like a sentence, and neatly clip the string. */
	for (n = 79; ; n--)
	{
		if ((tmp[n] == ' ') || (tmp[n] == '\0')) continue;
		else
		{
			if ((tmp[n] == '!') || (tmp[n] == '.') || (tmp[n] == '?'))
			{
				tmp[n + 1] = '\0';
				for (i = n + 2; i < 80; i++) tmp[i] = '\0';
				break;
			}
			else
			{
				tmp[n + 1] = '.';
				tmp[n + 2] = '\0';
				for (i = n + 3; i < 80; i++) tmp[i] = '\0';
				break;
			}
		}
	}

	/* Start the sentence with a capital letter. */
	if (islower(tmp[0])) tmp[0] = toupper(tmp[0]);

	/* Return the string */
	return tmp;

}




/*
 * Save a "bones" file for a dead character.  Now activated and (slightly)
 * altered.  Allows the inclusion of personalized strings.
 */
static void make_bones(void)
{
	FILE *fp;

	char str[1024];
	char answer;
	byte choice=0;

	int i;

	/* Ignore wizards and borgs */
	if (!(p_ptr->noscore & 0x00FF))
	{
		/* Ignore people who die in town */
		if (p_ptr->depth)
		{
			int level;
			char tmp[128];

			/* Slightly more tenacious saving routine. */
			for (i = 0; i < 5; i++)
			{
				/* Ghost hovers near level of death. */
				if (i == 0) level = p_ptr->depth;
				else level = p_ptr->depth + 5 - damroll(2, 4);
				if (level < 1) level = randint1(4);

				/* XXX XXX XXX "Bones" name */
				sprintf(tmp, "bone.%03d", level);

				/* Build the filename */
				path_build(str, 1024, ANGBAND_DIR_BONE, tmp);

				/* Attempt to open the bones file */
				fp = my_fopen(str, "r");

				/* Close it right away */
				if (fp) my_fclose(fp);

				/* Do not over-write a previous ghost */
				if (fp) continue;

				/* If no file by that name exists, we can make a new one. */
				if (!(fp)) break;
			}

			/* Failure */
			if (fp) return;

			/* File type is "TEXT" */
			FILE_TYPE(FILE_TYPE_TEXT);

			/* Try to write a new "Bones File" */
			fp = my_fopen(str, "w");

			/* Not allowed to write it?  Weird. */
			if (!fp) return;

			/* Save the info */
			if (op_ptr->full_name[0] != '\0') fprintf(fp, "%s\n", op_ptr->full_name);
			else fprintf(fp, "Anonymous\n");

			fprintf(fp, "%d\n", p_ptr->psex);
			fprintf(fp, "%d\n", p_ptr->prace);
			fprintf(fp, "%d\n", p_ptr->pclass);

			/* Clear screen */
			Term_clear();

			while(1)
			{
				/* Ask the player if he wants to
				 * add a personalized string.
				 */
				prt("Information about your character has been saved in a bones file.", 15, 7);

				prt("Would you like to give ", 16, 3);

				if (p_ptr->psex == SEX_MALE) prt("his ", 16, 26);
				else if (p_ptr->psex == SEX_FEMALE) prt("her ", 16, 26);
				else prt("the ", 16, 23);

				prt("ghost a special message or description? (y/n)", 16, 30);

				answer = inkey();

				/* Clear last line */
				clear_from(15);
				clear_from(16);

				/* Determine what the personalized string will be used for.  */
				if ((answer == 'Y') || (answer == 'y'))
				{
					prt("Will you add something for your ghost to say, or add", 15, 13);
					prt("to the monster description? ((M)essage/(D)escription)", 16, 13);

					while(1)
					{
						answer = inkey();

						clear_from(15);
						clear_from(16);

						if ((answer == 'M') || (answer == 'm'))
						{
							choice = 1;
							break;
						}
						else if ((answer == 'D') || (answer == 'd'))
						{
							choice = 2;
							break;
						}
						else if (answer == ESCAPE)
						{
							choice = 0;
							break;
						}
						else
						{
							prt("Please press 'm' to include a message, 'd' to include", 15, 13);
							prt("a description, or the Escape key to cancel.", 16, 18);
						}
					}
				}
				else if ((answer == 'N') || (answer == 'n') || (answer == ESCAPE))
				{
					choice = 0;
					break;
				}


				/* If requested, get the personalized string, and write it and
				 * info on how it should be used in the bones file.  Otherwise,
				 * indicate the absence of such a string.
				 */
				if (choice) fprintf(fp, "%d:%s\n",
				       choice, get_personalized_string(choice));
				else fprintf(fp, "0: \n");

				/* Close and save the Bones file */
				my_fclose(fp);

				return;
			}
		}
	}
}




/*
 * Display the tombstone
 */
static void print_tomb(void)
{
	const char *p;

	char tmp[160];

	ang_file *fp;
	char buf[1024];


	time_t ct = time((time_t)0);

	/* Clear screen */
	Term_clear();

	/* Open the death file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "dead.txt");
	fp = file_open(buf, MODE_READ, -1);

	/* Dump */
	if (fp)
	{
		int i=0;

		/* Dump the file to the screen */
		while (file_getl(fp, buf, sizeof(buf)))
		{
			/* Display and advance */
			put_str(buf, i++, 0);
		}

		file_close(fp);
	}


	/* King or Queen */
	if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = "Magnificent";
	}

	/* Normal */
	else
	{
		p = cp_text + cp_ptr->title[(p_ptr->lev - 1) / 5];
	}

	center_string(buf, op_ptr->full_name);
	put_str(buf, 6, 11);

	center_string(buf, "the");
	put_str(buf, 7, 11);

	center_string(buf, p);
	put_str(buf, 8, 11);


	center_string(buf, cp_name + cp_ptr->name);
	put_str(buf, 10, 11);

	sprintf(tmp, "Level: %d", (int)p_ptr->lev);
	center_string(buf, tmp);
	put_str(buf, 11, 11);

	sprintf(tmp, "Exp: %ld", (long)p_ptr->exp);
	center_string(buf, tmp);
	put_str(buf, 12, 11);

	sprintf(tmp, "AU: %ld", (long)p_ptr->au);
	center_string(buf, tmp);
	put_str(buf, 13, 11);

	sprintf(tmp, "Killed on Level %d", p_ptr->depth);
	center_string(buf, tmp);
	put_str(buf, 14, 11);

	sprintf(tmp, "by %s.", p_ptr->died_from);
	center_string(buf, tmp);
	put_str(buf, 15, 11);


	sprintf(tmp, "%-.24s", ctime(&ct));
	center_string(buf, tmp);
	put_str(buf, 17, 11);
}


/*
 * Know inventory and home items upon death
 */
static void death_knowledge(void)
{
	int i;

	object_type *o_ptr;

	store_type *st_ptr = &store[STORE_HOME];


	/* Hack -- Know everything in the inven/equip */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Fully known */
		o_ptr->ident |= (IDENT_MENTAL);
	}

	/* Hack -- Know everything in the home */
	for (i = 0; i < st_ptr->stock_num; i++)
	{
		o_ptr = &st_ptr->stock[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Fully known */
		o_ptr->ident |= (IDENT_MENTAL);
	}

	/* Hack -- Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();
}

/*
 * Change the player into a Winner
 */
static void kingly(void)
{
	char buf[1024];
	ang_file *fp;

	int wid, hgt;
	int i = 2;
	int width = 0;

	/* Hack -- retire in town */
	p_ptr->depth = 0;

	/* Fake death */
	strcpy(p_ptr->died_from, "Ripe Old Age");

	/* Restore the experience */
	p_ptr->exp = p_ptr->max_exp;

	/* Restore the level */
	p_ptr->lev = p_ptr->max_lev;

	/* Hack -- Instant Gold */
	p_ptr->au += 10000000L;

	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "crown.txt");
	fp = file_open(buf, MODE_READ, -1);


	/* Clear screen */
	Term_clear();
	Term_get_size(&wid, &hgt);

	if (fp)
	{
		/* Get us the first line of file, which tells us how long the */
		/* longest line is */
		file_getl(fp, buf, sizeof(buf));
		sscanf(buf, "%d", &width);
		if (!width) width = 25;

		/* Dump the file to the screen */
		while (file_getl(fp, buf, sizeof(buf)))
			put_str(buf, i++, (wid/2) - (width/2));

		file_close(fp);
	}

	put_str(format("All Hail the Mighty %s!", sp_ptr->winner), i, 22);

	/* Flush input */
	flush();

	/* Wait for response */
	pause_line(23);
}



/*
 * Display some character info
 */
static void show_info(void)
{
	int i, j, k;

	object_type *o_ptr;

	store_type *st_ptr = &store[STORE_HOME];

	screen_save();

	/* Display player */
	display_player(0);

	/* Prompt for inventory */
	prt("Hit any key to see more information (ESC to abort): ", 23, 0);

	/* Allow abort at this point */
	if (inkey() == ESCAPE) return;

	/* Show equipment and inventory */

	/* Equipment -- if any */
	if (p_ptr->equip_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_equip();
		prt("You are using: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Inventory -- if any */
	if (p_ptr->inven_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_inven();
		prt("You are carrying: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}



	/* Home -- if anything there */
	if (st_ptr->stock_num)
	{
		/* Display contents of the home */
		for (k = 0, i = 0; i < st_ptr->stock_num; k++)
		{
			/* Clear screen */
			Term_clear();

			/* Show 12 items */
			for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++)
			{
				byte attr;

				char o_name[80];
				char tmp_val[80];

				/* Get the object */
				o_ptr = &st_ptr->stock[i];

				/* Print header, clear line */
				strnfmt(tmp_val, sizeof(tmp_val), "%c) ", I2A(j));
				prt(tmp_val, j+2, 4);

				/* Get the object description */
				object_desc(o_name, o_ptr, TRUE, 3);

				/* Get the inventory color */
				attr = tval_to_attr[o_ptr->tval & 0x7F];

				/* Display the object */
				c_put_str(attr, o_name, j+2, 7);
			}

			/* Caption */
			prt(format("Your home contains (page %d): -more-", k+1), 0, 0);

			/* Wait for it */
			if (inkey() == ESCAPE) return;
		}
	}

	screen_load();
}



/*
 * Special version of 'do_cmd_examine'
 */
static void death_examine(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	const char *q, *s;


	/* Start out in "display" mode */
	p_ptr->command_see = TRUE;

	/* Get an item */
	q = "Examine which item? ";
	s = "You have nothing to examine.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP))) return;

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

	/* Fully known */
	o_ptr->ident |= (IDENT_MENTAL);

	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	msg("Examining %s...", o_name);

	/* Describe it fully */
	do_cmd_observe(o_ptr, FALSE);
}


/*
 * Handle character death
 */
void close_game_aux(void)
{
	int ch;

	const char *p;

	/* Flush all input keys */
	flush();

	/* Dump bones file */
	make_bones();

	/* Prompt */
	p = "['i' for info, 'f' to file, 't' for scores, 'x' to examine, or ESC]";

	/* Handle retirement */
	if (p_ptr->total_winner) kingly();

	/* Get time of death */
	(void)time(&death_time);

	/* You are dead */
	print_tomb();

	/* Hack - Know everything upon death */
	death_knowledge();

	/* Enter player in high score list */
	enter_score(&death_time);

	/* Flush all input keys */
	flush();

	/* Flush messages */
	message_flush();

	/* Forever */
	while (1)
	{
		/* Describe options */
		Term_putstr(2, 23, -1, TERM_WHITE, p);

		/* Query */
		ch = inkey();

		/* Exit */
		if (ch == ESCAPE)
		{
			if (get_check("Do you want to quit? ")) break;
		}

		/* File dump */
		else if (ch == 'f')
		{
			char ftmp[80];

			sprintf(ftmp, "%s.txt", op_ptr->base_name);

			if (get_string("File name: ", ftmp, 80))
			{
				if (ftmp[0] && (ftmp[0] != ' '))
				{
					errr err;

					/* Save screen */
					screen_save();

					/* Dump a character file */
					err = file_character(ftmp, FALSE);

					/* Load screen */
					screen_load();

					/* Check result */
					if (err)
					{
						msg_print("Character dump failed!");
					}
					else
					{
						msg_print("Character dump successful.");
					}

					/* Flush messages */
					message_flush();
				}
			}
		}

		/* Show more info */
		else if (ch == 'i')
		{
			/* Save screen */
			screen_save();

			/* Show the character */
			show_info();

			/* Load screen */
			screen_load();
		}

		/* Show top scores */
		else if (ch == 't')
		{
			/* Save screen */
			screen_save();

			/* Show the scores */
			show_scores();


			/* Load screen */
			screen_load();
		}

		/* Examine an item */
		else if (ch == 'x')
		{
			death_examine();
		}
	}


	/* Save dead player */
	if (!save_player())
	{
		msg("death save failed!");
		message_flush();
	}

}




