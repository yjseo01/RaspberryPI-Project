TARGET = daemon_server
CC = gcc
CROSS_CC = aarch64-linux-gnu-gcc

# 크로스 컴파일 시 헤더, 라이브러리 경로
CROSS_CFLAGS = -I/usr/aarch64-linux-gnu/include -I/usr/aarch64-linux-gnu/include/wiringPi -Ilibrasp
CROSS_LDFLAGS = -L/usr/aarch64-linux-gnu/lib -lwiringPi -lcrypt -lpthread -lm -Wl,-rpath-link=/usr/aarch64-linux-gnu/lib

CFLAGS = -Wall -O2 
LDFLAGS = -lpthread 

SRCS = main.c thd_func.c queue.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean all_lib

# 공유 라이브러리 빌드
#all_lib: librasp.so

#librasp.so: librasp/librasp.c librasp/librasp.h
#	$(CROSS_CC) $(CROSS_CFLAGS) -fPIC -shared -o librasp.so librasp/librasp.c $(CROSS_LDFLAGS)

# 메인 타겟 빌드
all: all_lib $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 오브젝트 파일 컴파일
%.o: %.c daemon_server.h thd_func.h queue.h 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) librasp.so
