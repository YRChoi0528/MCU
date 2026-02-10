# Timer_Registers.md

## 요약
해당 문서는 STM32F103의 Timer(TIM) 레지스터 구조를 정리하고,  
`tim.c / tim.h` 코드가 **어떤 레지스터를 어떻게 제어하는지**를 연결해 설명한다.  

---

## 1. TIM 주변장치(타이머)와 레지스터 구조 개요
STM32의 TIM은 “카운터(CNT)를 어떤 클럭으로 얼마나 빠르게 증가/감소시키고,  
특정 조건(ARR 도달, CCR 비교 일치 등)에서 이벤트/출력을 만들 것인가”를 레지스터로 설정한다.

TIM 레지스터는 타이머 종류(General-purpose / Advanced-control 등)에 따라 일부 차이가 있지만,  
학습/실습에서 자주 만지는 핵심 레지스터는 대부분 공통이다.

|범주|대표 레지스터|의미|
|---|---|---|
|기본 Time Base|CR1, PSC, ARR, CNT, EGR, SR|주기/카운터 동작|
|채널(OC/PWM/IC)|CCMR1/2, CCER, CCR1~4|채널 모드/출력/캡처|
|인터럽트/이벤트|DIER, SR|이벤트 enable/flag (깊게 다루지 않음)|

---

## 2. TIM 클럭과 RCC의 관계
TIM 레지스터는 존재하지만, **해당 TIM의 버스 클럭이 활성화되지 않으면 동작하지 않는다.**

- TIM2~TIM7 등 많은 타이머는 **APB1**에 연결
- TIM1(Advanced) 등 일부는 **APB2**에 연결

실습 코드에서는 보통 아래처럼 활성화한다(예시).

```c
// 예시: TIM2 클럭 Enable (APB1)
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

// 예시: TIM1 클럭 Enable (APB2)
RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
```

> “어느 버스에 연결되어 있는지”는 데이터시트/레퍼런스 매뉴얼 표를 확인하면 된다.

---

## 3. TIM Time Base 핵심 레지스터 (PSC, ARR, CNT, CR1)

### 3.1 PSC (Prescaler)
- 역할: TIM 입력 클럭을 분주하여 **CNT가 증가하는 속도**를 결정
- 동작: 실제 분주비는 `(PSC + 1)`

예)
- PSC=71 → 72분주 → (72MHz 기준) 1MHz 카운터 클럭 구성 가능

### 3.2 ARR (Auto-Reload Register)
- 역할: “주기(Period)”의 Top 값
- 업카운트 기준: CNT가 0→ARR까지 진행하면 1주기 완료로 보고 Update 이벤트 발생

### 3.3 CNT (Counter)
- 역할: 실제 카운터 값(증가/감소)
- 읽어서 현재 진행 상태를 확인하거나, 특정 값으로 초기화 가능

### 3.4 CR1 (Control Register 1)에서 주로 보는 비트들
CR1은 타이머의 **기본 동작 모드**를 담는다.

- CEN: Counter Enable (타이머 시작/정지)
- DIR: Up/Down(증가/감소) 방향
- CMS: Center-aligned 모드
- ARPE: ARR preload (ARR 갱신 타이밍 제어)

> 비트명/정확한 위치는 타이머 종류에 따라 상세가 다를 수 있으므로, “역할” 중심으로 이해하고 레퍼런스 매뉴얼로 최종 확인한다.

---

## 4. EGR / SR / DIER : 이벤트 생성 및 상태 플래그(최소)
### 4.1 EGR (Event Generation Register)
- 역할: 소프트웨어로 **Update Event 등 이벤트를 강제로 발생**
- 대표: UG(Update Generation) 비트

실습에서 “PSC/ARR 값을 즉시 적용”시키고 싶을 때 EGR(UG)를 사용하기도 한다.

### 4.2 SR (Status Register)
- 역할: Update, CCx match 등 **이벤트 발생 상태(flag)** 가 저장됨
- 대표: UIF(Update Interrupt Flag), CC1IF~ 등

### 4.3 DIER (DMA/Interrupt Enable Register)
- 역할: 각 이벤트에 대해 인터럽트/DMA 요청을 enable
- 본 문서에서는 구조만 인지하고, 상세 사용은 별도 문서(Interrupt)로 분리하는 편이 좋다.

---

## 5. 채널 레지스터: CCMR / CCER / CCR (OC/PWM/IC 공통 뼈대)

### 5.1 CCMR1 / CCMR2 (Capture/Compare Mode Register)
- 역할: 채널의 “모드”를 결정
  - Output Compare(OC) / PWM 모드 선택
  - Input Capture(IC) 입력 선택/필터 등

일반적으로
- CH1~CH2는 CCMR1
- CH3~CH4는 CCMR2
형태로 나뉘어 있다(타이머에 따라 채널 수가 다를 수 있음).

