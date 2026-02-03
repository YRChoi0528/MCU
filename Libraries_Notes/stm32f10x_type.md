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

**volatile의 의미**
- 변수를 선언할 때 앞에 volatile을 붙이면 컴파일러는 해당 변수를 최적화에서 제외하여 항상 메모리에 접근하도록 만든다.
- 즉, 해당 변수는 언제든지 값이 바뀔 수 있으니까 항상 메모리에 접근하라고 컴파일러에게 알려주는 것이다.
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
```
- `u32`는 "부호 없는 32비트 정수"라는 의미로 쓰기 위한 별칭이다.
- 코드에서 `u32 x;`라고 보면 "정확히 어떤 크기의 정수인지"가 즉시 드러나도록 만든다.

### (B) volatile 타입
```c
typedef volatile unsigned long vu32;
```
- `vu32`는 "부호 없는 32비트 정수이지만, 값이 외부 요인으로 바뀔 수 있으므로 최적화로 접근을 생략하면 안 되는" 대상을 표현할 때 사용한다.
#### 예제: 인터럽트에 의해 바뀌는 변수
```c
int flag = 0;

while(flage == 0)
{
  // 대기
}
```
이 코드의 의도는:
- 인터럽트에서 `flag = 1;`이 되면
- `while`을 빠져나온다
컴파일러가 보기엔?
- `flag`는 이 함수 안에서 한 번도 바뀌지 않음
- 그럼 이렇게 생각할 수 있음:
> "flag는 항상 0이네. 이 while은 무한 루프네."
그래서 최적화 후에는:
```c
while (1)
{
  // 대기
}
```
처럼 바뀔 수도 있다.
→ 인터럽트가 `flag`를 바꿔도, CPU는 다시 메모리를 읽지 않는다.

### (C) 상태 enum + 검증 매크로
```c
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) ((STATE == DISABLE) || (STATE == ENABLE))
```
- `FunctionalState`는 단순히 0/1이 아니라 "ENABLE/DISABLE이라는 의미"를 코드에 남기는 용도다.
- `IS_FUNCTIONAL_STATE`는 함수 인자 검증에 쓰이는 패턴이며, `assert_param()`과 결합되는 경우가 많다.
