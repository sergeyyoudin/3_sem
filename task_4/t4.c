#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define N 10000000
#define my_sem_name "/my_named_posix_semaphore"

double * res;
long long n_points;
int n_threads;
sem_t * sem_res;
sem_t * sem_thread;
int th_cnt = 0;

void * cnt_routine(void * args)
{
	int thr_id = th_cnt++;
	sem_post(sem_thread);

	long long i = 0;
	long long int sum = 0;

	struct drand48_data randBuffer;
	srand48_r(time(NULL), &randBuffer);

	double x, y;
	for (i = 0; i < n_points ; ++i) {
		drand48_r(&randBuffer, &x);
		drand48_r(&randBuffer, &y);
		if(y < x * x * x)		
	    		++sum;
	}

	res[thr_id] = (double)sum / n_points;
	sem_post(sem_res);
	return NULL;
}

int main(int argc, char * argv [])
{

	if(argc < 2){
		return 0;
	}
	n_threads = atoll(argv[1]);
	n_points = N;
	n_points /= n_threads;

	if((sem_res = sem_open(my_sem_name, O_CREAT, 0776, 0)) == SEM_FAILED){
		perror("sem_open()");
		return -1;
	}
	if((sem_thread = sem_open(my_sem_name, O_CREAT, 0776, 0)) == SEM_FAILED){
		perror("sem_open()");
		return -1;
	}

	res = (double *)malloc(n_threads * sizeof(double));
	pthread_t * id = (pthread_t *)malloc(n_threads * sizeof(pthread_t));

	int err, i;
	srand(time(0));
	struct timespec start, stop;

	if(clock_gettime(CLOCK_MONOTONIC, &start) < 0){
		perror("clock gettime()");
		return -1;
	}
	
	for(i = 0; i < n_threads; ++i) {
		err = pthread_create(id + i, NULL, &cnt_routine, NULL);
		if(err) {
		    printf("Can't create thread :[%s]", strerror(err));
		}
		sem_wait(sem_thread);
	}
	
	for(i = 0; i < n_threads; ++i) {
		sem_wait(sem_res);
	}
	
	if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1 ) {
		perror("clock gettime()");
		return -1;
	}
	double accum = (stop.tv_sec - start.tv_sec) + ( stop.tv_nsec - start.tv_nsec) / 1.0e9;
	printf("%lf\n", accum);

	FILE * fdt = fopen("time.txt", "a");
	fprintf(fdt, "%d: %lf\n", n_threads, accum);
	fclose(fdt);

	double integ = 0;
	for(i = 0; i < n_threads; ++i){
		integ += res[i];
	}
	integ /= n_threads;

	printf("res: %lf\n", integ);
		
	sem_close(sem_res);
	sem_close(sem_thread);

	free(id);
	free(res);
	return 0;
}
