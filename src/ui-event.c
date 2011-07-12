/*
 * File: ui-event.c
 * Purpose: Utility functions relating to UI events
 *
 * Copyright (c) 2011 Andi Sidwell
 */
#include "angband.h"
#include "ui-event.h"


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
	if (isdigit(c)) return (D2I(c));
	if (isalpha(c)) return (A2I(tolower(c)) + 10);
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



