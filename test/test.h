#include <stdio.h>
#include <sys/wait.h>

#define returns_zero(count, flag, message, routine) \
        count += 1; \
	printf("  =>  %s: ", message); \
	if(! routine()) \
	{\
		puts("ok!"); \
	} \
	else \
        { \
            flag += 1; \
        }

#define fork_to_test(codeblock) \
        int     pid, test_status; \
\
	fflush(stdout); \
	fflush(stderr); \
        pid = fork(); \
        if(pid) \
        { \
                waitpid(pid, &test_status, 0); \
        } \
\
        else \
        { \
                codeblock \
        } \
\
        if(WIFEXITED(test_status))\
        { \
                if(WEXITSTATUS(test_status)) \
                        printf("Child exited with status %d\n", WEXITSTATUS(test_status)); \
                return WEXITSTATUS(test_status); \
        } \
        else if(WIFSTOPPED(test_status)) \
        { \
                printf("Child stopped by signal %d\n", WSTOPSIG(test_status)); \
                return -1; \
        } \
        else \
        { \
                printf("Child terminated by signal %d\n", WTERMSIG(test_status)); \
                return -2; \
        } \

#define fork_and_get_exitcode(status, codeblock) \
        int     pid, test_status; \
\
	fflush(stdout); \
	fflush(stderr); \
        pid = fork(); \
        if(pid) \
        { \
                waitpid(pid, &test_status, 0); \
        } \
\
        else \
        { \
                codeblock \
        } \
\
        if(WIFEXITED(test_status))\
        { \
                if(WEXITSTATUS(test_status)) \
                        printf("Child exited with status %d\n", WEXITSTATUS(test_status)); \
                status = WEXITSTATUS(test_status); \
        } \
        else if(WIFSTOPPED(test_status)) \
        { \
                printf("Child stopped by signal %d\n", WSTOPSIG(test_status)); \
                status = -1; \
        } \
        else \
        { \
                printf("Child terminated by signal %d\n", WTERMSIG(test_status)); \
                status = -2; \
        } \

