# Timer_Registers.md

## 요약
해당 문서는 STM32F103의 Timer(TIM) 레지스터 구조를 정리하고,  
`tim.c / tim.h` 코드가 **어떤 레지스터를 어떻게 제어하는지**를 연결해 설명한다.  

---

## 1. TIM 주변장치(타이머)와 레지스터 구조 개요
STM32의 TIM은 **카운터(CNT)를 어떤 클럭으로 얼마나 빠르게 증가/감소시키고,  
특정 조건(ARR 도달, CCR 비교 일치 등)에서 이벤트/출력을 만들 것인가**를 레지스터로 설정한다.

|레지스터|역할|
|--------|----|
|TIMx_CNT|실제 카운트 값 저장|
|TIMx_PSC|Prescaler 설정|
|TIMx_ARR|Auto-Reload 값|
|TIMx_CCR1~4|비교/캡처 레지스터|
|TIMx_CR1|제어 레지스터 1|
|TIMx_CR2|제어 레지스터 2|
|TIMx_SMCR|슬레이브 모드 제어|
|TIMx_DIER|인터럽트/DMA 허용|
|TIMx_SR|상태 레지스터|
|TIMx_EGR|이벤트 생성|
|TIMx_CCMR1|채널 1, 2 모드 설정|
|TIMx_CCMR2|채널 3, 4 모드 설정|
|TIMx_CCER|채널 출력 제어|

---

## 2. TIM 클럭 공급과 RCC의 관계
타이머 주변장치는 기본적으로 비활성화 상태이며, 이를 사용하기 위해서는 **RCC**를 통해 해당 타이머에 클럭을 공급해야 한다.

STM32F103에서 타이머는 연결된 버스에 따라 클럭 소스가 달라진다.
|버스|클럭 속도|해당 타이머|
|----|---------|------------|
|APB1|최대 36MHz|TIM2, TIM3, TIM4|
|APB2|최대 72MHz|TIM1|

**왜 APB1 버스트는 36MHz인데 TIM2~4는 72MHz로 동작하는가?**
- STM32F103 설계상 TIM2~4가 연결된 **APB1 버스**는 최대 **36MHz**까지만 감당할 수 있다.
- 하지만, 타이머가 너무 느려지만 정밀한 제어가 어렵기 때문에, 타이머 입구에 클럭을 다시 2배 늘려주는 **배수기**를 배치했다.

![Clock_Tree_TIMx](../images/Clock_Tree_TIMx.png)

그림 속 클럭 트리 로직을 문장으로 풀면 다음과 같다.
**APB1 분주비(Prescaler)가 1이면 그대로 가고, 1이 아니면(×2, ×4 등) 2배를 곱해라.**

우리가 사용하는 일반적인 72MHz 환경을 단계별로 설명하겠다.
  1. SYSCLK : 72MHz로 출발
  2. APB1 Prescaler : APB1 버스의 한계(36MHz)를 지키기 위해 **2분주**를 거친다.
     - 그 결과 주변 장치들이 쓰는 **PCLK1은 36MHz**가 된다.
  3. Timer Multiplier : APB1 Prescaler가 2였기 때문 이때 2를 곱하는 로직이 작동한다.
     - 결과적으로 타이머에 들어가는 클럭은 **36MHz × 2 = 72MHz**가 된다.

**관련 코드**
```c
/* TIM3를 사용하기 위해 APB1 버스 클럭을 활성화 */
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
```
> `RCC_APB1ENR` 레지스터의 `TIM3EN` 비트를 **SET**하여 TIM3 주변장치에 클럭을 공급한다.

---

## 3. Time-Base Unit : 주기와 분주 설정
Time-Base Unit은 타이머의 가장 기본이 되는 '시간 생성' 역할을 한다. PSC, ARR, CNT 세 레지스터의 조합으로 정확한 주기를 만들어낸다.

![Time Base Unit](../images/Time_Base_Unit.png)

