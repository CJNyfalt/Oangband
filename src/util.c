/* File: util.c */

/* Functions, etc. for various machines that need them.  Basic text-handing
 * functions.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


#ifdef PRIVATE_USER_PATH

/*
 * Create an ".angband/" directory in the users home directory.
 *
 * ToDo: Add error handling.
 * ToDo: Only create the directories when actually writing files.
 */
void create_user_dirs(void)
{
	char dirpath[1024];
	char subdirpath[1024];


	/* Get an absolute path from the filename */
	path_build(dirpath, sizeof(dirpath), PRIVATE_USER_PATH, "");

	/* Create the ~/.angband/ directory */
	mkdir(dirpath, 0700);

	/* Build the path to the variant-specific sub-directory */
	path_build(subdirpath, sizeof(subdirpath), dirpath, VERSION_NAME);

	/* Create the directory */
	mkdir(subdirpath, 0700);

#ifdef USE_PRIVATE_PATHS
	/* Build the path to the scores sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "scores");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "bone");

	/* Create the directory */
	mkdir(dirpath, 0700);

	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "data");

	/* Create the directory */
	mkdir(dirpath, 0700);

#endif /* USE_PRIVATE_PATHS */
	/* Build the path to the savefile sub-directory */
	path_build(dirpath, sizeof(dirpath), subdirpath, "save");

	/* Create the directory */
	mkdir(dirpath, 0700);
}

#endif /* PRIVATE_USER_PATH */



#ifdef SET_UID

# ifndef HAVE_USLEEP

/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
int usleep(unsigned long usecs)
{
	struct timeval      Timer;

	/* Paranoia -- No excessive sleeping */
	if (usecs > 4000000L) quit("Illegal usleep() call");

	/* Wait for it */
	Timer.tv_sec = (usecs / 1000000L);
	Timer.tv_usec = (usecs % 1000000L);

	/* Wait for it */
	if (select(0, NULL, NULL, NULL, &Timer) < 0)
	{
		/* Hack -- ignore interrupts */
		if (errno != EINTR) return -1;
	}

	/* Success */
	return 0;
}

# endif


/*
 * Find a default user name from the system.
 */
void user_name(char *buf, int id)
{
	struct passwd *pw;

	/* Look up the user name */
	if ((pw = getpwuid(id)))
	{
		strcpy(buf, pw->pw_name);
		buf[16] = '\0';

#ifdef CAPITALIZE_USER_NAME
		/* Hack -- capitalize the user name */
		if (islower(buf[0])) buf[0] = toupper(buf[0]);
#endif

		return;
	}

	/* Oops.  Hack -- default to "PLAYER" */
	strcpy(buf, "PLAYER");
}

#endif /* SET_UID */




#ifdef CHECK_MODIFICATION_TIME

errr check_modification_date(const char *raw_file, const char *template_file)
{
	char buf[1024];

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, template_file);

	/* If text file is newer than raw, force reprocessing */
	if(file_newer(buf, raw_file))
		return (-1);
	/* Otherwise do nothing */
	else
		return 0;
}

#endif /* CHECK_MODIFICATION_TIME */



/*
 * Flush all pending input.
 *
 * Actually, remember the flush, using the "inkey_xtra" flag, and in the
 * next call to "inkey()", perform the actual flushing, for efficiency,
 * and correctness of the "inkey()" function.
 */
void flush(void)
{
	/* Do it later */
	inkey_xtra = TRUE;
}




/*
 * Helper function called only from "inkey()"
 */
static ui_event inkey_aux(void)
{
	int w = 0;

	ui_event ke;

	/* Wait for a keypress */
	if (TRUE)
	{
		(void)(Term_inkey(&ke, TRUE, TRUE));
	}
	else
	{
		w = 0;

		/* Wait only as long as macro activation would wait*/
		while (Term_inkey(&ke, FALSE, TRUE) != 0)
		{
			/* Increase "wait" */
			w++;

			/* Excessive delay */
			if (w >= 100)
			{
				ui_event empty = EVENT_EMPTY;
				return empty;
			}

			/* Delay */
			Term_xtra(TERM_XTRA_DELAY, 10);
		}
	}

	return (ke);
}



/*
 * Mega-Hack -- special "inkey_next" pointer.  XXX XXX XXX
 *
 * This special pointer allows a sequence of keys to be "inserted" into
 * the stream of keys returned by "inkey()".  This key sequence cannot be
 * bypassed by the Borg.  We use it to implement keymaps.
 */
const char *inkey_next = NULL;


#ifdef ALLOW_BORG

/*
 * Mega-Hack -- special "inkey_hack" hook.  XXX XXX XXX
 *
 * This special function hook allows the "Borg" (see elsewhere) to take
 * control of the "inkey()" function, and substitute in fake keypresses.
 */
struct keypress (*inkey_hack)(int flush_first) = NULL;

#endif /* ALLOW_BORG */


/*
 * Get a keypress from the user.
 *
 * This function recognizes a few "global parameters".  These are variables
 * which, if set to TRUE before calling this function, will have an effect
 * on this function, and which are always reset to FALSE by this function
 * before this function returns.  Thus they function just like normal
 * parameters, except that most calls to this function can ignore them.
 *
 * If "inkey_xtra" is TRUE, then all pending keypresses will be flushed.
 * This is set by flush(), which doesn't actually flush anything itself
 * but uses that flag to trigger delayed flushing.
 *
 * If "inkey_scan" is TRUE, then we will immediately return "zero" if no
 * keypress is available, instead of waiting for a keypress.
 *
 * If "inkey_flag" is TRUE, then we are waiting for a command in the main
 * map interface, and we shouldn't show a cursor.
 *
 * If we are waiting for a keypress, and no keypress is ready, then we will
 * refresh (once) the window which was active when this function was called.
 *
 * Note that "back-quote" is automatically converted into "escape" for
 * convenience on machines with no "escape" key.
 *
 * If "angband_term[0]" is not active, we will make it active during this
 * function, so that the various "main-xxx.c" files can assume that input
 * is only requested (via "Term_inkey()") when "angband_term[0]" is active.
 *
 * Mega-Hack -- This function is used as the entry point for clearing the
 * "signal_count" variable, and of the "character_saved" variable.
 *
 * Mega-Hack -- Note the use of "inkey_hack" to allow the "Borg" to steal
 * control of the keyboard from the user.
 */
