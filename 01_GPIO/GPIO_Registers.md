# GPIO_Registers.md

## 요약
해당 문서는 STM32F103의 GPIO 레지스터 구조를 정리하고, 강의 자료 및 `gpio.c / gpio.h / main.c` 코드가 **어떤 레지스터를 어떻게 제어하는지**를 연결해 설명한다.

---

## 1. GPIO 레지스터와 포트 구조
STM32F103에서 각 GPIO 포트(GPIOA, GPIOB, ...)는 동일한 **레지스터 레이아웃**을 가진다.
- 각 포트는 독립적인 레지스터 세트를 가짐
- PA0과 PB0은 번호는 같아도 **완전히 다른 레지스터**로 제어됨
주요 레지스터는 다음과 같다.

|레지스터|역할|
|--------|----|
|GPIOx_CRL|핀 0~7 설정|
|GPIOx_CRH|핀 8~15 설정|
|GPIOx_IDR|입력 데이터|
|GPIOx_ODR|출력 데이터|
|GPIOx_BSRR|비트 Set/Reset|
|GPIOx_BRR|비트 Reset|
|GPIOx_LCKR|설정 잠금|

---

## 2. GPIO 클럭과 RCC의 관계

---

## 3. CRL / CRH : GPIO 모드 설정 레지스터

---

## 4. gpio.h : 레지스터 설정을 추상화한 열거형

---

## 5. GPIO_Init() 함수와 레지스터 동작

---

## 6. ODR, BSRR, BRR : 출력 제어 레지스터

---

## 7. IDR : 입력 데이터 레지스터

---

## 8. LCKR : 설정 잠금 레지스터

---

## 9. main.c와 레지스터 흐름 연결
