#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>

class ConnectionHandler
{
public:
    ConnectionHandler();

    char* convert_Request_to_string(struct ParsedRequest *req);
};

#endif // CONNECTIONHANDLER_H