ui_event inkey_ex(void)
{
	bool cursor_state;
	ui_event kk;
	ui_event ke = EVENT_EMPTY;

	bool done = FALSE;

	term *old = Term;

	/* Delayed flush */
	if (inkey_xtra) {
		Term_flush();
		inkey_next = NULL;
		inkey_xtra = FALSE;
	}

	/* Hack -- Use the "inkey_next" pointer */
	if (inkey_next && *inkey_next)
	{
		/* Get next character, and advance */
		ke.key.code = *inkey_next++;
		ke.type = EVT_KBRD;

		/* Cancel the various "global parameters" */
		inkey_flag = inkey_scan = FALSE;

		/* Accept result */
		return (ke);
	}

	/* Forget pointer */
	inkey_next = NULL;

#ifdef ALLOW_BORG

	/* Mega-Hack -- Use the special hook */
	if (inkey_hack && ((ke.key = (*inkey_hack)(inkey_xtra)) != 0))
	{
		/* Cancel the various "global parameters" */
		inkey_flag = inkey_scan = FALSE;
		ke.type = EVT_KBRD;

		/* Accept result */
		return (ke);
	}

#endif /* ALLOW_BORG */


	/* Get the cursor state */
	(void)Term_get_cursor(&cursor_state);

	/* Show the cursor if waiting, except sometimes in "command" mode */
	if (!inkey_scan && (!inkey_flag || character_icky))
		(void)Term_set_cursor(TRUE);


	/* Hack -- Activate main screen */
	Term_activate(term_screen);


	/* Get a key */
	while (ke.type == EVT_NONE)
	{
		/* Hack -- Handle "inkey_scan" */
		if (inkey_scan &&
		    (0 != Term_inkey(&kk, FALSE, FALSE)))
			break;


		/* Hack -- Flush output once when no key ready */
		if (!done && (0 != Term_inkey(&kk, FALSE, FALSE)))
		{
			/* Hack -- activate proper term */
			Term_activate(old);

			/* Flush output */
			Term_fresh();

			/* Hack -- activate main screen */
			Term_activate(term_screen);

			/* Mega-Hack -- reset saved flag */
			character_saved = FALSE;

			/* Mega-Hack -- reset signal counter */
			signal_count = 0;

			/* Only once */
			done = TRUE;
		}


		/* Get a key (see above) */
		ke = inkey_aux();


		/* Treat back-quote as escape */
		if (ke.key.code == '`')
			ke.key.code = ESCAPE;
	}


	/* Hack -- restore the term */
	Term_activate(old);


	/* Restore the cursor */
	Term_set_cursor(cursor_state);


	/* Cancel the various "global parameters" */
	inkey_flag = inkey_scan = FALSE;


	/* Return the keypress */
	return (ke);
}


/*
 * Get a keypress or mouse click from the user.
 */
void anykey(void)
{
	ui_event ke = EVENT_EMPTY;

	/* Only accept a keypress or mouse click */
	while (ke.type != EVT_MOUSE && ke.type != EVT_KBRD)
		ke = inkey_ex();
}

/*
 * Get a "keypress" from the user.
 */
char inkey(void)
{
	ui_event ke = EVENT_EMPTY;

	/* Only accept a keypress */
	while (ke.type != EVT_ESCAPE && ke.type != EVT_KBRD)
		ke = inkey_ex();

	/* Paranoia */
	if (ke.type == EVT_ESCAPE) {
		ke.type = EVT_KBRD;
		ke.key.code = ESCAPE;
		ke.key.mods = 0;
	}

	return ke.key.code;
}



/*
 * Flush the screen, make a noise
 */
void bell(const char *reason)
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Hack -- memorize the reason if possible */
	if (character_generated && reason) message_add(reason, MSG_BELL);

	/* Make a bell noise (if allowed) */
	if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
}


/*
 * Hack -- Make a (relevant?) sound
 */
void sound(int val)
{
	/* No sound */
	if (!use_sound) return;

	/* Make a sound (if allowed) */
	/* Old sound interface removed, add new here */
}





/*
 * Move the cursor
 */
void move_cursor(int row, int col)
{
	Term_gotoxy(col, row);
}



/*
 * Hack -- flush
 */
static void msg_flush(int x)
{
	byte a = TERM_L_BLUE;

	/* Pause for response */
	Term_putstr(x, 0, -1, a, "-more-");

	/* Get an acceptable keypress */
	if (!auto_more)
	{
		while (1)
		{
			int cmd = inkey();
			if (quick_messages) break;
			if ((cmd == ESCAPE) || (cmd == ' ')) break;
			if ((cmd == '\n') || (cmd == '\r')) break;
			bell("Illegal response to a 'more' prompt!");
		}
	}

	/* Clear the line */
	Term_erase(0, 0, 255);
}


static int message_column = 0;


