#include <windows.h>
#include "Utils.h"

byte Decrypt(byte* pb, int len, byte rn)
{
	byte sum=100;
	for (int i=0; i<len; i++)
	{
		byte b=*pb;
		*pb-=sum+rn;
		sum+=*pb+5;
		pb++;
	}
	return sum;
}

byte Crypt(byte* pb, int len, byte rn)
{
	byte sum=100;
	for (int i=0; i<len; i++)
	{
		byte b=*pb;
		*pb+=sum+rn;
		sum+=b+5;
		pb++;
	}
	return sum;
}

void strcopy(char* s1, char* s2, int bufsize)
{
	int i=0;
	int l=strlen(s2);
	if (l>=bufsize) l=bufsize-1;
	while (i<l)
	{
		s1[i]=s2[i];
		i++;
	}
	s1[i]=0;
}
