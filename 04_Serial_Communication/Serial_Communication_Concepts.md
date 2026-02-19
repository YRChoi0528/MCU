# Serial Communication Concepts

## 요약

해당 문서는 STM32 기반 임베디드 시스템에서 사용되는 직렬 통신 방식 중 수업 시간에 배우게 되는 USART, I2C, SPI에 대한 상위 개요 문서이다.
각 통신 방식의 동작 개념, 구조적 특징, 사용 목적을 정리하고 이후 하위 문서(USART_Registers.md, I2C_Registers.md, SPI_Registers.md)에서 레지스터 단위의 상세 내용을 다룬다.

---

## 1. 직렬 통신 개요

임베디드 시스템에서 통신은 CPU 외부와 데이터를 교환하기 위한 핵심 수단이다.
STM32 MCU는 다양한 직렬 통신 주변장치를 제공하며, 각 통신은 **동기/비동기 방식**, **클럭 존재 여부**, **배선 수**, **속도**, **연결 구조** 측면에서 차이를 가진다.

본 문서에서는 다음 세 가지 통신을 다룬다.

* USART (Universal Synchronous/Asynchronous Receiver/Transmitter)
* I2C (Inter-Integrated Circuit)
* SPI (Serial Peripheral Interface)

---

## 2. USART 개념

### 2.1 기본 특징

- 동기(Synchronous)/비동기(Asynchronous) 통신 방식
- 비동기로 많이 쓰이지만, 필요시 클럭(SCLK)을 핀으로 출력하여 동기식으로도 사용 가능
- TX와 RX 라인이 분리되어 있어 동시에 송수신이 가능한 **Full-Duplex(전이중)** 통신
- TX, RX (필수), SCLK(필요시), GND로 구성

### 2.2 동작 개념

- 송신 측과 수신 측이 사전에 약속된 보드레이트(Baud Rate)를 기준으로 통신
- 데이터 프레임은 Start Bit, Data Bit, (Optional) Parity Bit, Stop Bit로 구성

### 2.3 사용 목적

- PC와 MCU 간 디버깅용 통신
- 로그 출력, 콘솔 인터페이스
- 비교적 저속이지만 구현이 단순

---

## 3. I2C 개념

### 3.1 기본 특징

- 동기(Synchronous) 통신 방식
- 하나의 데이터 라인(SDA)으로 송신과 수신을 번갈아 하는 **Half-Duplex(반이중)** 통신
- 2선(SDA, SCL) 기반 버스 구조
- 풀업 저항을 사용하는 Open-Drain 방식

### 3.2 동작 개념

- 하나 이상의 Master와 다수의 Slave가 버스를 공유
- Slave는 고유 주소(Address)로 식별됨
- START / STOP 조건을 통해 프레임 경계를 정의

### 3.3 사용 목적

- 온습도 센서 등 저속 주변장치
- 배선 수를 최소화해야 하는 경우
- 다수의 디바이스를 하나의 버스로 연결할 때

---

## 4. SPI 개념

### 4.1 기본 특징

- 동기(Synchronous) 통신 방식
- MOSI, MISO, SCK, NSS로 구성
- Full-Duplex 통신 지원

### 4.2 동작 개념

- Master가 클럭(SCK)을 생성
- Slave Select(NSS)를 통해 대상 디바이스 선택
- 클럭 에지에 맞춰 데이터 시프트

### 4.3 사용 목적

* Flash Memory 등 고속 데이터 전송
* 짧은 거리, 높은 처리량이 필요한 경우

---

## 5. 통신 방식 비교 요약

|항목|UART|I2C|SPI|
|----|----|---|---|
|동기 여부|비동기|동기|동기|
|클럭 라인|없음|있음|있음|
|배선 수|적음|매우 적음|많음|
|속도|낮음|중간|높음|
|연결 구조|1:1|1:N|1:N|

---
