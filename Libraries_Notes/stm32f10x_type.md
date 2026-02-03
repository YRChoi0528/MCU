# stm32f10x_type.h 정리

## 역할(한 줄 요약)
`stm32f10x_type.h`는 **라이브러리 전체에서 공통으로 사용할 정수 타입/volatile 타입/상태 enum 등을 정의하여 코드 의미를 통일하는 파일**이다.

---

## 이 파일을 어떻게 다뤄야 하는가
- 원칙적으로 **수정하지 않는다.**
- 목적은 “외우기”가 아니라:
  - `u32`, `vu32`, `FunctionalState` 같은 타입/상태가 의미하는 바를 정확히 이해하는 것

---

## 구성 요소

### 1) 고정 의미 타입(별칭 typedef)
예시:
- `u8, u16, u32` : 부호 없는 정수
- `s8, s16, s32` : 부호 있는 정수

> 포인트: C의 `int/long`은 환경에 따라 크기가 달라질 수 있다.  
> 라이브러리에서는 “의미가 고정된 이름”을 쓰려는 목적이 있다.

### 2) volatile / const 변형 타입
예시:
- `vu32` : `volatile unsigned long`
- `vuc32` : `volatile unsigned long const` (읽기 전용 volatile)

**volatile의 의미(실습에서 중요한 이유)**
- 컴파일러가 “값이 안 바뀐다”고 가정하고 최적화해버리면 안 되는 대상에 사용한다.
- 하드웨어 레지스터, 인터럽트/하드웨어에 의해 바뀌는 값 등에 필요해진다.

### 3) 상태를 표현하는 enum
예시:
- `bool` : `FALSE/TRUE`
- `FlagStatus, ITStatus` : `RESET/SET`
- `FunctionalState` : `DISABLE/ENABLE`
- `ErrorStatus` : `ERROR/SUCCESS`

### 4) 파라미터 검사용 매크로
- `IS_FUNCTIONAL_STATE(STATE)`  
  → “ENABLE/DISABLE만 허용” 같은 방어 체크에 사용된다.

### 5) 타입별 최댓값/최솟값 매크로
- `U8_MAX`, `S16_MIN` 등

---

## 예시 발췌와 해설(필요 최소)

### (A) 타입 별칭
```c
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;
