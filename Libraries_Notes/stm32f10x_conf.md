# stm32f10x_conf.h 정리

## 역할(한 줄 요약)
`stm32f10x_conf.h`는 **프로젝트에서 사용할 펌웨어 라이브러리 기능(주변장치 포함 여부, 디버그 assert, 클럭 기준값 등)을 선택하는 설정 파일**이다.

---

## 이 파일을 언제 확인/수정하는가
- **새 주변장치를 쓰기 시작할 때**
  - 예: GPIO만 쓰다가 UART를 추가하는 시점 → 해당 모듈이 “포함/활성화” 상태인지 확인
- **디버깅 강화를 원할 때**
  - `assert_param()`을 활성화해서 잘못된 인자/상태를 조기에 잡고 싶을 때
- **외부 오실레이터 값(HSE)이 보드와 다를 때**
  - 보드의 외부 크리스탈이 8MHz가 아닌 경우

> 이 파일은 “라이브러리 코드”라기보다 **프로젝트 설정 스위치**에 가깝다.

---

## 핵심 구성 요소

### 1) 헤더 가드(중복 include 방지)
- `#ifndef __STM32F10x_CONF_H` ~ `#endif`

### 2) 공통 타입 포함
- `#include "stm32f10x_type.h"`
- 라이브러리 전반에서 쓰는 `u32`, `u8` 등의 타입을 conf에서도 사용하기 때문에 포함한다.

### 3) 디버그 모드 / assert 활성화
- 파일 상단에 `// #define DEBUG 1` 형태로 존재
- `DEBUG`가 정의되면(= 주석(//) 삭제) `assert_param(expr)`매크로가 활성화 된다.

### 4) 주변장치(모듈) 포함 여부 스위치
- `_GPIO`, `_GPIOA`, `_RCC`, `_NVIC` 같은 매크로들이 켜져/꺼져 있음
- 해당 파일 기준 `_GPIO`같은 매크로가 정의된 경우에만 관련 코드가 포함되도록 구성되어 있다.

### 5) HSE_Value (외부 오실레이터 기준값)
- 예시: `#define HSE_Value ((u32)8000000)`
- 클럭 설정/계산(지연, PLL 설정 등)에서 기준으로 쓰인다.

---

## 예시 발췌와 해설

### (A) DEBUG / assert_param
```c
// #define DEBUG    1

#ifdef  DEBUG
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((u8 *)__FILE__, __LINE__))
  void assert_failed(u8* file, u32 line);
#else
  #define assert_param(expr) ((void)0)
#endif
```
- `DEBUG`가 켜지면( `#define DEBUG 1` )
- - `assert_param(expr)`은 `expr`이 거짓일 때 `assert_failed(u8* file, u32 line);`을 호출한다.
  - 즉, "문제 발생 위치(파일, 라인)"까지 알 수 있게 된다.
  - `__FILE__` 과 `__LINE__`은 C 전처리기가 제공하는 표준 매크로이며, 소스 파일명과 줄 번호를 얻을 수 있다.
  - `assert_failed(u8* file, u32 line)`은 선언만 제공하고 실제 정의는 사용자가 구현해야 한다.
- `DEBUG`가 꺼지면( `//#define DEBUG 1` )
- - `assert_param(expr)`은 아무것도 하지 않는다.
- 실습에서는 기본적으로 비활성 상태로 두는 경우가 많지만, 개인 학습/디버깅 단계에서 한 번쯤 활성화해서 동작을 확인하는 것을 권장한다.

### (B) 모듈 포함 스위치
```c
#define _GPIO
#define _GPIOA
#define _GPIOB
#define _GPIOC
#define _AFIO
```
- `_GPIO`가 "GIPO 모듈 전체 사용"
- `_GPIOA/_GPIOB/...`은 "포트 단위" 또는 "세부 기능 단위"를 세분화한 스위치 성격(ON/OFF 성격).
- 라이브러리 소스에서 해당 매크로가 정의되어 있을 때만 코드가 컴파일에 포함되는 형태를 가정한다.

### (C) HSE_Value
```
#define HSE_Value ((u32)8000000)
```
- 보드의 외부 오실레이터가 8MHz가 아니라면 반드시 실제 값으로 맞춰야 한다.
- 클럭/타이머/지연 계산이 전반적으로 영향을 받는다.
- 오실레이터는 "주기적인 전기 신호(클록)를 스스로 생성하는 능동형 발진 부품"이다.
```
MCU는 스스로 시간의 흐름을 알 수 없다.
따라서 "지금 이 명령을 실행하고, 다음 명령은 언제 실행할지"를 결정하려면 외부에서 일정한 주기로 반복되는 신호가 필요하다.
이 반복 신호를 만들어 주는 것이 오실레이터 이다.
```
