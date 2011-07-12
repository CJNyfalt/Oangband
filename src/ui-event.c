/*
 * File: ui-event.c
 * Purpose: Utility functions relating to UI events
 *
 * Copyright (c) 2011 Andi Sidwell
 */
#include "angband.h"
#include "ui-event.h"

/**
 * Map keycodes to their textual equivalent.
 */
static const struct {
	keycode_t code;
	const char *desc;
} mappings[] = {
	{ ESCAPE, "Escape" },
	{ KC_RETURN, "Return" },
	{ KC_ENTER, "Enter" },
	{ KC_TAB, "Tab" },
	{ KC_DELETE, "Delete" },
	{ KC_BACKSPACE, "Backspace" },
	{ ARROW_DOWN, "Down" },
	{ ARROW_LEFT, "Left" },
	{ ARROW_RIGHT, "Right" },
	{ ARROW_UP, "Up" },
	{ KC_F1, "F1" },
	{ KC_F2, "F2" },
	{ KC_F3, "F3" },
	{ KC_F4, "F4" },
	{ KC_F5, "F5" },
	{ KC_F6, "F6" },
	{ KC_F7, "F7" },
	{ KC_F8, "F8" },
	{ KC_F9, "F9" },
	{ KC_F10, "F10" },
	{ KC_F11, "F11" },
	{ KC_F12, "F12" },
	{ KC_F13, "F13" },
	{ KC_F14, "F14" },
	{ KC_F15, "F15" },
	{ KC_HELP, "Help" },
	{ KC_HOME, "Home" },
	{ KC_PGUP, "PageUp" },
	{ KC_END, "End" },
	{ KC_PGDOWN, "PageDown" },
	{ KC_INSERT, "Insert" },
	{ KC_PAUSE, "Pause" },
	{ KC_BREAK, "Break" },
	{ KC_BEGIN, "Begin" },
};


/**
 * Given a string, try and find it in "mappings".
 */
keycode_t keycode_find_code(const char *str, size_t len)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(mappings); i++) {
		if (strncmp(str, mappings[i].desc, len) == 0)
			return mappings[i].code;
	}
	return 0;
}


/**
 * Given a keycode, return its textual mapping.
 */
const char *keycode_find_desc(keycode_t kc)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(mappings); i++) {
		if (mappings[i].code == kc)
			return mappings[i].desc;
	}
	return NULL;
}

/*
 * Convert a decimal to a single digit hex number
 */
static char hexify(unsigned int i)
{
	return (hexsym[i%16]);
}




/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c)
{
	if (isdigit((unsigned char)c)) return (D2I(c));
	if (isalpha((unsigned char)c)) return (A2I(tolower((unsigned char)c)) + 10);
	return (0);
}


/*
 * Hack -- convert a printable string into real ascii
 *
 * This function will not work on non-ascii systems.
 *
 * To be safe, "buf" should be at least as large as "str".
 */
void text_to_ascii(char *buf, const char *str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		/* Backslash codes */
		if (*str == '\\')
		{
			/* Skip the backslash */
			str++;

			/* Hack -- simple way to specify Escape */
			if (*str == 'e')
			{
				*s++ = ESCAPE;
			}

			/* Hack -- simple way to specify "space" */
			else if (*str == 's')
			{
				*s++ = ' ';
			}

			/* Backspace */
			else if (*str == 'b')
			{
				*s++ = '\b';
			}

			/* Newline */
			else if (*str == 'n')
			{
				*s++ = '\n';
			}

			/* Return */
			else if (*str == 'r')
			{
				*s++ = '\r';
			}

			/* Tab */
			else if (*str == 't')
			{
				*s++ = '\t';
			}

			/* Actual "backslash" */
			else if (*str == '\\')
			{
				*s++ = '\\';
			}

			/* Hack -- Actual "caret" */
			else if (*str == '^')
			{
				*s++ = '^';
			}

			/* Hack -- Hex-mode */
			else if (*str == 'x')
			{
				*s = 16 * dehex(*++str);
				*s++ += dehex(*++str);
			}

			/* Oops */
			else
			{
				*s = *str;
			}

			/* Skip the final char */
			str++;
		}

		/* Normal Control codes */
		else if (*str == '^')
		{
			str++;
			*s++ = (*str++ & 037);
		}

		/* Normal chars */
		else
		{
			*s++ = *str++;
		}
	}

	/* Terminate */
	*s = '\0';
}


/*
 * Hack -- convert a string into a printable form
 *
 * This function will not work on non-ascii systems.
 *
 * To be safe, "buf" should be at least four times as large as "str".
 */
void ascii_to_text(char *buf, const char *str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		byte i = (byte)(*str++);

		if (i == ESCAPE)
		{
			*s++ = '\\';
			*s++ = 'e';
		}
		else if (i == ' ')
		{
			*s++ = '\\';
			*s++ = 's';
		}
		else if (i == '\b')
		{
			*s++ = '\\';
			*s++ = 'b';
		}
		else if (i == '\t')
		{
			*s++ = '\\';
			*s++ = 't';
		}
		else if (i == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (i == '\r')
		{
			*s++ = '\\';
			*s++ = 'r';
		}
		else if (i == '\\')
		{
			*s++ = '\\';
			*s++ = '\\';
		}
		else if (i == '^')
		{
			*s++ = '\\';
			*s++ = '^';
		}
		else if (i < 32)
		{
			*s++ = '^';
			*s++ = i + 64;
		}
		else if (i < 127)
		{
			*s++ = i;
		}
		else
		{
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify(i / 16);
			*s++ = hexify(i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}



