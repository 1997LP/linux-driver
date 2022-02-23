
#ifndef __COMMON_H_
#define __COMMON_H_	

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char	vu_char;

#include "config.h"
#include "types.h"
#include <stdarg.h>
#include <stddef.h>

#define CONFIG_SYS_CBSIZE 128

size_t strnlen(const char * s, size_t count);
size_t strlen(const char * s);
unsigned long simple_strtoul(const char *cp, char **endp,unsigned int base);
int strict_strtoul(const char *cp, unsigned int base, unsigned long *res);
long simple_strtol(const char *cp, char **endp, unsigned int base);
unsigned long ustrtoul(const char *cp, char **endp, unsigned int base);
unsigned long long ustrtoull(const char *cp, char **endp, unsigned int base);
unsigned long long simple_strtoull(const char *cp, char **endp,unsigned int base);

void * memcpy(void *dest, const void *src, size_t count);
void * memset(void * s,int c,size_t count);
int memcmp(const void * cs,const void * ct,size_t count);
int strcmp(const char * cs,const char * ct);
char * strncpy(char * dest,const char *src,size_t count);
char * strcpy(char * dest,const char *src);
void hex2str(unsigned char *pHex, unsigned char *pAscii, int nLen);
void print_hex_64(u64 val);


void wr_addr32(unsigned long addr, unsigned int data);
unsigned int rd_addr32(unsigned long addr);
void wr_addr8(unsigned long addr, unsigned char data);
unsigned char rd_addr8(unsigned long addr);
void wr_addr16(unsigned    long addr ,short data);
short rd_addr16(unsigned long addr);
void wr_addr64(unsigned long addr, long long data);
long long rd_addr64(unsigned long addr);
void board_init_f(unsigned long bootflag);

#define   word64(x)     *((volatile unsigned long  *)(x))
#define   word32(x)     *((volatile unsigned int   *)(x))
#define   short16(x)    *((volatile unsigned short *)(x))
#define   char8(x)      *((volatile unsigned char  *)(x))

#endif /* __ASSEMBLY__ */

#endif	/* __COMMON_H_ */
