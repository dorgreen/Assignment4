#include "types.h"
#include "x86.h"

void*
memset(void *dst, int c, uint n)
{
  if ((int)dst%4 == 0 && n%4 == 0){
    c &= 0xFF;
    stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
  } else
    stosb(dst, c, n);
  return dst;
}

int
memcmp(const void *v1, const void *v2, uint n)
{
  const uchar *s1, *s2;

  s1 = v1;
  s2 = v2;
  while(n-- > 0){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++, s2++;
  }

  return 0;
}

void*
memmove(void *dst, const void *src, uint n)
{
  const char *s;
  char *d;

  s = src;
  d = dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void*
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

int
strncmp(const char *p, const char *q, uint n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

char*
strncpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char*
safestrcpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  if(n <= 0)
    return os;
  while(--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
}

int
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

// TODO: TEST ME!
// Helper functions for string handling

int itoa(char* string, int num){
    // TODO: REFACTOR
    // TODO: CHECK RETURN VALUE FOR CORRECTNESS
    int i = 0;
    int len = 0;
    if (num == 0){
        string[0] = '0';
        return 1;
    }

    while(num != 0){
        string[len] = num % 10 + '0';
        num = num / 10;
        len++;
    }
    for(i = 0; i < len/2; i++){
        char tmp = string[i];
        string[i] = string[len - 1 - i];
        string[len - 1 - i] = tmp;
    }
    return len;
}

int buff_append(char *buff, char *data){
    int current_length = strlen(buff);
    int length_to_add = strlen(data);

    memmove(buff + current_length, data, length_to_add);
    return length_to_add;
}
int buff_append_num(char *buff, int data){
    char int_buff[4] = {0};
    itoa(int_buff, data);
    int chars_written = buff_append(buff, int_buff);
    return chars_written;
}

