#ifndef STRING_H
#define STRING_H

#ifdef  __cplusplus
extern "C" { 
#endif

#define bcopy(Src,Dest,Len) memcpy(Dest,Src,Len)
#define bzero(Dest,Len) memset(Dest,0,Len)

extern void *memchr(void *Src, int Char,size_t n);
extern void *memcpy(void *s1, const void *s2, size_t n);
extern int memcmp(char *Str1, char *Str2,size_t n);
extern void *memmove(void *s1, const void *s2, size_t n);
extern void *memset(void *Dest, int FillChar, size_t Len);
extern char *strcpy(char *s1, const char *s2);
extern char *strncpy(char *s1, const char *s2, size_t n);
extern char *strcat(char *s1, const char *s2);
extern char *strncat(char *s1, const char *s2, size_t n);
extern int strcmp(char *s1, const char *s2);
extern int stricmp(char *s1, const char *s2);
extern int strcasecmp(char *s1, const char *s2);
extern int strncmp(char *s1, const char *s2, size_t n);
extern size_t strlen(const char *s1);

extern char *strchr(const char *s, int c);
extern ulong strtoul(const char *nsptr, char **endptr, int base);
extern long strtol(const char *nsptr, char **endptr, int base);

#define CopyMem(a,b,c) memcpy(a,b,c)
#define SetMem(a,b,c) memset(a,b,c)

#ifdef  __cplusplus 
}
#endif

#endif
