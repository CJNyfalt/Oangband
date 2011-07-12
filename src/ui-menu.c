/*
 * File: ui-menu.c
 * Purpose: Generic menu interaction functions
 *
 * Copyright (c) 2007 Pete Mack
 * Copyright (c) 2010 Andi Sidwell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "ui-event.h"
#include "ui-menu.h"

/* Cursor colours */
const byte curs_attrs[2][2] =
{
	{ TERM_SLATE, TERM_BLUE },      /* Greyed row */
	{ TERM_WHITE, TERM_L_BLUE }     /* Valid row */
};

/* Some useful constants */
const char lower_case[] = "abcdefghijklmnopqrstuvwxyz";
const char upper_case[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char all_letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* Display an event, with possible preference overrides */
static void display_action_aux(menu_action *act, byte color, int row, int col, int wid)
{
	/* TODO: add preference support */
	/* TODO: wizard mode should show more data */
	Term_erase(col, row, wid);

	if (act->name)
		Term_putstr(col, row, wid, color, act->name);
}

/* ------------------------------------------------------------------------
 * MN_ACTIONS HELPER FUNCTIONS
 *
 * MN_ACTIONS is the type of menu iterator that displays a simple list of
 * menu_actions.
 * ------------------------------------------------------------------------ */

static char menu_action_tag(menu_type *m, int oid)
{
	menu_action *acts = menu_priv(m);
	return acts[oid].tag;
}

static int menu_action_valid(menu_type *m, int oid)
{
	menu_action *acts = menu_priv(m);

	if (acts[oid].flags & MN_ACT_HIDDEN)
		return 2;

	return acts[oid].name ? TRUE : FALSE;
}

static void menu_action_display(menu_type *m, int oid, bool cursor, int row, int col, int width)
{
	menu_action *acts = menu_priv(m);
	byte color = curs_attrs[!(acts[oid].flags & (MN_ACT_GRAYED))][0 != cursor];

	display_action_aux(&acts[oid], color, row, col, width);
}

static bool menu_action_handle(menu_type *m, const ui_event *event, int oid)
{
	menu_action *acts = menu_priv(m);

	if (event->type == EVT_SELECT)
	{
		if (!(acts->flags & MN_ACT_GRAYED) && acts[oid].action)
		{
			acts[oid].action(acts[oid].name, m->cursor);
			return TRUE;
		}
	}

	return FALSE;
}


/* Virtual function table for action_events */
static const menu_iter menu_iter_actions =
{
	menu_action_tag,
	menu_action_valid,
	menu_action_display,
	menu_action_handle,
	NULL
};


/* ------------------------------------------------------------------------
 * MN_STRINGS HELPER FUNCTIONS
 *
 * MN_STRINGS is the type of menu iterator that displays a simple list of
 * strings - no action is associated, as selection will just return the index.
 * ------------------------------------------------------------------------ */
static void display_string(menu_type *m, int oid, bool cursor,
			   int row, int col, int width)
{
	const char **items = menu_priv(m);
	byte color = curs_attrs[CURS_KNOWN][0 != cursor];
	Term_putstr(col, row, width, color, items[oid]);
}

/* Virtual function table for displaying arrays of strings */
const menu_iter menu_iter_strings =
{
	NULL,              /* get_tag() */
	NULL,              /* valid_row() */
	display_string,    /* display_row() */
	NULL,              /* row_handler() */
	NULL
};




/* ================== SKINS ============== */


/* Scrolling menu */
/* Find the position of a cursor given a screen address */
static int scrolling_get_cursor(int row, int col, int n, int top, region *loc)
{
	int cursor = row - loc->row + top;
	if (cursor >= n) cursor = n - 1;

	return cursor;
}
