#ifndef _GENERIC_H
#define _GENERIC_H

#define O_NONE	    0
#define O_TAR	    1
#define O_CPIO	    2
#define O_PAX	    3
#define O_RDUP	    4

#define I_NONE	    0			/* used by rdup-tr, allows for file stat */
#define I_LIST      1			/* the input is a list of files names */
#define I_RDUP      2			/* the input is the standard rdup output */
#define I_RDUP_C    3			/* the input is rdup -c output */

#endif /* _GENERIC_H */
