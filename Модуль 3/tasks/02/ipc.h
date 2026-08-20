#ifndef IPC_H
#define IPC_H

#include "protocol.h"

int create_broker_queue(void);
int connect_to_broker_queue(void);
int send_queue_text(int queue_id, long type, const char *text);
int remove_queue(int queue_id);

#endif
