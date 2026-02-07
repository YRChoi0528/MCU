# stm32f103_pin

![STM32F103 핀맵](../images/STM32F103_Pin.png)

## 설명에 앞서
- 각 챕터에서 다루지 않는 내용들은 추가 설명을 간략하게 하겠다. 그 외 추가 설명이 없는 것은 각 챕터에서 다루겠다.

---

## 핀 역할 설명

### 1. 전원 관련 핀
- VDD / VSS : 디지털 영역
- VDDA / VSSA : 아날로그 영역
- VBAT : 저전력·항상 유지 영역

---

### 2. 클럭 관련 핀
- OSC_IN / OSC_OUT : 고속 외부 크리스탈 연결
- OSC32_IN / OSC32_OUT : 저속 외부 크리스탈 연결

---

### 3. 리셋·부트 관련 핀
NRST : 외부 리셋 입력
BOOT0 / BOOT1 : 부트 모드 선택
|BOOT1|BOOT0|BOOT MODE|설명 및 용도|
| --- | --- | ------- | ---------- |
|X|0|User Flash Memory|일반 실행: 사용자가 작성하여 다운로드한 프로그램이 실행되는 기본 모드입니다.|
|0|1|System Momory|공장 출하 부트로더: ST에서 미리 내장해둔 코드를 실행합니다. 주로 UART 인터페이스를 통해 새로운 펌웨어를 구울 때 사용합니다.|
|1|1|Embedded SRAM|램 실행: Flash 메모리가 아닌 SRAM에서 직접 코드를 실행합니다. 주로 개발 단계에서 Flash 쓰기 횟수를 아끼거나 빠른 테스트를 할 때 사용합니다.|

---

### 4. GPIO 포트 핀
- 기본적으로 범용 입출력
- 설정에 따라 주변장치 기능으로 다중화(Multiplexing) 됨
- 예:
- - PA9 → USART1_TX
  - PB6 → I2C1_SCL
  - PA5 → SPI1_SCK

---

### 5. TIMER 관련 핀
- 각 타이머의 채널(CH1~CH4), 보조 입력(ETR), 보완 출력(CHxN)

---

### 6. 통신 인터페이스 핀
- USART1/2/3 : TX, RX, CTS, RTS, CK
- SPI1/2 : NSS, SCK, MISO, MOSI
- I2C1/2/ : SCL, SDA, SMBA
- CAN : RX, TX
- USB : DM, DP

---

### 7. 디버그 및 시스템 핀
- JTMS /JTCK / JTDI / JTDO
- SWDIO / SWCLK
- MCO
