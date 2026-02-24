# EXTI_Concepts.md

## 요약
EXTI는 외부 신호의 변화를 감지하여 </br>
(1) CPU가 개입하는 **인터럽트(Interrupt)** 또는 </br>
(2) CPU 개입 없이 내부 신호로만 전달되는 **이벤트(Event)** </br>
를 발생시키는 하드웨어 블록이다.

EXTI는 **감지**를 담당하고, </br>
실제로 CPU가 ISR로 진입하는 제어는 NVIC가 담당한다. </br>

---

## 1. EXTI란 무엇인가

EXTI (External Interrupt)는 GPIO 핀의 신호 변화뿐만 아니라 PVD(전압 감지), RTC Alarm, USB Wakeup 등 시스템의 주요 변화에 따라 </br>
다음 중 하나를 발생시키는 컨트롤러이다.

- Interrupt : NVIC를 통해 CPU가 ISR을 실행
- Event : CPU를 중단시키지 않고 내부 이벤트 신호만 발생

즉, EXTI는 "외부 입력 → 내부 사건 발생"의 관문 역할을 한다.

---

## 2. EXTI Line의 의미

EXTI는 Line0 ~ Line18까지 존재한다.

Line 번호는 Pin 번호와 동일(예: PA0, PB0, PC0 → 모두 EXTI Lin0 후보) </br>
하지만 Line16-18은 다른 곳과 연결되어 있다.

- Line16 : PVD Output
- Line17 : RTC Alarm event
- Line18 : USB Wakeup from suspend event

`AFIO_EXTICR` 설정에 의해, 한 라인에는 **하나의 포트만 연결** 될 수 있다.

---

## 3. AFIO_EXTICR

AFIO_EXTICR은 GPIO 핀이 EXTI Line과 연결하기 위한 '교환기' 역할을 한다. </br>
4개의 레지스터(`AFIO_EXTICR1~4`)가 존재하며, 각 레지스터는 라인의 포트를 결정한다.

- 코드 예시 : ` GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);`

---

## 4. 인터럽트 vs 이벤트

EXTI는 두 가지 모드를 가진다.

- Interrupt Mode : **NVIC**로 신호를 보내 CPU의 현재 작업을 중단하고 ISR을 실행한다.
- Event Mode : CPU 개입 없이 내부 이벤트 라인으로 전달되어 `WFE` 명령어로 잠든 CPU를 꺠우거나, 다른 주변장치의 동작을 시작하는 트리거로 사용된다.

---

## 5. 트리거 및 소프트웨어 제어

- **에지 트리거** : Rising, Falling, 또는 둘 다(Both) 감지 가능하도록 설정할 수 있다.
- **SWIER(Software Interrupt Event Register)** : 하드웨어 핀 입력 없이도 **코드에서 비트를 쓰는 것만으로 강제 인럽트**를 발생시킬 수 있어 디버깅에 유리하다.

---

## 6. 인터럽트 완성을 위한 6단계 조건

인터럽트가 정상적으로 실행되려면 다음 체크리스트를 모두 만족해야 한다.

1. **GPIO 설정** : 해당 핀을 `Input Floating` 또는 `Pull-Up/Down` 모드로 설정
2. **AFIO 활성화** : `AFIO_EXTICR`을 통해 포트와 라인 연결
3. **트리거 설정** : Rising 또는 Falling 설정
4. **마스크 해제** : `EXTI_IMR`에서 해당 라인 허용
5. **NVIC 승인** : NVIC에서 해당 채널 Enable 및 우선순위 설정
6. **PR Bit 클리어** : ISR 종료 전 `EXTI_PR`의 해당 Bit를 **1을 써서 클리어**해야 인터럽트 무한 반복을 방지할 수 있다

---

## 7. EXTI와 NVIC 관계

EXTI는 신호를 만드는 **발생원(Source)**이고, NVIC는 이를 관리하는 **관제소(Controller)**이다.
- **독립 핸들러** : EXTI0-4 (개별적인 인터럽트 주소 보유).
- **공유 핸들러** : EXTI5-9, EXTI10-15 는 하나의 ISR 공유. 공유 핸들러 내부에서는 어떤 라인에서 신호가 왔는지 `EXTI_GetITStatus`로 확인해야 한다.
