# Timer - PSC와 CKD 비교 (블록도 기준 설명)

![General_Purpose_Timer_Block_Diagram](../../images/General_Purpose_Timer_Block_Diagram.png)

## 1. 목적
해당 문서는 첨부된 **General Purpose Timer Block Diagram**에 표시된 내용만을 근거로 </br> PSC와 CKD의 구조적 차이를 정리한다.

추가적인 내부 클럭 설명이나 데이터시트 확장 개념은 포함하지 않는다.
---

## 2. 블록도에서 확인 가능한 클럭
블록도에 표시된 클럭 관련 신호는 다음과 같다.

|용어|의미|
|----|----|
|TIMxCLK|RCC에서 타이머로 공급되는 클럭|
|CK_PSC|PSC 입력 클럭|
|PSC|분주기 레지스터|
|CK_CNT|PSC를 거친 후 CNT로 가는 클럭|
|CNT|카운터 레지스터|
|Internal Clock(CK_INT)||

---

## 3. PSC의 위치와 역할
블록도에서 PSC는 다음 경로에 위치한다.
```
TIMxCLK → CK_PSC → PSC → CK_CNT → CNT
```
그림에서 확인 가능한 사실:
  1. PSC는 TIMxCLK 뒤에 위치한다.
  2. PSC의 출력은 CK_CNT이다.
  3. CK_CNT는 직접 CNT로 연결된다.
따라서 그림만 기준으로 보면:
> PSC는 카운터(CNT)로 들어가는 클럭을 분주한다.
> 즉,
> > PSC 설정은 CNT 증가 속도에 직접 영향을 준다.
> > ARR, CRR과 연결된 주기/비교 동작에 영향을 준다.
