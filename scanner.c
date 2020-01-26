#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "scanner.h"
#include "constants.h"
#include "rand.h"
static uint32_t get_random_ip(void);

void scanner_init(void) {
    int rsck;
    pid_t scanner_pid;

#ifdef DEBUG
    fflush(stderr);
#endif

    scanner_pid = fork();
    if (scanner_pid > 0 || scanner_pid == -1) {
        return; /* return to main */
    }

    rand_init();


}

static uint32_t get_random_ip(void) {
    uint32_t tmp;
    uint8_t o1, o2, o3, o4;

#ifdef DEBUG
    struct in_addr in;
#endif

    do {
        tmp = rand();

        o1 = tmp & 0xff;
        o2 = (tmp >> 8) & 0xff;
        o3 = (tmp >> 16) & 0xff;
        o4 = (tmp >> 24) & 0xff;
    } while (
        o1 == 127 ||  // 127.0.0.0/8      - Loopback
        (o1 == 0) ||  // 0.0.0.0/8        - Invalid address space
        (o1 == 3) ||  // 3.0.0.0/8        - General Electric Company
        (o1 == 15 || o1 == 16) ||  // 15.0.0.0/7       - Hewlett-Packard Company
        (o1 == 17 || o1 == 19 || o1 == 48 || o1 == 53) || // Apple, Ford, Prudential, Daimler, 
        (o1 == 56) ||              // 56.0.0.0/8       - US Postal Service
        (o1 == 10) ||              // 10.0.0.0/8       - Internal network
        (o1 == 192 && o2 == 168) ||  // 192.168.0.0/16   - Internal network
        (o1 == 172 && o2 >= 16 &&
         o2 < 32) ||  // 172.16.0.0/14    - Internal network
        (o1 == 100 && o2 >= 64 &&
         o2 < 127) ||               // 100.64.0.0/10    - IANA NAT reserved
        (o1 == 169 && o2 > 254) ||  // 169.254.0.0/16   - IANA NAT reserved
        (o1 == 198 && o2 >= 18 &&
         o2 < 20) ||    // 198.18.0.0/15    - IANA Special use
        (o1 >= 224) ||  // 224.*.*.*+       - Multicast
        (o1 == 6 || o1 == 7 || o1 == 11 || o1 == 21 || o1 == 22 || o1 == 26 ||
         o1 == 28 || o1 == 29 || o1 == 30 || o1 == 33 || o1 == 55 ||
         o1 == 214 || o1 == 215)  // Department of Defense
    );

#ifdef DEBUG
    in.s_addr = INET_ADDR(o1, o2, o3, o4);
    fprintf(stderr, "scanner: randomly generated ip: %s", inet_ntoa(in));
#endif

    return INET_ADDR(o1, o2, o3, o4);
}

static uint32_t util_local_addr(void) {
    int fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
#ifdef DEBUG
        fprintf(stderr, "[-] util_local_addr: failed to call socket\n");
#endif
        return 0;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INET_ADDR(8, 8, 8, 8);
    addr.sin_port = htons(53);

    connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    getsockname(fd, (struct sockaddr *)&addr, &addr_len);
    close(fd);
    return addr.sin_addr.s_addr;
}
