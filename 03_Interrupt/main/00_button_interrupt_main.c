#include "stm32f10x_lib.h"
#include "System_func.h"

/*
 * 전역 변수 설정
 * LED_data: 현재 GPIOC로 출력할 LED 데이터
 * vu16은 volatile unsigned 16-bit 정수형이며, Library → inc → stm32f10x_type.h 에서 확인 가능하다.
 * volatile은 컴파일러가 해당 변수를 최적화로 제거하거나
 * 레지스터에만 보관하지 않도록 하기 위해 사용한다.
 * 즉, 외부 요인(인터럽트 등)에 의해 값이 바뀔 수 있음을 의미한다.
 */
vu16 LED_data=0x0000; 

void INT_init();

int main(void){
  GPIO_InitTypeDef GPIO_InitStructure;

  Init_STM32F103();

  /*
   * 클럭 활성화 (APB2)
   * GPIOB: 스위치 입력용
   * GPIOC: LED 출력용
   * AFIO : EXTI 라인과 GPIO 포트 연결 설정용
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | 
                         RCC_APB2Periph_AFIO, ENABLE);

  /*
   * GPIOC 하위 8비트(PC0~PC7)를 LED 출력용 Push-Pull 출력으로 설정
   * 0xFF는 PC0~PC7에 해당하는 비트 마스크이다.
   */
  GPIO_InitStructure.GPIO_Pin = 0xFF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /*
   * GPIOB의 PB0, PB1, PB2, PB5, PB6, PB7, PB8, PB9를
   * 스위치 입력용 Floating 입력으로 설정
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_5|
                                GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  INT_init();
  
  while (1){
    /*
     * 현재 LED_data 값을 그대로 GPIOC 출력 레지스터에 기록
     * LED 패턴은 인터럽트 서비스 루틴에서 변경된다고 가정
     */
    GPIO_Write(GPIOC, LED_data);
  }//end while
}//end main

/* =============== Function =============== */

/*
 * 외부 인터럽트 초기화 함수
 * 대상 입력 핀:
 * - EXTI0   → PB0
 * - EXTI1   → PB1
 * - EXTI2   → PB2
 * - EXTI5~9 → PB5, PB6, PB7, PB8, PB9
 * Rising Edge에서 인터럽트가 발생하도록 설정한다.
 */
void INT_init(){
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /*
   * NVIC 설정
   * EXTI0, EXTI1, EXTI2, EXTI9_5 인터럽트 채널 활성화
   * 숫자가 작을수록 더 높은 우선순위를 가진다.
   */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /*
   * AFIO 설정
   * GPIOB의 각 핀을 대응되는 EXTI 라인에 연결
   */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource2);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

  /*
   * EXTI 설정
   * EXTI Line 0, 1, 2, 5, 6, 7, 8, 9를 인터럽트 모드로 사용
   * 버튼 입력이 High가 되는 순간(Rising Edge) 인터럽트 발생
   */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line5|
                                 EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}
