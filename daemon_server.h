#ifndef __DAEMON_SERVER_H__
#define __DAEMON_SERVER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <pthread.h>

//#include <wiringPi.h>

void* led_thd_func(void* arg);
void* handle_thd_func(void* arg);

#endif // __DAEMON_SERVER_H__