# NVIC_Concepts.md

## 요약
NVIC는 Cortex-M3 내부에 존재하는 인터럽트 컨트롤러로, </br>
인터럽트의 우선순위 관리와 벡터 분기를 담당한다. </br>
EXTI, TIM, USART 등 모든 인터럽트는 최종적으로 NVIC를 통해 CPU로 전달된다.

---

## 1. 인터럽트란 무엇인가

인터럽트(Interrupt)는 현재 실행 중인 작업을 일시 중단하고 </br>
우선 처리해야 할 사건을 수행하는 메커니즘이다.

동작 흐름:

1) 현재 코드 실행
2) 인터럽트 발생
3) PC(Program Counter), 상태 레지스터 자동 스택 저장
4) ISR(Interrupt Service Routine) 실행
5) 복귀 후 원래 코드 계속 실행

---

## 2. NVIC의 역할

NVIC (Nested Vectored Interrupt Controller)는

- 인터럽트 허용/차단 관리
- 우선순위 관리
- ISR 주소(벡터) 관리

를 수행한다.

NVIC의 세 가지 역할이 각각 수행되지 않을 경우 발생하는 문제를 기능별로 정리하면 다음과 같다.

### 2.1 인터럽트 허용/차단 관리

**정상 기능**
- 특정 IRQ(Interrupt Request)를 Enable/Disable 가능
- 필요 없는 인터럽트 차단 가능
- 저전력 모드 진입 시 선택적 허용 가능

**수행하지 못할 경우 발생하는 문제**
1. **모든 인터럽트가 항상 활성화**
   → 원하지 않는 인터럽트가 계속 CPU를 중단 시킴
   → 실시간 제어 불안정
   
2. **특정 인터럽트를 차단할 수 없음**
   → 초기화 중 인터럽트 발생 가능
   → 설정 중간 상태에서 ISR 실행 → 오동작
   
3. **저전력 제어 불가능**
   → Sleep/WFI 동작 비정상

즉, 시스템 제어권을 상실하게 된다.

### 2.2 우선순위 관리가 불가능한 경우

**정상 기능**
- 중요한 인터럽트를 먼저 처리
- Nested(중첩) Intterupt 가능
- 실시간성 보장

**수행하지 못할 경우 발생하는 문제**
1. **중요 인터럽트가 지연됨**
   → 데이터 손실 발생
   
2. **인터럽트 중첩 불가**
   → 긴 ISR 실행 중 긴급 인터럽트 지연
   
3. **실시간 시스템 붕괴**
   → 제어 시스템에서 치명적 오류

우선순위 관리는 실시간 보장의 핵심이다.

### 2.3 ISR 주소(벡터) 관리가 불가능한 경우

**정상 기능**
- IRQ 번호에 따라 정확한 ISR로 분기
- Vector Table 기반 점프

**수행하지 못할 경우 발생하는 문제**
1. **잘못된 ISR 실행**
   → 전혀 다른 코드 실행
   → 메모리 손상 가능
   
2. **HardFault 발생**
   → 잘못된 주소 점프
   → 시스템 정지
   
3. **인터럽트 미처리**
   → Pending bit만 유지
   → 무한 인터럽트 반복

ISR 벡터 관리 실패는 시스템 즉시 정지 수준의 오류로 이어진다.


모든 주변장치 인터럽트는
EXTI → NVIC → CPU 순서로 전달된다.

---

## 3. 인터럽트 벡터의 의미

벡터(Vector)란
각 인터럽트에 대응하는 ISR 시작 주소이다.

NVIC는 IRQ 번호에 따라
미리 정의된 ISR 주소로 분기한다.

예:

EXTI0 → EXTI0_IRQHandler()
TIM2  → TIM2_IRQHandler()

---

## 4. 인터럽트 우선순위 구조

NVIC 우선순위는 두 단계로 구성된다.

1) Preemption Priority
   → 인터럽트 중첩 가능 여부 결정

2) Sub Priority
   → 동일 Preemption 그룹 내 정렬 기준

값이 작을수록 높은 우선순위를 가진다.

---

## 5. Nested Interrupt (중첩 인터럽트)

높은 우선순위 인터럽트는
낮은 우선순위 ISR 실행 중에도 선점 가능하다.

이를 Nested Interrupt라 한다.

---

## 6. Maskable vs Non-Maskable Interrupt

- Maskable Interrupt
  → NVIC에서 Enable/Disable 가능

- Non-Maskable Interrupt (NMI)
  → 차단 불가능

STM32 대부분의 인터럽트는 Maskable이다.

---

## 7. NVIC와 EXTI, Timer의 관계

EXTI, Timer는 인터럽트 발생원(Source)이고
NVIC는 이를 CPU로 전달하는 중간 제어기이다.

즉,

[Peripheral] → [NVIC] → [CPU]