### 3.1 레지스터 상세
|레지스터|이름|역할|
|TIMx_PSC|Prescaler|입력 클럭을 나누어 카운터 속도를 조절|
|TIMx_ARR|Auto-Reload|카운터가 도달해야 할 목표값을 설정|
|TIMx_CNT|Counter|실제 숫자를 세는 16비트 레지스터|

### 3.2 레지스터 동작 흐름 (Up-count 시)
  1. **PSC**는 들어오는 고속 클럭을 나누어 **카운터가 1증가하는 데 걸리는 시간**을 결정
  2. **CNT**는 결정된 시간마다 숫자를 하나씩 올림
  3. **CNT**값이 **ARR**과 일치하면, 다음 클럭에서 **Update Event**를 발생시키고 **CNT**를 0으로 리셋한다.
---

## 4. 제어 및 상태 관리 (CR1, SR, EGR)
타이머의 동작 모드를 설정하고 이벤트 발생 여부를 확인하며, 설정값을 하드웨어에 반영하는 과정을 관리하는 레지스터들이다.

### 4.1 TIMx_CR1(Control Register 1)
타이머의 기본적인 동작 방향과 활성화 상태를 제어한다.

![TIMx_CR1](../images/TIMx_CR1.png)

|비트|이름|설명|
|----|----|----|
|Bit[0]|CEN|Counter Enable. `1`일 때 카운터 동작, `0`일 때 정지|
|Bit[1]|UDIS|Update Disable. 업데이트 이벤트 발생 여부를 결정|
|Bit[2]|URS|Update Request Source. 인터럽트/DMA 요청의 발생 소스를 제한|
|Bit[3]|OPM|One-Pulse Mode. `1`설정 시 다음 업데이트 이벤트에서 카운터 정지|
|Bit[4]|DIR|Direction. `0` : Up-counter, `1` : Down-counter|
|Bit[6:5]|CMS|Center-aligned Mode Selection. 중앙 정렬 모드 및 인터럽트 발생 시점 설정|
|Bit[7]|ARPE|Auto-reload Preload Enable. `1`이면 ARR 변경 시 다음 업데이트 이벤트에서 반영|
|Bit[9:8]|CKD|Clock Division. 디지털 필터 등에 사용되는 클럭 분주 설정|

### 4.2 TIMx_SR(Status Register)
타이머 내부에서 발생한 이벤트의 상태 플래그를 저장한다.
하드웨어에 의해 세트되며, 인터럽트 서비스 루틴 등에서 소프트웨어적으로 클리어해야 한다.

![TIMx_SR](../images/TIMx_SR.png)

|비트|이름|설명|
|----|----|----|
|Bit[12:9]|CCxOF|Capture/Compare x Overcapture Flag. 입력 캡처 모드에서만 사용|
|Bit[6]|TIF|Trigger Interrupt Flag. 설정된 트리거 입력 신호(외부 클럭, 내부 동기화 신호 등)가 감지되었을 때 하드웨어가 SET|
|Bit[4:1]|CCxIF|Capture/Compare x Interrupt Flag. 
- 출력 비교 :
- 입력 캡처 :
|
|Bit[0]|UIF||

### 4.3 TIMx_EGR(Event Generation Register)
소프트웨어적으로 이벤트를 강제 발생시켜 레지스터 값을 즉시 업데이트한다.

![TIMx_EGR](../images/TIMx_EGR.png)

|비트|이름|설명|
|----|----|----|
|Bit[6]|TG|
|Bit[4:1]|CCxG|
|Bit[0]|UG|

### 4.2 `TIM_TimeBaseInit()`함수와 레지스터 매핑
```c
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;        // PSC 레지스터에 저장
TIM_TimeBaseStructure.TIM_Period = 10000 - 1;          // ARR 레지스터에 저장
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // CR1 레지스터의 DIR 비트
TIM_TimeBaseStructure.TIM_ClockDivision = 0;           // CR1 레지스터의 CKD 비트

TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
```
---

## 5. Output Compare & PWM (CCMR, CCER, CCR)

---

## 6. 인터럽트 제어 (DIER)

---

