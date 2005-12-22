/* on mac OS (and prob. others) we don't have regex_t->buffer,
 * so wrap the whole thing in a struct */

#ifndef _WALKER_H_
#define _WALKER_H_

struct struct_regext
{
		int      max;   /* how many do we have */
	        regex_t expr[MAXDIR];   /* the regular expression */
};
typedef struct struct_regext reg_exp;

#endif /* _WALKER_H_ */
