# SPI Registers

## 요약

해당 문서는 STM32F10x의 **SPI(Serial Peripheral Interface)** 주변장치에 대해 레지스터 관점에서 정리한 문서이다. </br>
제공된 실습 코드(`main.c`, `at25fxx.c`)를 바탕으로 **SPI 레지스터 구성, 각 비트의 의미, 송수신 동작흐름** 및 **초기화 함수의 레지스터 제어 원리**를 중심으로 설명한다.

---

## 1. SPI 관련 클럭 및 핀

### 1.1 클럭

- **SPI1** : APB2 클럭(최대 72MHz) 사용
- **SPI2**, **SPI3** : APB1 클럭(최대 36MHz) 사용
- 실습에서는 SPI1을 사용하므로, 동작 전 반드시 `RCC_APB2Periph_SPI1`클럭을 활성화 해준다.

### 1.2 핀 구성(SPI1 Master 기준)
- **SCK(클럭)** : PA5 (Alternate Function Push-Pull 설정)
- **MISO(Master In Slave Out) : PA6 (Input Floating 권장, 실습에서는 AF_PP로 설정.)
- **MOSI(Master Out Slave In) : PA7 (Alternate Function Push-Pull 설정)
- NSS/CS(Chip Select) : PA4 (하드웨어 NSS 대신 GPIO `Out_PP`로 설정하여 소프트웨어적으로 제어)

> MISO 핀은 외부 Slave가 Master로 신호를 보내는 핀이므로 Input_Floating 또는 Input Pull-Up 으로 설정하는 것을 권장한다.
> STM32 하드웨어 특성상 Master 모드일 때 AF_PP로 설정해도 내부적으로 입력으로 우회되어 동작하기 때문에 실습 코드가 정상 작동하는 것이다.

---

## 2. SPI_CR1 (Control Register 1)

SPI 모듈의 활성화, Master/Slave 모드, 클럭 극성 및 위상, 데이터 포맷 등 통신의 핵심 규칙을 설정한다.
|비트|이름|설명|
|----|----|----|
|Bit[0]|CPHA|Clock Phase. <br> 데이터를 캡처하는 위상(0:첫 번째 엣지, 1:두 번째 엣지)을 설정한다.|

Bit[15],BIDIMODE,1-Line 양방향 통신 모드를 설정한다. (실습은 0: 2-Line 단방향)
Bit[11],DFF,"Data Frame Format.  0이면 8-bit, 1이면 16-bit 프레임으로 설정한다."
Bit[9],SSM,Software Slave Management.  1로 설정 시 내부 NSS 핀의 상태를 SSI 비트 값으로 대체한다.
Bit[8],SSI,"Internal Slave Select.  SSM이 1일 때, 이 비트의 값이 실제 하드웨어 NSS 핀의 상태를 대신한다."
Bit[7],LSBFIRST,"Frame Format.  0이면 MSB부터, 1이면 LSB부터 전송한다."
Bit[6],SPE,SPI Enable.  SPI 모듈의 전체 전원을 활성화한다.
Bit[5:3],BR[2:0],Baud Rate Control.  입력 클럭(APB2)을 분주하여 통신 속도를 결정한다. (예: 100 = fPCLK/32)
Bit[2],MSTR,"Master Selection.  0이면 Slave, 1이면 Master 모드로 동작한다."
Bit[1],CPOL,"Clock Polarity.  대기 상태(Idle)일 때 클럭의 기본 상태(0:Low, 1:High)를 설정한다."
