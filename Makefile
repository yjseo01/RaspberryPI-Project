TARGET = daemon_server
CROSS_CC = aarch64-linux-gnu-gcc

# 헤더 및 라이브러리 경로
CROSS_CFLAGS = -I/usr/aarch64-linux-gnu/include -I/usr/aarch64-linux-gnu/include/wiringPi  -I/usr/aarch64-linux-gnu/include/wiringPiI2C -I/usr/aarch64-linux-gnu/include/softTone -I/usr/aarch64-linux-gnu/include/softPwm -Ilibrasp
CROSS_LDFLAGS = -L/usr/aarch64-linux-gnu/lib -lwiringPi -lcrypt -lpthread -lm -Wl,-rpath-link=/usr/aarch64-linux-gnu/lib -L. -lrasp -Wl,-rpath=.

SRCS = main.c thd_func.c ctrl_func.c queue.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean all_lib

# 공유 라이브러리 빌드
#all_lib: librasp.so

#librasp.so: librasp/librasp.c librasp/librasp.h
#	$(CROSS_CC) $(CROSS_CFLAGS) -fPIC -shared -o librasp.so librasp/librasp.c $(CROSS_LDFLAGS)

# 전체 빌드
#all: all_lib $(TARGET)

$(TARGET): $(OBJS) librasp.so
	$(CROSS_CC) $(CROSS_CFLAGS) -o $@ $^ $(CROSS_LDFLAGS)

%.o: %.c daemon_server.h thd_func.h ctrl_func.h queue.h librasp/librasp.h
	$(CROSS_CC) $(CROSS_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) librasp.so
