#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>

#define _REENTRANT
#define min(a, b)            ((a) < (b)) ? (a) : (b)

/*                      Program Parameters.                           */

/*         Maximum value of N (dimension size of matrix).             */
#define MAXN                2000
/*               Maximum matrix size for printing.                    */
#define MAXPRINTSIZE          10

/*                          Matrix size.                              */
int N;

/*                      Matrices and vector.                          */
/*         We are given A * X = B; we have to solve for X.            */
volatile float A[MAXN][MAXN], B[MAXN], X[MAXN];

/*                      Number of threads to use.                     */
int NumThreads;

/*             For storing the identity of each thread.               */
pthread_t Threads[_POSIX_THREAD_THREADS_MAX];

pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CountLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t NextIter = PTHREAD_COND_INITIALIZER;

int Norm, CurrentRow, Count;

/*               This function spawns off <NumThreads>.               */
void create_threads(void);

/*       This function runs concurrently in <NumThreads> threads.     */
void *gaussPT(void *);

/*          This function determines the chunk size of rows.          */
int find_next_row_chunk(int *, int );

/*        This function implements a barrier synchronisation.         */
void barrier(int * );

/*        This function awaits the termination of all threads.        */
void wait_for_threads(void);

/*          Returns a seed for <srand()> based on the time.           */
unsigned int time_seed(void);

/*    Set the program parameters from the command-line arguments.     */
void parameters(int , char **);

/*           Initialise A, B and X; X is set to 0.0s.                 */
void initialise_inputs(void);

/*                    Print input matrices.                           */
void print_inputs(void);

/*                    Print solution matrix.                          */
void print_X(void);

void main(int argc, char **argv)
{
/*               Elapsed times using <gettimeofday()>.                */
 struct timeval etstart, etstop;
 struct timezone tzdummy;
 clock_t etstartt, etstoptt;

/*                 Elapsed times using <times()>.                     */
 unsigned long usecstart, usecstop;

/*                  CPU times for the threads.                        */
 struct tms cputstart, cputstop;

 int row, col;


 parameters(argc, argv);
 initialise_inputs();
 print_inputs();

 CurrentRow = Norm+1;
 Count = NumThreads-1;

 printf("Starting clock ...\n");
 gettimeofday(&etstart, &tzdummy);
 etstartt = times(&cputstart);

 create_threads();

 wait_for_threads();

/*
 * Diagonal elements are not normalised to 1.
 * This is treated in back substitution.
 */

/*                         Back substitution.                         */
 for (row = N-1; row >= 0; row--)
 {
  X[row] = B[row];
  for (col = N-1; col > row; col--)
  X[row] -= A[row][col]*X[col];
  X[row] /= A[row][row];
  }

 gettimeofday(&etstop, &tzdummy);
 etstoptt = times(&cputstop);
 printf("Stopped clock.\n");
 
 usecstart = (unsigned long)etstart.tv_sec*1000000+etstart.tv_usec;
 usecstop = (unsigned long)etstop.tv_sec*1000000+etstop.tv_usec;

 print_X();

 printf("Elapsed time = %g ms.\n", 
(float)(usecstop-usecstart)/(float)1000);

 printf("Elapsed time according to <times()> = %g ms.\n", 
(etstoptt-etstartt)/(float)CLK_TCK*1000);

 printf("CPU times are accurate to the nearest %g ms.\n", 
1.0/(float)CLK_TCK*1000.0);

 printf("The total CPU time for parent = %g ms.\n", 
(float)((cputstop.tms_utime+cputstop.tms_stime)-
(cputstart.tms_utime+cputstart.tms_stime))/(float)CLK_TCK*1000);

 printf("The system CPU time for parent = %g ms.\n", 
(float)(cputstop.tms_stime-cputstart.tms_stime)/(float)CLK_TCK*1000);
 }


unsigned int time_seed(void)
{
 struct timeval t;
 struct timezone tzdummy;

 gettimeofday(&t, &tzdummy);
 return (unsigned int)(t.tv_usec);
 }


