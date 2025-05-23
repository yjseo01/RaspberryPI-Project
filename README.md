# RaspberryPI-Project: TCP 기반 원격 장치 제어 프로그램

## 개요

본 프로젝트는 TCP 프로토콜을 이용하여 원격지의 장치를 실시간으로 제어하는 서버/클라이언트 프로그램입니다.  
- **서버:** 라즈베리파이에서 동작하는 데몬 서버  
- **클라이언트:** 우분투 환경에서 동작하는 터미널 클라이언트  

### 지원 장치 및 기능

| 장치       | 제어 / 기능                                      |
|------------|-------------------------------------------------|
| LED        | ON/OFF 제어 및 밝기 조절                         |
| 부저       | 소리 ON/OFF 제어 (음악 재생)                     |
| 조도 센서  | 조도 값 확인 / 빛 감지 여부에 따라 LED 자동 ON/OFF |
| 7 세그먼트 | 숫자(1~9) 표시, 1초마다 -1씩 감소, 0되면 부저 울림 |

- 멀티 스레드 기반 장치 제어
- 장치 제어 프로그램은 공유 라이브러리로 작성하여 동적 로드

---

## 빌드 및 실행 방법
```bash
$ git clone https://github.com/yjseo01/RaspberryPI-Project.git
```
### 서버
#### 1. 빌드 (우분투에서 크로스 컴파일)
```bash
$ cd RaspberryPI-Project
$ make
```

#### 2. 서버 파일 전송 및 설정 (우분투 → 라즈베리파이)
```bash
$ scp daemon_server librasp.so index.html <사용자 이름>@<라즈베리파이 IP 주소>:~/project
```
#### 3. 라즈베리파이 설정
```bash
$ sudo ln -s /home/<사용자 이름>/project/index.html /index.html
$ sudo ln -s /home/<사용자 이름>/project/librasp.so /librasp.so
$ export LD_LIBRARY_PATH=/home/<사용자 이름>/project
```

#### 4. 서버 실행
```bash
$ ./daemon_server
```
#### 5. 서버 로그 확인
```bash
$ journalctl -t daemon_server -f
```

#### 6. 서버 종료
```bash
$ kill -2 <daemon_server_pid>
```

### 클라이언트
#### 1. 클라이언트 실행
```bash
$ ./client
```
---
## API 명세서
| HTTP Method | URI 경로 | 설명 | 파라미터 (쿼리) | 응답 내용 | 비고 |
| --- | --- | --- | --- | --- | --- |
| GET | `/` 또는 `/index.html` | index.html 파일 제공 | 없음 | HTML 페이지 또는 404 오류 |  |
| GET | `/ledon` | LED 켜기 | 없음 | `Command received` | 내부적으로 `cmd_led_on()` 호출 |
| GET | `/ledoff` | LED 끄기 | 없음 | `Command received` | 내부적으로 `cmd_led_off()` 호출 |
| GET | `/ledbrightness?value=NN` | LED 밝기 조절 | `value`: 0~255 | `Command received` | `cmd_led_set_brightness()` 호출 |
| GET | `/buzzon` | 부저 음악 재생 시작 (비동기) | 없음 | `Command received` | 별도 스레드에서 멜로디 재생 |
| GET | `/buzzoff` | 부저 정지 | 없음 | `Command received` | `pthread_cancel()` 사용 |
| GET | `/autoled` | 조도 센서를 기반으로 자동 제어 | 없음 | 조도 값과 함께 상태 메시지 | `cmd_get_brightness()` 사용 |
| GET | `/countdown?sec=NN` | 타이머 시작 (비동기) | `sec`: 초 단위 시간 | `Command received` | `cmd_countdown()` 호출, 비동기 |
| GET | `/play?freq=NNN` | 특정 주파수의 부저음 0.5초 재생 | `freq`: 부저 주파수(Hz) | `Command received` | `softToneWrite()` 사용 |
| 기타 | (알 수 없는 경로) | 존재하지 않는 경로 요청 처리 | - | `404 Not Found` |  |
| 기타 | (GET 외 메서드) | 지원하지 않는 HTTP 메서드 처리 | - | `405 Method Not Allowed` | GET만 지원됨 |

---
## 프로젝트 구조
```plain
.
├── Makefile
├── README.md
├── client
├── client.c
├── client.h
├── ctrl_func.c
├── ctrl_func.h
├── daemon_server
├── daemon_server.h
├── handle_lib.c
├── handle_lib.h
├── index.html
├── librasp
│   ├── librasp.c
│   └── librasp.h
├── librasp.so
├── main.c
├── thd_func.c
└── thd_func.h
```
## 아키텍처 
![image](https://github.com/user-attachments/assets/133e3993-81eb-4820-b41e-4633e4b577f8)

---