/*
 * Output a message to the top line of the screen.
 *
 * Break long messages into multiple pieces (40-72 chars).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are memorized for later reference (see above).
 *
 * We could do a "Term_fresh()" to provide "flicker" if needed.
 *
 * The global "msg_flag" variable can be cleared to tell us to "erase" any
 * "pending" messages still on the screen, instead of using "msg_flush()".
 * This should only be done when the user is known to have read the message.
 *
 * We must be very careful about using the "msg("%s", )" functions without
 * explicitly calling the special "msg("%s", NULL)" function, since this may
 * result in the loss of information if the screen is cleared, or if anything
 * is displayed on the top line.
 *
 * Hack -- Note that "msg("%s", NULL)" will clear the top line even if no
 * messages are pending.
 */
static void msg_print_aux(u16b type, const char *msg)
{
	int n;
	char *t;
	char buf[1024];
	byte color = TERM_WHITE;
	int w, h;

	if (!Term)
		return;

	/* Obtain the size */
	(void)Term_get_size(&w, &h);

	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Message Length */
	n = (msg ? strlen(msg) : 0);

	/* Hack -- flush when requested or needed */
	if (message_column && (!msg || ((message_column + n) > (w - 8))))
	{
		/* Flush */
		msg_flush(message_column);

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		message_column = 0;
	}


	/* No message */
	if (!msg) return;

	/* Paranoia */
	if (n > 1000) return;


	/* Memorize the message (if legal) */
	if (character_generated && !(p_ptr->is_dead))
		message_add(msg, type);

	/* Window stuff */
	p_ptr->window |= (PW_MESSAGE);

	/* Copy it */
	my_strcpy(buf, msg, sizeof(buf));

	/* Analyze the buffer */
	t = buf;

	/* Get the color of the message */
	color = message_type_color(type);

	/* HACK -- no "black" messages */
	if (color == TERM_DARK) color = TERM_WHITE;

	/* Split message */
	while (n > w - 1)
	{
		char oops;

		int check, split;

		/* Default split */
		split = w - 8;

		/* Find the rightmost split point */
		for (check = (w / 2); check < w - 8; check++)
			if (t[check] == ' ') split = check;

		/* Save the split character */
		oops = t[split];

		/* Split the message */
		t[split] = '\0';

		/* Display part of the message */
		Term_putstr(0, 0, split, color, t);

		/* Flush it */
		msg_flush(split + 1);

		/* Restore the split character */
		t[split] = oops;

		/* Insert a space */
		t[--split] = ' ';

		/* Prepare to recurse on the rest of "buf" */
		t += split; n -= split;
	}

	/* Display the tail of the message */
	Term_putstr(message_column, 0, n, color, t);

	/* Remember the message */
	msg_flag = TRUE;

	/* Remember the position */
	message_column += n + 1;

	/* Optional refresh */
	if (fresh_after) Term_fresh();
}


/*
 * Print a message in the default color (white)
 */
void msg_print(const char *msg)
{
	msg_print_aux(MSG_GENERIC, msg);
}


/*
 * Display a formatted message, using "vstrnfmt()" and "msg("%s", )".
 */
void msg(const char *fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	msg_print_aux(MSG_GENERIC, buf);
}


/*
 * Display a formatted message and play the associated sound.
 */
void msgt(unsigned int type, const char *fmt, ...)
{
	va_list vp;
	char buf[1024];

	va_start(vp, fmt);
	vstrnfmt(buf, sizeof(buf), fmt, vp);
	va_end(vp);
	sound(type);
	msg_print_aux(type, buf);
}

/*
 * Print the queued messages.
 */
void message_flush(void)
{
	/* Hack -- Reset */
	if (!msg_flag) message_column = 0;

	/* Flush when needed */
	if (message_column)
	{
		/* Print pending messages */
		if (Term)
			msg_flush(message_column);

		/* Forget it */
		msg_flag = FALSE;

		/* Reset */
		message_column = 0;
	}
}



/*
 * Save the screen, and increase the "icky" depth.
 *
 * This function must match exactly one call to "screen_load()".
 */
void screen_save(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Save the screen (if legal) */
	Term_save();

	/* Increase "icky" depth */
	character_icky++;
}


/*
 * Load the screen, and decrease the "icky" depth.
 *
 * This function must match exactly one call to "screen_save()".
 */
void screen_load(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Load the screen (if legal) */
	Term_load();

	/* Decrease "icky" depth */
	character_icky--;

	/* Mega hack -redraw big graphics - sorry NRM */
	if (character_icky == 0 && (tile_width > 1 || tile_height > 1))
		Term_redraw();
}


/*
 * Display a string on the screen using an attribute.
 *
 * At the given location, using the given attribute, if allowed,
 * add the given string.  Do not clear the line.
 */
void c_put_str(byte attr, const char *str, int row, int col)
{
	/* Position cursor, Dump the attr/text */
	Term_putstr(col, row, -1, attr, str);
}


/*
 * As above, but in "white"
 */
void put_str(const char *str, int row, int col)
{
	/* Spawn */
	Term_putstr(col, row, -1, TERM_WHITE, str);
}


/*
 * As above, but centered horizontally
 */
void put_str_center(const char *str, int row)
{
	int len = strlen(str);
	int col = (Term->wid - len) / 2;

	Term_putstr(col, row, -1, TERM_WHITE, str);
}


/*
 * Display a string on the screen using an attribute, and clear
 * to the end of the line.
 */
void c_prt(byte attr, const char *str, int row, int col)
{
	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr(-1, attr, str);
}


/*
 * As above, but in "white"
 */
void prt(const char *str, int row, int col)
{
	/* Spawn */
	c_prt(TERM_WHITE, str, row, col);
}


/*
 * As above, but in "white"
 */