void parameters(int argc, char **argv)
{
/*        <submit> is set to 1 if submission parameters are used.     */
 int submit = 0;

/*                            Random seed.                            */
 int seed = 0;

/*               <L_cuserid> is a macro defined in stdio.h            */
/*                      <uid> contains the user name.                 */
 char uid[L_cuserid + 2];

/*                             Randomise.                             */
 srand(time_seed());

/*                     Read command-line arguments.                   */
if (argc != 3)
   {
    if (argc == 2 && !strcmp(argv[1], "submit"))
       {
/*                      Use submission parameters.                    */
        submit = 1;
        N = 4;
        NumThreads = 2;
        printf("Submission run for \"%s\".\n", cuserid(uid));
        srand(uid[0]);
        }
    else
       {
        if (argc == 4)
           {
	    seed = atoi(argv[3]);
	    srand(seed);
	    printf("Random seed = %d.\n", seed);
            }
        else
           {
	    printf("Usage: %s <matrix_dimension> <num_threads> \
[random seed]\n", argv[0]);
	    printf("       %s submit\n", argv[0]);
            exit(0);
            }
        }
    }

/*                    Interpret command-line arguments.               */
if (!submit)
   {
    N = atoi(argv[1]);
    if (N < 1 || N > MAXN)
       {
        printf("N = %d is out of range.\n", N);
        exit(0);
        }

    NumThreads = atoi(argv[2]);
    if (NumThreads < 1)
       {
        printf("Warning: Invalid number of threads = %d.  Using 1.\n", 
NumThreads);
        NumThreads = 1;
        }
    if (NumThreads > _POSIX_THREAD_THREADS_MAX)
       {
        printf("Warning: %d threads requested; only %d available.\n", 
NumThreads, _POSIX_THREAD_THREADS_MAX);
        NumThreads = _POSIX_THREAD_THREADS_MAX;
        }
    }

/*                        Print parameters.                           */
printf("Matrix dimension N = %d.\n", N);
printf("Number of threads = %d.\n", NumThreads);
}


void initialise_inputs(void)
{
 int row, col;

 printf("\nInitialising ...\n");
 for (col = 0; col < N; col++)
 {
/*
 * The addition of 1 is to ensure non-zero entries in the
 * coefficient matrix A.
 */
  for (row = 0; row < N; row++)
  A[row][col] = (float)rand()/RAND_MAX+1;
  B[col] = (float)rand()/RAND_MAX+1;
  X[col] = 0.0;
  }
 }


void print_inputs(void)
{
 int row, col;

 if (N < MAXPRINTSIZE)
    {
     printf("\nA =\n\t");
     for (row = 0; row < N; row++)
     {
      for (col = 0; col < N; col++)
      printf("%6.3f  %s", A[row][col], (col < N-1) ? ", " : ";\n\t");
      }
     printf("\nB = [");
     for (col = 0; col < N; col++)
     printf("%6.3f  %s", B[col], (col < N-1) ? "; " : "]\n");
     }
 }


void print_X(void)
{
 int row;

 if (N < MAXPRINTSIZE)
    {
     printf("\nX = [");
     for (row = 0; row < N; row++)
     printf("%6.3f  %s", X[row], (row < N-1) ? "; " : "]\n");
     }
 }


void *gaussPT(void *dummy)
{
/*  <myRow> denotes the first row of the chunk assigned to a thread.  */
 int myRow = 0, row, col;

/*                     Normalisation row.                             */
 int myNorm = 0;

 float multiplier;
 int chunkSize;

/*                Actual Gaussian elimination begins here.            */

 while (myNorm < N-1)
 {
/*        Ascertain the row chunk to be assigned to this thread.      */
  while (chunkSize = find_next_row_chunk(&myRow, myNorm))
  {  

/*      We perform the eliminations across these rows concurrently.   */
   for (row = myRow; row < (min(N, myRow+chunkSize)); row++)
   {
    multiplier = A[row][myNorm]/A[myNorm][myNorm];
    for (col = myNorm; col < N; col++)
    A[row][col] -= A[myNorm][col]*multiplier;
    B[row] -= B[myNorm]*multiplier;
    }
   }

/*           We wait until all threads are done with this stage.      */
/*          We then proceed to handle the next normalisation row.     */
  barrier(&myNorm);
  }
 }


void barrier(int *myNorm)
{
 
/*         We implement synchronisation using condition variables.    */
 pthread_mutex_lock(&CountLock);
  
 if (Count == 0)
    {
/*  Only the last thread for each value of <Norm> reaches this point. */
     Norm++;
     Count = NumThreads-1;
     CurrentRow = Norm+1;
     pthread_cond_broadcast(&NextIter);
     }
 else
    {
     Count--;
     pthread_cond_wait(&NextIter, &CountLock);
     }

/*    <*myNorm> is each thread's view of the global variable <Norm>.  */
 *myNorm = Norm;

 pthread_mutex_unlock(&CountLock);
 }

  
int find_next_row_chunk(int *myRow, int myNorm)
{
 int chunkSize;


 pthread_mutex_lock(&Mutex);

 *myRow = CurrentRow;

/*    For guided-self scheduling, we determine the chunk size here.   */
 chunkSize = (*myRow < N) ? (N-myNorm-1)/(2*NumThreads)+1 : 0;
 CurrentRow += chunkSize;
 
 pthread_mutex_unlock(&Mutex);

 return chunkSize;
 }


void create_threads(void)
{
 int i;


 for (i = 0; i < NumThreads; i++)
 pthread_create(&Threads[i], NULL, gaussPT, NULL);
 }


void wait_for_threads(void)
{
 int i;


 for (i = 0; i < NumThreads; i++)
 pthread_join(Threads[i], NULL);
 }
