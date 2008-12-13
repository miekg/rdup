#ifndef _IO_H
#define _IO_H

#define O_NONE	    0
#define O_TAR	    1
#define O_CPIO	    2
#define O_PAX	    3
#define O_RDUP	    4

#define I_LIST      1			/* the input is a list of files names */
#define I_RDUP      2			/* the input is the standard rdup output */

#define DO_STAT	    0			/* used by rdup-tr, allows for file stat */
#define NO_STAT	    1			
#define NO_STAT_CONTENT	    2	

#endif /* _IO_H */
