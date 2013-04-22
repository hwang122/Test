#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10

extern int i_max(int ,int);

void change(int (*a)[N]){
    int i, j;
    for(i = 0; i < N; i++)
        for(j = 0; j < N; j++)
            a[i][j] = i*N + j;
}

int main()
{
    printf("Hello world!\n");
    int MAX, MIN;

    MAX = 10;
    MIN = 5;
    printf("max value is %d.\n", i_max(MAX, MIN));

    int data1[N][N];
    int i, j;

    change(data1);

    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++)
            printf("%3d ", data1[i][j]);
        printf("\n");
    }

    int a[N][N], b[5][N];

    for(i = 0; i < N; i++)
        for(j = 0; j < N; j++)
            a[i][j] = i * N + j;

    memmove(b, a, sizeof(int)*50);

    printf("b = \n");
    for(i = 0; i < 5; i++){
        for(j = 0; j < N; j++)
            printf("%3d ", b[i][j]);
        printf("\n");

    }
    return 0;
}
