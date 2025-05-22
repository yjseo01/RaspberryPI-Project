#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.0.68"
#define SERVER_PORT 8080
#define MAX_BUF 1024

void send_http_get(const char*);