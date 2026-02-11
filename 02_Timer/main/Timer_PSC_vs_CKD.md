# Timer - PSC와 CKD의 차이 및 사용 이유

## 1. 개요
STM32 Timer에는 두 가지 분주기가 존재한다.
- **PSC (Prescaler)**
- **CKD (Clock Division, CR1의 CKD[1:0])**

두 설정 모두 "클럭을 나눈다"는 공통점이 있지만, **분주 대상화 목적이 완전히 다르다.**
해당 문서는 블록도 기준으로 PSC와 CKD의 구조적 차이와 실제 실습/설계에서 PSC만 주로 사용하는 이유를 정리한다.

---

## 2. 블록도 기준 구조적 차이

![General_Purpose_Timer_Block_Diagram](../images/General_Purpose_Timer_Block_Diagram.png)

### 2.1 PSC의 위치
```
CK_PSC → PSC → CK_CNT → CNT 
```
`CK_PSC`는 "아직 분주되지 않은 타이머 클럭"이다.
PSC는 **입력된 타이머 클럭(CK_PSC)**을 분주한 후 **카운터(CNT)**로 들어가는 **클럭(CK_CNT)**을 만든다.
> 즉, PSC는 카운터 속도를 직접 결정한다.

### 2.2 CKD의 위치
CKD는 CR1 레지스터의 CKD[1:0] 비트이며, <\br> 카운터가 아니라 내부 필터/샘플링 회로로 들어가는 클럭을 분주한다.
---

## 3. 기능적 차이



---

## 4. PSC의 역할

## 5. CKD의 역할

## 6. 왜 대부분 PSC만 설정하는가?

## 7. 실습에서의 일반적인 사용
