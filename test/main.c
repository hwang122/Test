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

void change_1(int a[N][N]){
    int i, j;
    for(i = 0; i < N; i++)
        for(j = 0; j < N; j++)
            a[i][j] = i*N;
}

void recur(int n, char *temp){
    char *a;
    a = (char*)malloc(sizeof(int));
    memset(a, 0, sizeof(int));
    sprintf(a, "%d", n);
    strcat(temp, a);
    strcat(temp, "\n");
    n--;
    if(n > 0)
        recur(n, temp);
}


int main()
{
    int MAX, MIN;

    MAX = 10;
    MIN = 5;
    printf("max value is %d.\n", i_max(MAX, MIN));

    int data1[N][N];
    int i, j;

    change_1(data1);

    printf("change_1...\n");
    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++)
            printf("%3d ", data1[i][j]);
        printf("\n");
    }

    change(data1);
    printf("change...\n");
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

    FILE *f;

    f = fopen("testfile.txt", "w");

    float c[N][N];

    for(i = 0; i < N; i++)
        for(j = 0; j < N; j++)
            c[i][j] = 1.0;

    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++)
            fprintf(f, "   %.7e", c[i][j]);
        fprintf(f, "\n");
    }
    fclose(f);

    char testchar[80];
    strcat(testchar, "test ");
    strcat(testchar, "strcat.");
    puts(testchar);

    printf("\n");

    char temp[100] = "";
    recur(10, temp);
    printf("%s\n", temp);

    void *ptr;
    ptr = (int*)10;

    printf("%d", (int)ptr);

    return 0;
}
