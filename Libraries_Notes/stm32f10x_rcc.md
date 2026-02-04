# stm32f10x_rcc.h / stm32f10x_rcc.c 정리

## 역할
`stm32f10x_rcc` 모듈은 STM32F10x 마이크로컨트롤러의 Reset and Clock Control(RCC) 블록을 제어하기 위한 표준 펌웨어 라이브러리이다.
시스템 클럭 소스 선택, 클럭 분주, 주변장치 클럭 게이팅, 리셋 제어, 클럭 상태 조회 기능을 함수 단위로 제공한다.

---

## 파일 구성
- **stm32f10x_rcc.h**
  - RCC 라이브러리에서 제공하는 함수들의 *프로토타입*
  - RCC 설정 결과를 담기 위한 `RCC_ClocksTypeDef` 구조체
  - RCC 설정에 사용되는 *매크로 상수* 및 파라미터 검사용 매크로
- **stm32f10x_rcc.c**
  - 헤더에 선언된 함수들의 실제 구현
  - RCC 레지스터(`RCC->CR`, `RCC->CFGR`, `RCC->APB2ENR` 등)에 대한 접근 코드 포함

---

## 타입 정의
```c
typedef struct
{
  u32 SYSCLK_Frequency;
  u32 HCLK_Frequency;
  u32 PCLK1_Frequency;
  u32 PCLK2_Frequency;
  u32 ADCCLK_Frequency;
} RCC_ClocksTypeDef;
```

이 구조체는 현재 RCC 설정을 기준으로 계산된 각 클럭 도메인의 주파수를 저장하기 위해 사용된다.
측정값이 아니라, 설정 레지스터를 기반으로 *계산된 결과*를 담는다.

---

## 매크로 상수의 성격
`stm32f10x_rcc.h`에는 C 열거형(enum)은 정의되어 있지 않으며,
대신 **매크로 상수(`#define`) 형태로 모든 설정 값이 제공**된다.

예를 들어 HSE 설정은 다음과 같이 정의된다.

```c
#define RCC_HSE_OFF     ((u32)0x00000000)
#define RCC_HSE_ON      ((u32)0x00010000)
#define RCC_HSE_Bypass  ((u32)0x00040000)
```

이 값들은 RCC 제어 레지스터의 특정 비트 조합을 의미하며,
함수 인자로 전달되어 내부에서 그대로 레지스터에 반영된다.

---

## 파라미터 검증 매크로
헤더 파일에는 설정 값의 유효성을 검사하기 위한 매크로가 다수 포함되어 있다.

```c
#define IS_RCC_HSE(HSE) \
  ((HSE == RCC_HSE_OFF) || (HSE == RCC_HSE_ON) || (HSE == RCC_HSE_Bypass))
```

이 매크로들은 라이브러리 함수 내부에서 `assert_param()`과 함께 사용되며,
허용되지 않은 설정 값이 전달되는 것을 방지하기 위한 목적이다.

---

## 제공 기능 분류

### 1) 시스템 클럭 소스 제어
- HSI, HSE, PLL 설정 및 활성화
- 시스템 클럭(SYSCLK) 소스 선택

### 2) 버스 클럭 분주
- AHB(HCLK)
- APB1(PCLK1)
- APB2(PCLK2)

각 버스에 공급되는 클럭의 분주 비율을 설정한다.

---

### 3) 주변장치 클럭 게이팅
- AHB / APB1 / APB2 버스별 주변장치 클럭 Enable/Disable

주변장치는 해당 버스 클럭이 활성화되어야만 동작한다.

---

### 4) 주변장치 리셋 제어
- APB1, APB2 주변장치 리셋
- Backup 도메인 리셋

---

### 5) 특수 클럭 설정
- ADC 클럭
- USB 클럭
- RTC 클럭
- LSE / LSI 설정
- MCO 출력 설정

---

### 6) 클럭 상태 및 인터럽트
- 클럭 준비 완료 플래그 조회
- RCC 인터럽트 설정 및 상태 확인
- Clock Security System(CSS) 제어

---

### 7) 클럭 주파수 계산
- 현재 설정을 기준으로 각 클럭 도메인의 주파수 계산
- 결과는 `RCC_ClocksTypeDef` 구조체에 저장

---

## 정리
`stm32f10x_rcc.h`는 함수 선언과 매크로 상수 중심으로 구성된 인터페이스 헤더이며,
설정 값은 열거형이 아닌 **매크로 상수 조합**으로 표현된다.
실제 레지스터 제어 로직은 `stm32f10x_rcc.c`에 구현되어 있으며,
이 모듈은 STM32F10x 시스템 클럭 구조의 중심 역할을 담당한다.