void prt_center(const char *str, int row)
{
	int len = strlen(str);
	int col = (Term->wid - len) / 2;

	/* Spawn */
	c_prt(TERM_WHITE, str, row, col);
}



/*
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line.  Also, allow "newline" to force
 * a "wrap" to the next line.  Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Accept values for left and right margins. -LM-
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "c_roff()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void c_roff(byte a, const char *str, byte l_margin, byte r_margin)
{
	int x, y;

	int w, h;

	const char * s;


	/* Obtain the size */
	(void)Term_get_size(&w, &h);

	/* Accept right margin if provided and legal. */
	if ((r_margin) && (r_margin <= w)) w = r_margin;

	/* Obtain the cursor. */
	(void)Term_locate(&x, &y);

	/* If cursor is left of the left margin,... */
	if ((l_margin) && (x < l_margin))
	{
		/* ...move it to the left margin. */
		move_cursor(y, l_margin);

		/* Increment column count. */
		x = l_margin;
	}


	/* Process the string */
	for (s = str; *s; s++)
	{
		char ch;

		/* Force wrap */
		if (*s == '\n')
		{
			/* Wrap */
			x = l_margin;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			/* Go immediately to the next character. */
			continue;
		}

		/* Clean up the char */
		ch = (isprint(*s) ? *s : ' ');

		/* Wrap words as needed */
		if ((x >= w - 1) && (ch != ' '))
		{
			int i, n = l_margin;

			byte av[256];
			char cv[256];

			/* Wrap word */
			if (x < w)
			{
				/* Scan existing text */
				for (i = w - 2; i >= l_margin; i--)
				{
					/* Grab existing attr/char */
					Term_what(i, y, &av[i], &cv[i]);

					/* Break on space */
					if (cv[i] == ' ') break;

					/* Track current word */
					n = i;
				}
			}

			/* Special case */
			if (n == l_margin) n = w;

			/* Clear line */
			Term_erase(n, y, 255);

			/* Wrap */
			x = l_margin;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			/* Wrap the word (if any) */
			for (i = n; i < w - 1; i++)
			{
				/* Dump */
				Term_addch(av[i], cv[i]);

				/* Advance (no wrap) */
				if (++x > w) x = w;
			}
		}

		/* Dump */
		Term_addch(a, ch);

		/* Advance. */
		if (++x > w) x = w;
	}
}


/*
 * As above, but in "white"
 */
void roff(const char *str, byte l_margin, byte r_margin)
{
	/* Spawn */
	c_roff(TERM_WHITE, str, l_margin, r_margin);
}




/*
 * Clear part of the screen
 */
void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}




/*
 * Get some input at the cursor location.
 * Assume the buffer is initialized to a default string.
 * Note that this string is often "empty" (see below).
 * The default buffer is displayed in yellow until cleared.
 * Pressing RETURN right away accepts the default entry.
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 * The buffer must be large enough for 'len+1' characters.
 */
