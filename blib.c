#include <blib.h>

size_t strlen(const char *s) {
	size_t len=0;
	while(*(s+len)!='\0') {
		len++;
	}
	return len;
}

char *strcpy(char *dst, const char *src) {
	panic("please implement");
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *res = dst;
	while (*src && n--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strcat(char *dst, const char *src) {
	panic("please implement");
}

int strcmp(const char *s1, const char *s2) {
	while(*s1!='\0'&&*s2!='\0') {
		if(*s1!=*s2) {
			return *s1-*s2;
		}
		s1++;
		s2++;
	}	
	if(*s1=='\0'&&*s2=='\0')
		return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n--) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		if (*s1 == 0) {
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	size_t i;
	for(i=0;i<n;i++) {
		*(((int *)s)+i)=c;
	}
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in;
	char *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	size_t i=0;
	for(i=0;*((unsigned int *)s1)!='\0'&&*s2!='\0'&&i<n;i++) {
		if(
	}
}
