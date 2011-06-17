/* File: h-system.h */

#ifndef INCLUDED_H_SYSTEM_H
#define INCLUDED_H_SYSTEM_H

/*
 * Include the basic "system" files.
 *
 * Make sure all "system" constants/macros are defined.
 * Make sure all "system" functions have "extern" declarations.
 *
 * This file is a big hack to make other files less of a hack.
 * This file has been rebuilt -- it may need a little more work.
 */

/** ANSI C headers **/
#include <ctype.h>
#include <errno.h>
/* limits.h */
/* assert.h */

#include <stdarg.h>
#include <stdio.h>
#if defined(NeXT)
# include <libc.h>
#else
# include <stdlib.h>
#endif /* NeXT */
/* stddef.h */
#include <string.h>
#include <time.h>


/** Other headers **/





#ifdef SET_UID

# include <sys/types.h>

# if defined(Pyramid) || defined(NeXT) || defined(SUNOS) || \
     defined(NCR3K) || defined(SUNOS) || defined(ibm032) || \
     defined(__osf__) || defined(ISC) || defined(SGI) || \
     defined(linux)
#  include <sys/time.h>
# endif

# if !defined(SGI) && !defined(ULTRIX)
#  include <sys/timeb.h>
# endif

#endif /* SET_UID */





#if defined(MACINTOSH) && defined(__MWERKS__)
# include <unix.h>
#endif

#if defined(WINDOWS) || defined(MSDOS)
# include <io.h>
#endif

#if !defined(MACINTOSH) && \
    !defined(RISCOS) && !defined(__MWERKS__)
# if defined(__TURBOC__) || defined(__WATCOMC__)
#  include <mem.h>
# else
#  include <memory.h>
# endif
#endif


#if !defined(NeXT) && !defined(RISCOS)
# include <fcntl.h>
#endif


#ifdef SET_UID

# ifndef USG
#  include <sys/param.h>
#  include <sys/file.h>
# endif

# ifdef linux
#  include <sys/file.h>
# endif

# include <pwd.h>

# include <unistd.h>

# include <sys/stat.h>

# if defined(SOLARIS)
#  include <netdb.h>
# endif

#endif /* SET_UID */

#if defined(__DJGPP__) || defined(__MWERKS__)
#include <unistd.h>
#endif /* __DJGPP__ || __MWERKS__ */




#endif /* INCLUDE_H_SYSTEM_H */


