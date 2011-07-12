#ifndef INCLUDED_UI_MENU_H
#define INCLUDED_UI_MENU_H

/*** Constants ***/

/* Colors for interactive menus */
enum
{
	CURS_UNKNOWN = 0,		/* Use gray / dark blue for cursor */
	CURS_KNOWN = 1			/* Use white / light blue for cursor */
};

/* Cursor colours for different states */
extern const byte curs_attrs[2][2];

/* Standard menu orderings */
extern const char lower_case[];			/* abc..z */
extern const char upper_case[];			/* ABC..Z */
extern const char all_letters[];		/* abc..zABC..Z */


/*
 * Together, these classes define the constant properties of
 * the various menu classes.

 */
typedef struct menu_type menu_type;



/*** Predefined menu "skins" ***/

/**
 * Types of predefined skins available.
 */
typedef enum
{
	/*
	 * A simple list of actions with an associated name and id.
	 * Private data: an array of menu_action
	 */
	MN_ITER_ACTIONS = 1,

	/*
	 * A list of strings to be selected from - no associated actions.
	 * Private data: an array of const char *
	 */
	MN_ITER_STRINGS = 2
} menu_iter_id;


/**
 * Primitive menu item with bound action.
 */
typedef struct
{
	int flags;
	char tag;
	const char *name;
	void (*action)(const char *title, int row);
} menu_action;


/**
 * Flags for menu_actions.
 */
#define MN_ACT_GRAYED     0x0001 /* Allows selection but no action */
#define MN_ACT_HIDDEN     0x0002 /* Row is hidden, but may be selected via tag */

/*** Menu skins ***/

/**
 * Identifiers for the kind of layout to use
 */
typedef enum
{
	MN_SKIN_SCROLL = 1,   /**< Ordinary scrollable single-column list */
	MN_SKIN_COLUMNS = 2   /**< Multicolumn view */
} skin_id;


/* Class functions for menu layout */
typedef struct
{
	/* Determines the cursor index given a (mouse) location */
	int (*get_cursor)(int row, int col, int n, int top, region *loc);

	/* Displays the current list of visible menu items */
	void (*display_list)(menu_type *menu, int cursor, int *top, region *);

	/* Specifies the relative menu item given the state of the menu */
	char (*get_tag)(menu_type *menu, int pos);

	/* Process a direction */
	ui_event (*process_dir)(menu_type *menu, int dir);
} menu_skin;

#endif /* INCLUDED_UI_MENU_H */
