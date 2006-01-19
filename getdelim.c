#ifndef HAVE_GETDELIM

#include "rdup.h"

/* copy glibc source and modified to make it compile */
ssize_t 
getdelim(char **lineptr, size_t *n, int delim, FILE *fp) 
{
  int result;
  ssize_t cur_len = 0;
  ssize_t len;

  if (lineptr == NULL || n == NULL) {
	  return -1;
  }
  return 0;
}

#endif /* !HAVE_GETDELIM */
