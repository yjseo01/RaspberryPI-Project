# 서버 타겟과 교차 컴파일 설정
TARGET = daemon_server
CROSS_CC = aarch64-linux-gnu-gcc
CROSS_CFLAGS = -I/usr/aarch64-linux-gnu/include -I/usr/aarch64-linux-gnu/include/wiringPi -I/usr/aarch64-linux-gnu/include/wiringPiI2C -I/usr/aarch64-linux-gnu/include/softTone -I/usr/aarch64-linux-gnu/include/softPwm -Ilibrasp
CROSS_LDFLAGS = -L/usr/aarch64-linux-gnu/lib -lwiringPi -lcrypt -lpthread -lm -Wl,-rpath-link=/usr/aarch64-linux-gnu/lib -L. -lrasp -Wl,-rpath=.

# 클라이언트 타겟과 일반 컴파일러 설정
CLIENT = client
CC = gcc
CFLAGS = -I.  # 필요 시 클라이언트 전용 헤더 추가

# 소스 파일 정의
SRCS = main.c thd_func.c ctrl_func.c
OBJS = $(SRCS:.c=.o)

CLIENT_SRC = client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

.PHONY: all clean

# 전체 빌드 (서버 + 클라이언트)
all: $(TARGET) $(CLIENT)

# 라이브러리
librasp.so: librasp/librasp.c librasp/librasp.h
	$(CROSS_CC) $(CROSS_CFLAGS) -fPIC -shared -o $@ $< $(CROSS_LDFLAGS)

# 서버 빌드
$(TARGET): $(OBJS) librasp.so
	$(CROSS_CC) $(CROSS_CFLAGS) -o $@ $(OBJS) $(CROSS_LDFLAGS)

%.o: %.c daemon_server.h thd_func.h ctrl_func.h librasp/librasp.h
	$(CROSS_CC) $(CROSS_CFLAGS) -c $< -o $@

# 클라이언트 빌드
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c client.h
	$(CC) $(CFLAGS) -c $< -o $@

# 정리
clean:
	rm -f $(TARGET) $(OBJS) $(CLIENT) $(CLIENT_OBJ) librasp.so
