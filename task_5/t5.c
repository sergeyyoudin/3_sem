#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int out = 0, k = 128;
pid_t pid;

void one(int signo) {
	out += k;
	k /= 2;
	kill(pid, SIGUSR1);
}

void zero(int signo) {
	k /= 2;
	kill(pid, SIGUSR1);
}

void rec(int signo) {
}

void childexit(int signo) {
	exit(0);
}


int main(int argc, char * argv[]) {
	if (argc < 3) {
		return 0;
	}
	sigset_t set;
	sigemptyset(&set);

	struct sigaction sa_one;
	memset(&sa_one, 0, sizeof(sa_one));
	sa_one.sa_handler = one;
	sigfillset(&sa_one.sa_mask);
	sigaction(SIGUSR1, &sa_one, NULL);

	struct sigaction sa_zero;
	memset(&sa_zero, 0, sizeof(sa_zero));
	sa_zero.sa_handler = zero;
	sigfillset(&sa_zero.sa_mask);
	sigaction(SIGUSR2, &sa_zero, NULL);

	struct sigaction sa_exit;
	memset(&sa_exit, 0, sizeof(sa_exit));
	sa_exit.sa_handler = childexit;
	sigfillset(&sa_exit.sa_mask);
	sigaction(SIGCHLD, &sa_exit, NULL);

	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGUSR2);
	sigaddset(&set, SIGCHLD);

	sigprocmask(SIG_BLOCK, &set, NULL);
	sigemptyset(&set);


	pid = fork();
	if (pid < 0) {
		perror("fork()");
	}
	else if (pid) {
		int fdout = open(argv[2], O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
		while (1) {
			if (k == 0) {
				char x = (char)out;
				write(fdout, &x, 1);
				k = 128;
				out = 0;
			}
			sigsuspend(&set);
		}
	}	
	else {
		struct sigaction empty;
		memset(&empty, 0, sizeof(empty));
		empty.sa_handler = rec;
		sigfillset(&empty.sa_mask);
		sigaction(SIGUSR1, &empty, NULL);

		int fd = open(argv[1], O_CREAT | O_RDONLY);
		if (fd < 0) {
			printf("open() error");
		}
		pid_t ppid = getppid();
		char c, x;
		int i;
		while (read(fd, &c, 1) > 0) {
			for (i = 0; i < 8; ++i) {
				x = (c & 1 << (7 - i)) >> (7 - i);
				if (x) {
					kill(ppid, SIGUSR1);
				}
				else {
					kill(ppid, SIGUSR2);
				}
				sigsuspend(&set);
			}
		}
		close(fd);
		exit(0);
	}
	return 0;
}
