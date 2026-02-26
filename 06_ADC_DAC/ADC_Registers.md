# ADC Registers

## 1. 요약

해당 문서는 STM32F10x의 **ADC** 주변장치에 대해 레지스터 관점에서 정리한 문서이다. <br>
실습 코드를 바탕으로 각 레지스터별 섹션을 나누고, **각 레지스터의 핵심 bit 구성과 의미**, 그리고 실제 코드가 하드웨어 레지스터를 어떻게 제어하여 변환을 수행하는지 분석한다.

---

## 2. ADC_CR1 (Control Register 1)

![ADC_CR1](../images/ADC_CR1.png)

ADC의 해상도, 아날로그 와치독(Watchdog), 인터럽트 활성화 및 스캔 모드 등 부가적인 제어 기능을 설정하는 레지스터이다.

비트,이름,설명
Bit[23],AWDEN,Analog Watchdog Enable on Regular Channels.  일반 채널에 대해 아날로그 워치독 기능을 활성화한다.
Bit[13],JAUTO,Automatic Injected Group Conversion.  일반 채널 변환이 끝난 후 자동으로 삽입(Injected) 채널 변환을 시작할지 설정한다.
Bit[11],DISCEN,Discontinuous Mode on Regular Channels.  일반 채널에서 불연속(Discontinuous) 변환 모드를 사용할지 설정한다.
Bit[8],SCAN,"Scan Mode Enable.  1로 설정 시, ADC_SQR 레지스터에 등록된 여러 채널을 순차적으로 스캔하며 변환하는 모드를 켠다. 단일 채널 변환 시에는 0으로 둔다."
Bit[7],JEOCIE,Interrupt Enable for Injected Channels.  삽입 채널의 변환 완료(JEOC) 인터럽트를 활성화한다.
Bit[6],AWDIE,Analog Watchdog Interrupt Enable.  아날로그 전압이 설정된 임계치를 벗어날 때 발생하는 인터럽트를 켠다.
Bit[5],EOCIE,Interrupt Enable for EOC.  일반 채널의 변환 완료(EOC) 플래그가 1이 될 때 하드웨어 인터럽트를 발생시키도록 설정한다.

---

## 3. ADC_CR2 (Control Register 2)

![ADC_CR2](../images/ADC_CR2.png)


---

## 4. ADC_SMPR (Sample Time Register)

![ADC_SMPR](../images/ADC_SMPR.png)


---

## 5. ADC_SQR (Regular Sequence Register)

![ADC_SQR1](../images/ADC_SQR1.png)
![ADC_SQR2](../images/ADC_SQR2.png)
![ADC_SQR3](../images/ADC_SQR3.png)


---

## 6. ADC_SR (Status Register)

![ADC_SR](../images/ADC_SR.png)


---

## 7. ADC_DR (Data Register)

![ADC_DR](../images/ADC_DR.png)
