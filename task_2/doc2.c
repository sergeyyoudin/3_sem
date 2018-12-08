#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
	int txd[2];
	int rxd[2];
} dpipe_t;


int main() {
	char buf[4096];
	int size;
	dpipe_t dp;

	if(pipe(dp.txd) < 0) {
		printf("pipe() failed\n");
		return 1;
	}
	if(pipe(dp.rxd) < 0) {
		printf("pipe() failed\n");
		return 2;
	}
	
	pid_t pid = fork();
	if (pid == -1) {
		printf("fork() failed\n");
		return 3;
	}
	int status;
	if (pid) {
		close(dp.txd[0]);
		close(dp.rxd[1]);
		while((size = read(0, buf, sizeof(buf) - 1)) > 0) {
			buf[size] = 0;
			write(dp.txd[1], buf, size);
			printf("Parent sent:\t\t%s", buf);
			size = read(dp.rxd[0], buf, sizeof(buf) - 1);
			buf[size] = 0;
			printf("Parent received:\t%s", buf);
		}                        
		write(dp.txd[1], "\0", 1);
		waitpid(pid, &status, 0);
		printf("Return code: %d\n", WEXITSTATUS(status));
		close(dp.txd[1]);
		close(dp.rxd[0]);
	}
	else{
                close(dp.txd[1]);
                close(dp.rxd[0]);
                while((size = read(dp.txd[0], buf, sizeof(buf) - 1)) > 0) {
                        buf[size] = 0;
			if (!buf[0])
				break;
                        printf("Child received:\t\t%s", buf);
                        size = write(dp.rxd[1], buf, sizeof(buf) - 1);
                        buf[size] = 0;
                        printf("Child sent:\t\t%s", buf);
                }
		close(dp.txd[0]);
		close(dp.rxd[1]);
		exit(0);
	}

	return 0;
}