### 5.2 CCER (Capture/Compare Enable Register)
- 역할: 채널 출력 enable, polarity(반전) 등을 설정
- 예) CC1E(채널1 enable), CC1P(채널1 polarity) 등

### 5.3 CCR1~CCR4 (Capture/Compare Register)
- 역할:
  - OC/PWM: “비교 기준(듀티/토글 시점)” 값
  - IC: “캡처된 CNT 값”이 저장되는 레지스터

---

## 6. (고급 타이머) BDTR 등 확장 레지스터 개요
TIM1 같은 Advanced-control 타이머에는 PWM 출력 안정성을 위한 추가 레지스터가 있다.

- BDTR: Break and Dead-Time (브레이크/데드타임 등)
- RCR: Repetition Counter (반복 카운트)

입문 실습(PWM 출력 등)에서는
> “추가 레지스터가 존재한다” 정도만 알고, 필요할 때만 깊게 들어가면 된다.

---

## 7. stm32f10x_tim.h : 설정 구조체(Typedef)로 레지스터를 추상화
표준 라이브러리는 “레지스터 직접 비트 조작” 대신, 설정 구조체를 통해 주요 파라미터를 전달하도록 설계되어 있다.

```c
typedef struct
{
  u16 TIM_Period;          /* ARR */
  u16 TIM_Prescaler;       /* PSC */
  u16 TIM_ClockDivision;   /* CR1의 CKD */
  u16 TIM_CounterMode;     /* CR1의 DIR/CMS 관련 */
} TIM_TimeBaseInitTypeDef;
```

```c
typedef struct
{
  u16 TIM_OCMode;          /* CCMR의 OC 모드 (PWM1 등) */
  u16 TIM_Channel;         /* CH1~CH4 */
  u16 TIM_Pulse;           /* CCR 값 */
  u16 TIM_OCPolarity;      /* CCER polarity */
} TIM_OCInitTypeDef;
```

> 구조체 멤버 이름이 곧 “어느 레지스터를 설정하는지”를 강하게 암시한다.

---

## 8. stm32f10x_tim.c : 함수가 실제로 조작하는 레지스터 흐름

### 8.1 TIM_TimeBaseInit() : ARR, PSC, CR1 설정
`TIM_TimeBaseInit()` 내부는 핵심적으로 다음을 수행한다.

```c
// Autoreload(ARR) 설정
TIMx->ARR = TIM_TimeBaseInitStruct->TIM_Period;

// Prescaler(PSC) 설정
TIMx->PSC = TIM_TimeBaseInitStruct->TIM_Prescaler;

// CounterMode, ClockDivision -> CR1에 반영
TIMx->CR1 &= CR1_CKD_Mask & CR1_CounterMode_Mask;
TIMx->CR1 |= (u32)TIM_TimeBaseInitStruct->TIM_ClockDivision |
             TIM_TimeBaseInitStruct->TIM_CounterMode;
```

정리하면,
- **TIM_Period → ARR**
- **TIM_Prescaler → PSC**
- **TIM_CounterMode / TIM_ClockDivision → CR1**

---

### 8.2 TIM_Cmd() : CEN 비트로 시작/정지
타이머 구동은 보통 다음 순서로 한다.

1) RCC로 TIM 클럭 enable  
2) TimeBase 설정(PSC/ARR/CR1)  
3) 채널 설정(필요한 경우)  
4) `TIM_Cmd(TIMx, ENABLE)` 로 시작

> 결국 “시작/정지”는 CR1의 CEN 비트로 귀결된다.

---

### 8.3 PWM(출력 비교) 설정 흐름(요약)
PWM을 만드는 경우, 대표적으로 다음 레지스터 그룹이 함께 설정된다.

- PSC/ARR (주기)
- CCMR(채널 모드 = PWM1/PWM2 등)
- CCER(채널 enable/polarity)
- CCR(듀티)
- 필요 시 EGR(UG)로 즉시 적용

표준 라이브러리에서는 OC 관련 함수들이 CCMR/CCER/CCR을 적절히 채우는 형태다.

---

## 9. 실습 관점 체크리스트(“레지스터 흐름” 관점)
PWM 실습(Timer Counter PWM) 수준에서, 최소로 점검할 항목은 아래다.

- [ ] RCC에서 해당 TIM 클럭 enable 했는가?
- [ ] PSC/ARR 설정이 원하는 주기와 일치하는가?
- [ ] PWM 핀을 Alternate Function Output으로 설정했는가?(GPIO 설정)
- [ ] 채널(CCMR/CCER/CCR)이 올바른 모드/듀티/극성인가?
- [ ] 마지막에 TIM_Cmd로 실제로 시작했는가?

이 체크리스트만으로도 “동작 안 함”의 대부분을 빠르게 분류할 수 있다.
