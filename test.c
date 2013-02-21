#include <stdio.h>
#include <stdlib.h>

short *ptr;

int i;

void initp(int a)
{
     static short *p;
     p=malloc(a*sizeof(short));
     for(i=0; i< a; i++)
             p[i]=i;
     ptr=p;
}

void displayp()
{
     for(i=0; i< 10; i++)
              printf("p is %d\n",*(ptr+i));
}

int main()
{
    int a = 10;
    initp(a);
    displayp();
    getche();
    return 0;
}




