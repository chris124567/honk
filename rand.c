#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include <sys/types.h>

#include "rand.h"

void rand_init(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
#ifdef DEBUG
        fprintf(stderr,
                "[-] Failed to get current time in UTC for seeding RNG, "
                "falling back to getpid() and clock() based seed %u\n",
                (unsigned int)(clock() * getppid()));
#endif
        srand((unsigned int)((unsigned int)(clock() * getppid())));
    } else {
#ifdef DEBUG
        fprintf(stderr, "[+] Seeding RNG with time based value %lu\n",
                ts.tv_nsec ^ ts.tv_sec);
#endif
        srand((unsigned int)(ts.tv_nsec ^ ts.tv_sec));
    }
}

long rand_range(long min, long max) { return rand() % (max + 1 - min) + min; }
