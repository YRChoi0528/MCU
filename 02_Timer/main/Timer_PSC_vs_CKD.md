# Timer - PSC와 CKD 비교

![General_Purpose_Timer_Block_Diagram](../../images/General_Purpose_Timer_Block_Diagram.png)

해당 문서는 STM32 타이머 설정에서 **PSC(Prescaler)**와 **CKD(Clock Division)**의 차이를 정리한다
두 설정 모두 클럭을 나누는(분주) 동작을 하지만, **클럭이 도달하는 최종 목적지**가 다르다는 점이 핵심이다

---

## 1. 개요
프리스케일러(PSC)와 클럭 분주(CKD)는 모두 입력 클럭($f_{TIM}$)의 주파수를 낮추는 역할을 수행한다
- `PSC` : 타이머의 **메인 카운팅 속도**제어
- `CKD` : **입력 필터 및 데드타임 제어**를 위한 샘플링 속도를 제어

---

## 2. PSC (Prescaler)
타이머의 '시간 기준'을 만드는 설정이다.
- **목적지** : 카운터 레지스터(CNT)
- **역할** : 카운터가 숫자를 하나 올리는 데 걸리는 시간을 결정하며, 이를 통해 전체 주기를 설정한다
- **영향 범위** : 타이머의 모든 시간 기반 동작(PWM 주파수, 인터럽트 주기 등)에 영향을 준다
- **공식** : $f_{CNT}=\frac{f_{TIM}}{PSC+1}$

---

## 3. CKD (Clock Division)
외부 신호를 감시하거나 정밀한 출력을 제어하기 위한 '보조 시계' 설정이다.
- **목적지** : 디지털 필터(Digital Filter) 및 데드타임 발생기(Dead-time Generator)
- **역할** : 입력 캡처 신호의 노이즈를 제거하기 위한 샘플링 클럭(t_{DTS})의 주기를 생성한다
- **영향 범위** : 카운터의 숫자 증가 속도에는 아무런 영향을 주지 않으며, 오직 필터링 성능이나 데드타임의 정밀도에만 관여한다
- **설정값** : `TIM_CKD_DIV1`, `TIM_CKD_DIV2`, `TIM_CKD_DIV4` 중 선택 가능

### 3.1 $t_{DTS}$(Dead-time and Digital Filter Sampling Clock)
타이머가 숫자를 세는 메인 시계(CK_CNT)와는 별개로, **입력 신호를 감시(필터링)하거나 출력 신호의 안전 구간(데드타임)을 계산**하기 위해 존재한다

#### 3.1.1 $t_{DTS}$의 생성 원리
타이머로 들어오는 내부 클럭($f_{TIM}$ 또는 CK_INT)을 `TIMx_CR1` 레지스터의 **CKD(Clock Division)** 비트 설정에 따라 나누어 생성한다,.
- **CKD = 00 ** $\(DIV1\):t_{DTR}=t_{CK_INT}$
- **CKD = 01 ** $\(DIV2\):t_{DTR}=2 \times t_{CK_INT}$
- **CKD = 10 ** $\(DIV4\):t_{DTR}=4 \times t_{CK_INT}$

### 3.2 디지털 필터(Digital Filter)
입력 캡처(Input Capture) 경로에 위치하며, 외부에서 들어오는 신호의 **노이즈를 제거**하는 역할을 한다
- 동작 원리 : 신호가 변했을 때(예: Low → High), 하드웨어가 이를 즉시 인정하지 않고 $t_{DTS}$

---

## 4. 차이점 비교표
|구분|PSC(Prescaler)|CKD(Clock Division)|
|----|--------------|-------------------|
|타겟 레지스터|`TIMx_PSC`|`TIMx_CR1`(Bit[9:8])|
|최종 목적지|Counter(CNT)|Digital Filter / Dead-time|
|주요 용도|PWM 주파수 및 시간 지연 설정|입력 신호의 노이즈 제거(필터링)|
|동작 특징|모든 타이머 동작의 기초가 됨|입력 캡처나 모터 제어 시에만 주로 사용됨|
