#include "stm32f10x_lib.h"
#include "System_func.h"

void Timer_init(){
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
 
  TIM_TimeBaseInitStruct.TIM_Prescaler = 7200-1;  
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStruct.TIM_Period = 10-1;  
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
}

void delay_ms(u16 time){
  u16 i; 
  TIM_Cmd(TIM2,ENABLE); /* 타이머 시작 */    
  for(i=0;i<time;i++){
    while(TIM_GetFlagStatus(TIM2,TIM_IT_Update)==RESET);
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);
  }
  TIM_Cmd(TIM2,DISABLE); /* 타이머 정지 */
}

/* 
 * 전역 변수 설정
 * stop: LED 이동 정지/재개 플래그 (0: 이동, 1: 정지)
 * dir: LED 이동 방향 플래그 (0: 왼쪽 시프트, 1: 오른쪽 시프트)
 */
char stop=0, dir=0;

int main(void){
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* 초기 점등할 LED 위치 (0x0080 = 최상위 비트(PC7) ) */
  unsigned int LED_data = 0x0080;

  Init_STM32F103();
  Timer_init();
  
  /* 
   * 클럭 활성화 (APB2)
   * GPIOB: 버튼 입력용
   * GPIOC: LED 출력용
   * AFIO: 외부 인터럽트(EXTI) 라인과 포트 연결 설정용 (필수)
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
                         | RCC_APB2Periph_AFIO, ENABLE);
  
  /* PC0~PC7 (LED 연결 핀) Push-Pull 출력 설정 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* PB0~PB2 (버튼 연결 핀) Floating 입력 설정 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /*
   * NVIC(중첩 인터럽트 제어기) 설정
   * EXTI0, EXTI1, EXTI2 인터럽트 채널 활성화 및 우선순위 지정
   * PreemptionPriority가 낮을수록 우선순위가 높음 (0이 가장 먼저 실행됨)
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
  
  /*
   * AFIO 설정: GPIOB의 0, 1, 2번 핀을 각각 EXTI Line 0, 1, 2에 연결
   */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource2);
  
  /*
   * EXTI(외부 인터럽트) 모드 및 트리거 설정
   * 버튼이 눌릴 때(Rising Edge) 인터럽트가 발생하도록 설정
   */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  while (1){
    /* stop 상태가 0(진행)일 때만 LED 시프트 동작 수행 */
    if(stop == 0){
      if(dir==0){
        /* dir == 0: 왼쪽으로 이동. 최하위 비트(0x01)면 다시 최상위(0x80)로 순환 */
        if(LED_data==0x80)
          LED_data = 0x01;
        else
          LED_data <<= 1;
      }
      else{
        /* dir == 1: 오른쪽으로 이동. 최상위 비트(0x80)면 다시 최하위(0x01)로 순환 */
        if(LED_data==0x01)
          LED_data = 0x80;
        else
          LED_data >>= 1;
      }
      
      /* 이전 LED 상태 지우고 새로운 패턴 출력 후 125ms 대기 */
      GPIO_ResetBits(GPIOC, 0xFF);
      GPIO_SetBits(GPIOC, LED_data);
      delay_ms(125);
    }
    else{
      /* stop == 1 이면 아무 동작 없이 무한 대기 (LED 멈춤 효과) */
    }
  }//end while
}//end main
