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
  Together, these classes define the constant properties of
  the various menu classes.

  A menu consists of:
   - menu_iter, which describes how to handle the type of "list" that's
     being displayed as a menu
   - a menu_skin, which describes the layout of the menu on the screen.
   - various bits and bobs of other data (e.g. the actual list of entries)
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


/**
 * Underlying function set for displaying lists in a certain kind of way.
 */
typedef struct
{
	/* Returns menu item tag (optional) */
	char (*get_tag)(menu_type *menu, int oid);

	/*
	 * Validity checker (optional--all rows are assumed valid if not present)
	 * Return values will be interpreted as: 0 = no, 1 = yes, 2 = hide.
	 */
	int (*valid_row)(menu_type *menu, int oid);

	/* Displays a menu row */
	void (*display_row)(menu_type *menu, int oid, bool cursor,
			    int row, int col, int width);

	/* Handle 'positive' events (selections or cmd_keys) */
	/* XXX split out into a select handler and a cmd_key handler */
	bool (*row_handler)(menu_type *menu, const ui_event *event, int oid);

	/* Called when the screen resizes */
	void (*resize)(menu_type *m);
} menu_iter;



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



/*** Base menu structure ***/

/**
 * Flags for menu appearance & behaviour
 */
enum
{
	/* Tags are associated with the view, not the element */
	MN_REL_TAGS = 0x01,

	/* No tags -- movement key and mouse browsing only */
	MN_NO_TAGS = 0x02,

	/* Tags to be generated by the display function */
	MN_PVT_TAGS = 0x04,

	/* Tag selections can be made regardless of the case of the key pressed.
	 * i.e. 'a' activates the line tagged 'A'. */
	MN_CASELESS_TAGS = 0x08,

	/* double tap (or keypress) for selection; single tap is cursor movement */
	MN_DBL_TAP = 0x10,

	/* no select events to be triggered */
	MN_NO_ACTION = 0x20
} menu_type_flags;


/* Base menu type */
struct menu_type
{
	/** Public variables **/
	const char *header;
	const char *title;
	const char *prompt;

	/* Keyboard shortcuts for menu selection-- shouldn't overlap cmd_keys */
	const char *selections;

	/* String of characters that when pressed, menu handler should be called */
	/* Mustn't overlap with 'selections' or some items may be unselectable */
	const char *cmd_keys;

	/* auxiliary browser help function */
	void (*browse_hook)(int oid, void *db, const region *loc);

	/* Flags specifying the behavior of this menu (from menu_type_flags) */
	int flags;


	/** Private variables **/

	/* Stored boundary, set by menu_layout().  This is used to calculate
	 * where the menu should be displayed on display & resize */
	region boundary;

	int filter_count;        /* number of rows in current view */
	const int *filter_list;  /* optional filter (view) of menu objects */

	int count;               /* number of rows in underlying data set */
	void *menu_data;         /* the data used to access rows. */

	const menu_skin *skin;      /* menu display style functions */
	const menu_iter *row_funcs; /* menu skin functions */

	/* State variables */
	int cursor;             /* Currently selected row */
	int top;                /* Position in list for partial display */
	region active;          /* Subregion actually active for selection */

};

#endif /* INCLUDED_UI_MENU_H */