bool askfor_aux(char *buf, int len)
{
	int y, x;

	int i = 0;

	int k = 0;

	bool done = FALSE;


	/* Locate the cursor */
	Term_locate(&x, &y);


	/* Paranoia -- check len */
	if (len < 1) len = 1;

	/* Paranoia -- check column */
	if ((x < 0) || (x >= 80)) x = 0;

	/* Restrict the length */
	if (x + len > 80) len = 80 - x;


	/* Paranoia -- Clip the default entry */
	buf[len-1] = '\0';


	/* Display the default answer */
	Term_erase(x, y, len);
	Term_putstr(x, y, -1, TERM_YELLOW, buf);


	/* Process input */
	while (!done)
	{
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		i = inkey();

		/* Analyze the key */
		switch (i)
		{
			case ESCAPE:
			{
				k = 0;
				done = TRUE;
				break;
			}

			case '\n':
			case '\r':
			{
				k = strlen(buf);
				done = TRUE;
				break;
			}

			case 0x7F:
			case '\010':
			{
				if (k > 0) k--;
				break;
			}

			default:
			{
				if ((k < len - 1) && (isprint(i)))
				{
					buf[k++] = i;
				}
				else
				{
					bell("Illegal edit key!");
				}
				break;
			}
		}

		/* Terminate */
		buf[k] = '\0';

		/* Update the entry */
		Term_erase(x, y, len);
		Term_putstr(x, y, -1, TERM_WHITE, buf);
	}

	/* Aborted */
	if (i == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Get a string from the user
 *
 * The "prompt" should take the form "Prompt: "
 *
 * Note that the initial contents of the string is used as
 * the default response, so be sure to "clear" it if needed.
 *
 * We clear the input, and return FALSE, on "ESCAPE".
 */
bool get_string(const char * prompt, char *buf, int len)
{
	bool res;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}



/*
 * Request a "quantity" from the user
 *
 * Allow "p_ptr->command_arg" to specify a quantity
 */
s16b get_quantity(const char *prompt, int max)
{
	int amt = 1;


	/* Use "command_arg" */
	if (p_ptr->command_arg)
	{
		/* Extract a number */
		amt = p_ptr->command_arg;

		/* Clear "command_arg" */
		p_ptr->command_arg = 0;
	}
#ifdef ALLOW_REPEAT /* TNB */

		/* Get the item index */
		else if ((max != 1) && repeat_pull(&amt)) {
		}

#endif /* ALLOW_REPEAT */


	/* Prompt if needed */
	else if (max != 1)
	{
		char tmp[80];

		char buf[80];

		/* Build a prompt if needed */
		if (!prompt)
		{
			/* Build a prompt */
			strnfmt(tmp, sizeof(tmp), "Quantity (0-%d): ", max);

			/* Use that prompt */
			prompt = tmp;
		}

		/* Build the default */
		strnfmt(buf, sizeof(buf), "%d", amt);

		/* Ask for a quantity */
		if (!get_string(prompt, buf, 6)) return (0);

		/* Extract a number */
		amt = atoi(buf);

		/* A letter means "all" */
		if (isalpha(buf[0])) amt = max;
	}

	/* Enforce the maximum */
	if (amt > max) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

#ifdef ALLOW_REPEAT /* TNB */

	if (amt) repeat_push(amt);

#endif /* ALLOW_REPEAT */

	/* Return the result */
	return (amt);
}


/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check(const char *prompt)
{
	int i;

	char buf[80];

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[y/n] ", prompt);

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (TRUE)
	{
		i = inkey();
		if (quick_messages) break;
		if (i == ESCAPE) break;
		if (strchr("YyNn", i)) break;
		bell("Illegal response to a 'yes/no' question!");
	}

	/* Erase the prompt */
	prt("", 0, 0);

	/* Normal negation */
	if ((i != 'Y') && (i != 'y')) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(const char * prompt, char *command)
{
	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display a prompt */
	prt(prompt, 0, 0);

	/* Get a key */
	*command = inkey();

	/* Clear the prompt */
	prt("", 0, 0);

	/* Handle "cancel" */
	if (*command == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Pause for user response
 *
 * This function is stupid.  XXX XXX XXX
 */
void pause_line(int row)
{
	int i;
	prt("", row, 0);
	put_str("[Press any key to continue]", row, 23);
	i = inkey();
	prt("", row, 0);
}




/*
 * Same as above, but always maximal
 */
unsigned int maxroll(unsigned int num, unsigned int sides)
{
	return (num * sides);
}




/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (ch)
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		return (TRUE);
	}

	return (FALSE);
}



#if 0

/*
 * Replace the first instance of "target" in "buf" with "insert"
 * If "insert" is NULL, just remove the first instance of "target"
 * In either case, return TRUE if "target" is found.
 *
 * XXX Could be made more efficient, especially in the
 * case where "insert" is smaller than "target".
 */
static bool insert_str(char *buf, const char * target, const char * insert)
{
	int i, len;
	int b_len, t_len, i_len;

	/* Attempt to find the target (modify "buf") */
	buf = strstr(buf, target);

	/* No target found */
	if (!buf) return (FALSE);

	/* Be sure we have an insertion string */
	if (!insert) insert = "";

	/* Extract some lengths */
	t_len = strlen(target);
	i_len = strlen(insert);
	b_len = strlen(buf);

	/* How much "movement" do we need? */
	len = i_len - t_len;

	/* We need less space (for insert) */
	if (len < 0)
	{
		for (i = t_len; i < b_len; ++i) buf[i+len] = buf[i];
	}

	/* We need more space (for insert) */
	else if (len > 0)
	{
		for (i = b_len-1; i >= t_len; --i) buf[i+len] = buf[i];
	}

	/* If movement occured, we need a new terminator */
	if (len) buf[b_len+len] = '\0';

	/* Now copy the insertion string */
	for (i = 0; i < i_len; ++i) buf[i] = insert[i];

	/* Successful operation */
	return (TRUE);
}


#endif



#define REPEAT_MAX             20

/* Number of chars saved */
static int repeat__cnt = 0;

/* Current index */
static int repeat__idx = 0;

/* Saved "stuff" */
static int repeat__key[REPEAT_MAX];

void repeat_push(int what)
{
	/* Too many keys */
	if (repeat__cnt == REPEAT_MAX) return;

	/* Push the "stuff" */
	repeat__key[repeat__cnt++] = what;

	/* Prevents us from pulling keys */
	++repeat__idx;
}

bool repeat_pull(int *what)
{
	/* All out of keys */
	if (repeat__idx == repeat__cnt) return (FALSE);

	/* Grab the next key, advance */
	*what = repeat__key[repeat__idx++];

	/* Success */
	return (TRUE);
}

void repeat_clear()
{
	/* Start over from the failed pull */
	if (repeat__idx) repeat__cnt = --repeat__idx;

	/* Paranioa */
	else repeat__cnt = repeat__idx;

	return;
}

/*
 * From Tim Baker's Easy Patch.
 */
void repeat_check(void)
{
	int what;

	/* Ignore some commands */
	if (p_ptr->command_cmd == ESCAPE) return;
	if (p_ptr->command_cmd == ' ') return;
	if (p_ptr->command_cmd == '\r') return;
	if (p_ptr->command_cmd == '\n') return;

	/* Repeat Last Command */
	if (p_ptr->command_cmd == 'n')
	{
		/* Reset */
		repeat__idx = 0;

		/* Get the command */
		if (repeat_pull(&what))
		{
			/* Save the command */
			p_ptr->command_cmd = what;
		}
	}

	/* Start saving new command */
	else
	{
		/* Reset */
		repeat__cnt = 0;
		repeat__idx = 0;

		what = p_ptr->command_cmd;

		/* Save this command */
		repeat_push(what);
	}
}


/*
 * Convert an input from tenths of a pound to tenths of a kilogram. -LM-
 */
int make_metric(int wgt)
{
	int metric_wgt;

	/* Convert to metric values, using normal rounding. */
	metric_wgt = wgt * 10 / 22;
	if ((wgt * 10) % 22 > 10) metric_wgt++;

	return metric_wgt;
}


/*
 * Accept values for y and x (considered as the endpoints of lines) between
 * 0 and 40, and return an angle in degrees (divided by two). -LM-
 *
 * This table's input and output needs some processing:
 *
 * Because this table gives degrees for a whole circle, up to radius 20, its
 * origin is at (x,y) = (20, 20).  Therefore, the input code needs to find
 * the origin grid (where the lines being compared come from), and then map
 * it to table grid 20,20.  Do not, however, actually try to compare the
 * angle of a line that begins and ends at the origin with any other line -
 * it is impossible mathematically, and the table will return the value "255".
 *
 * The output of this table also needs to be massaged, in order to avoid the
 * discontinuity at 0/180 degrees.  This can be done by:
 *   rotate = 90 - first value
 *   this rotates the first input to the 90 degree line)
 *   tmp = ABS(second value + rotate) % 180
 *   diff = ABS(90 - tmp) = the angular difference (divided by two) between
 *   the first and second values.
 */
byte get_angle_to_grid[41][41] =
{
	{  67,  66,  66,  66,  66,  66,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  48,  46,  45,  44,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  23,  23,  23,  23,  22 },
	{  68,  67,  66,  66,  66,  66,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  49,  47,  45,  43,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  23,  23,  23,  22,  21 },
	{  68,  68,  67,  66,  66,  66,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  49,  47,  45,  43,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  23,  23,  22,  21,  21 },
	{  68,  68,  68,  67,  66,  66,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  49,  47,  45,  43,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  23,  22,  21,  21,  21 },
	{  68,  68,  68,  68,  67,  66,  66,  65,  63,  62,  61,  60,  58,  57,  55,  54,  52,  50,  49,  47,  45,  43,  41,  40,  38,  36,  35,  33,  32,  30,  29,  28,  27,  25,  24,  24,  22,  21,  21,  21,  21 },
	{  68,  68,  68,  68,  68,  67,  66,  65,  64,  63,  62,  60,  59,  58,  56,  54,  52,  51,  49,  47,  45,  43,  41,  39,  38,  36,  34,  32,  31,  30,  28,  27,  26,  25,  24,  22,  21,  21,  21,  21,  21 },
	{  69,  69,  69,  69,  69,  68,  67,  66,  65,  64,  63,  61,  60,  58,  57,  55,  53,  51,  49,  47,  45,  43,  41,  39,  37,  35,  33,  32,  30,  29,  27,  26,  25,  24,  22,  21,  21,  21,  21,  21,  21 },
	{  70,  70,  70,  70,  70,  70,  69,  67,  66,  65,  64,  62,  61,  59,  57,  56,  54,  51,  49,  47,  45,  43,  41,  39,  36,  34,  33,  31,  29,  28,  26,  25,  24,  22,  21,  20,  20,  20,  20,  20,  20 },
	{  72,  72,  72,  72,   72,  71,  70,  69,  67,  66,  65,  63,  62,  60,  58,  56,  54,  52,  50,  47,  45,  43,  40,  38,  36,  34,  32,  30,  28,  27,  25,  24,  22,  21,  20,  19,  18,  18,  18,  18,  18 },
	{  73,  73,  73,  73,  73,  72,  71,  70,  69,  67,  66,  65,  63,  61,  59,  57,  55,  53,  50,  48,  45,  42,  40,  37,  35,  33,  31,  29,  27,  25,  24,  22,  21,  20,  19,  18,  17,  17,  17,  17,  17 },
	{  74,  74,  74,  74,  74,  73,  72,  71,  70,  69,  67,  66,  64,  62,  60,  58,  56,  53,  51,  48,  45,  42,  39,  37,  34,  32,  30,  28,  26,  24,  22,  21,  20,  19,  18,  17,  16,  16,  16,  16,  16 },
	{  75,  75,  75,  75,  75,  75,  74,  73,  72,  70,  69,  67,  66,  64,  62,  60,  57,  54,  51,  48,  45,  42,  39,  36,  33,  30,  28,  26,  24,  22,  21,  20,  18,  17,  16,  15,  15,  15,  15,  15,  15 },
	{  77,  77,  77,  77,  77,  76,  75,  74,  73,  72,  71,  69,  67,  66,  63,  61,  58,  55,  52,  49,  45,  41,  38,  35,  32,  29,  27,  24,  22,  21,  19,  18,  17,  16,  15,  14,  13,  13,  13,  13,  13 },
	{  78,  78,  78,  78,  78,  77,  77,  76,  75,  74,  73,  71,  69,  67,  65,  63,  60,  57,  53,  49,  45,  41,  37,  33,  30,  27,  25,  22,  21,  19,  17,  16,  15,  14,  13,  13,  12,  12,  12,  12,  12 },
	{  80,  80,  80,  80,  80,  79,  78,  78,  77,  76,  75,  73,  72,  70,  67,  65,  62,  58,  54,  50,  45,  40,  36,  32,  28,  25,  22,  20,  18,  17,  15,  14,  13,  12,  12,  11,  10,  10,  10,  10,  10 },
	{  81,  81,  81,  81,  81,  81,  80,  79,  79,  78,  77,  75,  74,  72,  70,  67,  64,  60,  56,  51,  45,  39,  34,  30,  26,  22,  20,  18,  16,  15,  13,  12,  11,  11,  10,   9,   9,   9,   9,   9,   9 },
	{  83,  83,  83,  83,  83,  83,  82,  81,  81,  80,  79,  78,  77,  75,  73,  71,  67,  63,  58,  52,  45,  38,  32,  27,  22,  19,  17,  15,  13,  12,  11,  10,   9,   9,   8,   7,   7,   7,   7,   7,   7 },
	{  85,  85,  85,  85,  85,  84,  84,  84,  83,  82,  82,  81,  80,  78,  77,  75,  72,  67,  62,  54,  45,  36,  28,  22,  18,  15,  13,  12,  10,   9,   8,   8,   7,   6,   6,   6,   5,   5,   5,   5,   5 },
	{  86,  86,  86,  86,  86,  86,  86,  86,  85,  85,  84,  84,  83,  82,  81,  79,  77,  73,  67,  58,  45,  32,  22,  17,  13,  11,   9,   8,   7,   6,   6,   5,   5,   4,   4,   4,   4,   4,   4,   4,   3 },
	{  87,  88,  88,  88,  88,  88,  88,  88,  88,  87,  87,  87,  86,  86,  85,  84,  83,  81,  77,  67,  45,  22,  13,   9,   7,   6,   5,   4,   4,   3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   1 },
	{  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90, 255,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
	{  91,  92,  92,  92,  92,  92,  92,  92,  92,  93,  93,  93,  94,  94,  95,  96,  97,  99, 103, 113, 135, 158, 167, 171, 173, 174, 175, 176, 176, 177, 177, 177, 178, 178, 178, 178, 178, 178, 178, 178, 179 },
	{  94,  94,  94,  94,  94,  94,  94,  94,  95,  95,  96,  96,  97,  98,  99, 101, 103, 107, 113, 122, 135, 148, 158, 163, 167, 169, 171, 172, 173, 174, 174, 175, 175, 176, 176, 176, 176, 176, 176, 176, 177 },
	{  95,  95,  95,  95,  95,  96,  96,  96,  97,  98,  98,  99, 100, 102, 103, 105, 108, 113, 118, 126, 135, 144, 152, 158, 162, 165, 167, 168, 170, 171, 172, 172, 173, 174, 174, 174, 175, 175, 175, 175, 175 },
	{  97,  97,  97,  97,  97,  97,  98,  99,  99, 100, 101, 102, 103, 105, 107, 109, 113, 117, 122, 128, 135, 142, 148, 153, 158, 161, 163, 165, 167, 168, 169, 170, 171, 171, 172, 173, 173, 173, 173, 173, 173 },
	{  99,  99,  99,  99,  99,  99, 100, 101, 101, 102, 103, 105, 106, 108, 110, 113, 116, 120, 124, 129, 135, 141, 146, 150, 154, 158, 160, 162, 164, 165, 167, 168, 169, 169, 170, 171, 171, 171, 171, 171, 171 },
	{ 100, 100, 100, 100, 100, 101, 102, 102, 103, 104, 105, 107, 108, 110, 113, 115, 118, 122, 126, 130, 135, 140, 144, 148, 152, 155, 158, 160, 162, 163, 165, 166, 167, 168, 168, 169, 170, 170, 170, 170, 170 },
	{ 102, 102, 102, 102, 102, 103, 103, 104, 105, 106, 107, 109, 111, 113, 115, 117, 120, 123, 127, 131, 135, 139, 143, 147, 150, 153, 155, 158, 159, 161, 163, 164, 165, 166, 167, 167, 168, 168, 168, 168, 168 },
	{ 103, 103, 103, 103, 103, 104, 105, 106, 107, 108, 109, 111, 113, 114, 117, 119, 122, 125, 128, 131, 135, 139, 142, 145, 148, 151, 153, 156, 158, 159, 161, 162, 163, 164, 165, 166, 167, 167, 167, 167, 167 },
	{ 105, 105, 105, 105, 105, 105, 106, 107, 108, 110, 111, 113, 114, 116, 118, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147, 150, 152, 154, 156, 158, 159, 160, 162, 163, 164, 165, 165, 165, 165, 165, 165 },
	{ 106, 106, 106, 106, 106, 107, 108, 109, 110, 111, 113, 114, 116, 118, 120, 122, 124, 127, 129, 132, 135, 138, 141, 143, 146, 148, 150, 152, 154, 156, 158, 159, 160, 161, 162, 163, 164, 164, 164, 164, 164 },
	{ 107, 107, 107, 107, 107, 108, 109, 110, 111, 113, 114, 115, 117, 119, 121, 123, 125, 127, 130, 132, 135, 138, 140, 143, 145, 147, 149, 151, 153, 155, 156, 158, 159, 160, 161, 162, 163, 163, 163, 163, 163 },
	{ 108, 108, 108, 108, 108, 109, 110, 111, 113, 114, 115, 117, 118, 120, 122, 124, 126, 128, 130, 133, 135, 137, 140, 142, 144, 146, 148, 150, 152, 153, 155, 156, 158, 159, 160, 161, 162, 162, 162, 162, 162 },
	{ 110, 110, 110, 110, 110, 110, 111, 113, 114, 115, 116, 118, 119, 121, 123, 124, 126, 129, 131, 133, 135, 137, 139, 141, 144, 146, 147, 149, 151, 152, 154, 155, 156, 158, 159, 160, 160, 160, 160, 160, 160 },
	{ 111, 111, 111, 111, 111, 112, 113, 114, 115, 116, 117, 119, 120, 122, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 145, 147, 148, 150, 151, 153, 154, 155, 156, 158, 159, 159, 159, 159, 159, 159 },
	{ 112, 112, 112, 112, 112, 113, 114, 115, 116, 117, 118, 120, 121, 122, 124, 126, 128, 129, 131, 133, 135, 137, 139, 141, 142, 144, 146, 148, 149, 150, 152, 153, 154, 155, 157, 158, 159, 159, 159, 159, 159 },
	{ 112, 112, 112, 112, 113, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 159, 159, 159, 159 },
	{ 112, 112, 112, 113, 113, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 158, 158, 158, 158 },
	{ 112, 112, 113, 114, 114, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 158, 158, 158, 158 },
	{ 112, 113, 114, 114, 114, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 158, 158, 158, 158 },
	{ 113, 114, 114, 114, 114, 114, 114, 115, 117, 118, 119, 120, 122, 123, 125, 126, 128, 130, 130, 132, 135, 136, 138, 140, 142, 144, 145, 147, 148, 150, 151, 152, 153, 155, 156, 157, 158, 158, 158, 158, 158 }

};



#ifdef SUPPORT_GAMMA

/*
 * XXX XXX XXX Important note about "colors" XXX XXX XXX
 *
 * The "TERM_*" color definitions list the "composition" of each
 * "Angband color" in terms of "quarters" of each of the three color
 * components (Red, Green, Blue), for example, TERM_UMBER is defined
 * as 2/4 Red, 1/4 Green, 0/4 Blue.
 *
 * These values are NOT gamma-corrected.  On most machines (with the
 * Macintosh being an important exception), you must "gamma-correct"
 * the given values, that is, "correct for the intrinsic non-linearity
 * of the phosphor", by converting the given intensity levels based
 * on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
 *
 * The actual formula for conversion is unknown to me at this time,
 * but you can use the table below for the most common gamma values.
 *
 * So, on most machines, simply convert the values based on the "gamma"
 * of the target screen, which is usually in the range 1.5 to 1.7, and
 * usually is closest to 1.7.  The converted value for each of the five
 * different "quarter" values is given below:
 *
 *  Given     Gamma 1.0       Gamma 1.5       Gamma 1.7     Hex 1.7
 *  -----       ----            ----            ----          ---
 *   0/4        0.00            0.00            0.00          #00
 *   1/4        0.25            0.27            0.28          #47
 *   2/4        0.50            0.55            0.56          #8f
 *   3/4        0.75            0.82            0.84          #d7
 *   4/4        1.00            1.00            1.00          #ff
 */

/* Table of gamma values */
byte gamma_table[256];

/* Table of ln(x/256) * 256 for x going from 0 -> 255 */
static s16b gamma_helper[256] =
{
0,-1420,-1242,-1138,-1065,-1007,-961,-921,-887,-857,-830,-806,-783,-762,-744,-726,
-710,-694,-679,-666,-652,-640,-628,-617,-606,-596,-586,-576,-567,-577,-549,-541,
-532,-525,-517,-509,-502,-495,-488,-482,-475,-469,-463,-457,-451,-455,-439,-434,
-429,-423,-418,-413,-408,-403,-398,-394,-389,-385,-380,-376,-371,-367,-363,-359,
-355,-351,-347,-343,-339,-336,-332,-328,-325,-321,-318,-314,-311,-308,-304,-301,
-298,-295,-291,-288,-285,-282,-279,-276,-273,-271,-268,-265,-262,-259,-257,-254,
-251,-248,-246,-243,-241,-238,-236,-233,-231,-228,-226,-223,-221,-219,-216,-214,
-212,-209,-207,-205,-203,-200,-198,-196,-194,-192,-190,-188,-186,-184,-182,-180,
-178,-176,-174,-172,-170,-168,-166,-164,-162,-160,-158,-156,-155,-153,-151,-149,
-147,-146,-144,-142,-140,-139,-137,-135,-134,-132,-130,-128,-127,-125,-124,-122,
-120,-119,-117,-116,-114,-112,-111,-109,-108,-106,-105,-103,-102,-100,-99,-97,
-96,-95,-93,-92,-90,-89,-87,-86,-85,-83,-82,-80,-79,-78,-76,-75,
-74,-72,-71,-70,-68,-67,-66,-65,-63,-62,-61,-59,-58,-57,-56,-54,
-53,-52,-51,-50,-48,-47,-46,-45,-44,-42,-41,-40,-39,-38,-37,-35,
-34,-33,-32,-31,-30,-29,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,
-17,-16,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1
};


/*
 * Build the gamma table so that floating point isn't needed.
 *
 * Note gamma goes from 0->256.  The old value of 100 is now 128.
 */
void build_gamma_table(int gamma)
{
	int i, n;

	/*
	 * value is the current sum.
	 * diff is the new term to add to the series.
	 */
	long value, diff;

	/* Hack - convergence is bad in these cases. */
	gamma_table[0] = 0;
	gamma_table[255] = 255;

	for (i = 1; i < 255; i++)
	{
		/*
		 * Initialise the Taylor series
		 *
		 * value and diff have been scaled by 256
		 */

		n = 1;
		value = 256 * 256;
		diff = ((long)gamma_helper[i]) * (gamma - 256);

		while (diff)
		{
			value += diff;
			n++;

			/*
			 * Use the following identiy to calculate the gamma table.
			 * exp(x) = 1 + x + x^2/2 + x^3/(2*3) + x^4/(2*3*4) +...
			 *
			 * n is the current term number.
			 *
			 * The gamma_helper array contains a table of
			 * ln(x/256) * 256
			 * This is used because a^b = exp(b*ln(a))
			 *
			 * In this case:
			 * a is i / 256
			 * b is gamma.
			 *
			 * Note that everything is scaled by 256 for accuracy,
			 * plus another factor of 256 for the final result to
			 * be from 0-255.  Thus gamma_helper[] * gamma must be
			 * divided by 256*256 each itteration, to get back to
			 * the original power series.
			 */
			diff = (((diff / 256) * gamma_helper[i]) *
				 (gamma - 256)) / (256 * n);
		}

		/*
		 * Store the value in the table so that the
		 * floating point pow function isn't needed.
		 */
		gamma_table[i] = ((long)(value / 256) * i) / 256;
	}
}

#endif /* SUPPORT_GAMMA */
