#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define SERVER_IP "192.168.0.68"
#define SERVER_PORT 8080
#define MAX_BUF 1024

void ignore_all_signals_except_int();
void send_http_get(const char*);
int note_to_freq(const char*); 
void clear_stdin_buffer();