/*
 * Copyright (C), 2000-2007 by the monit project group.
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rdup-tr.h"
#include "base64.h"

/* See Section 4 of RFC 3548, be URL safe, modified by Miek Gieben, 2009 */

/* Private prototypes */
static int is_base64(char c);
static char encode(unsigned char u);
static unsigned char decode(char c);

/**
 *  Implementation of base64 encoding/decoding. 
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *  @version \$Id: base64.c,v 1.19 2007/07/25 12:54:31 hauk Exp $
 *  
 */

/**
 * Base64 encode and return size data in 'src'. The caller must free the
 * returned string.
 * @param size The size of the data in src
 * @param src The data to be base64 encode
 * @return encoded string otherwise NULL
 */
char 
*encode_base64(int size, unsigned char *src) 
{
  int i;
  char *out, *p;

  if(!src)
    return NULL;

  if(!size)
    size= strlen((char *)src);
    
  out = g_malloc0(size*4/3+4);
  p = out;
    
  for(i=0; i<size; i+=3) {
      
    unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;
      
    b1 = src[i];
      
    if(i+1<size)
      b2 = src[i+1];
      
    if(i+2<size)
      b3 = src[i+2];
      
    b4= b1>>2;
    b5= ((b1&0x3)<<4)|(b2>>4);
    b6= ((b2&0xf)<<2)|(b3>>6);
    b7= b3&0x3f;
      
    *p++= encode(b4);
    *p++= encode(b5);
      
    if(i+1<size) {
      *p++= encode(b6);
    } else {
      *p++= '=';
    }
      
    if(i+2<size) {
      *p++= encode(b7);
    } else {
      *p++= '=';
    }
  }
  return out;
}


/**
 * Decode the base64 encoded string 'src' into the memory pointed to by
 * 'dest'. The dest buffer is <b>not</b> NUL terminated.
 * @param dest Pointer to memory for holding the decoded string.
 * Must be large enough to recieve the decoded string.
 * @param src A base64 encoded string.
 * @return TRUE (the length of the decoded string) if decode
 * succeeded otherwise FALSE.
 */
int 
decode_base64(unsigned char *dest, const char *src) 
{
  if(src && *src) {
  
    unsigned char *p= dest;
    int k, l= strlen(src)+1;
    unsigned char *buf= g_malloc0(l);
    
    /* Ignore non base64 chars as per the POSIX standard */
    for(k=0, l=0; src[k]; k++) {
      if(is_base64(src[k])) {
	buf[l++]= src[k];
      }
    } 
    
    for(k=0; k<l; k+=4) {
      char c1='A', c2='A', c3='A', c4='A';
      unsigned char b1=0, b2=0, b3=0, b4=0;
      
      c1= buf[k];
      if(k+1<l) {
	c2= buf[k+1];
      }
      
      if(k+2<l) {
	c3= buf[k+2];
      }
      
      if(k+3<l) {
	c4= buf[k+3];
      }
      
      b1= decode(c1);
      b2= decode(c2);
      b3= decode(c3);
      b4= decode(c4);
      
      *p++=((b1<<2)|(b2>>4) );
      
      if(c3 != '=') {
	*p++=(((b2&0xf)<<4)|(b3>>2) );
      }
      if(c4 != '=') {
	*p++=(((b3&0x3)<<6)|b4 );
      }
    }
    
    g_free(buf);
    return(p-dest);
  }
  return FALSE;
}

/**
 * Base64 encode one byte
 */
static char 
encode(unsigned char u) 
{
  if(u < 26)  return 'A'+u;
  if(u < 52)  return 'a'+(u-26);
  if(u < 62)  return '0'+(u-52);
  if(u == 62) return '-';  /* was + */
  
  /* return '/'; */
  return '_';
}

/**
 * Decode a base64 character
 */
static unsigned char 
decode(char c) 
{
  if(c >= 'A' && c <= 'Z') return(c - 'A');
  if(c >= 'a' && c <= 'z') return(c - 'a' + 26);
  if(c >= '0' && c <= '9') return(c - '0' + 52);
  if(c == '-')             return 62;  /* was '+' */
  return 63;
}

/**
 * Return TRUE if 'c' is a valid base64 character, otherwise FALSE
 */
static int 
is_base64(char c) 
{
  if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
     (c >= '0' && c <= '9') || (c == '-')             ||
     (c == '_')             || (c == '=')) {
    return TRUE;
  }
  return FALSE;
}
