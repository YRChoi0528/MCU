#include "stm32f10x_lib.h"
#include "System_func.h"

void Timer_init(){  
  TIM_TimeBaseInitTypeDef TIM2_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
 
  TIM2_TimeBaseInitStruct.TIM_Prescaler = 7200-1;  
  TIM2_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM2_TimeBaseInitStruct.TIM_Period = 10000-1;  
  TIM2_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM2,&TIM2_TimeBaseInitStruct);
}

/* FND에 출력할 숫자를 제어하는 전역 카운터 변수 (0~9) */
u8 time_1s = 0;

int main(void){

  NVIC_InitTypeDef NVIC_InitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;

  Init_STM32F103();   
  Timer_init();
  
  /* 0~9까지 숫자를 FND에 띄우기 위한 세그먼트 데이터 테이블 */
  unsigned char FND_DATA_TBL[]={0x3F,0X06,0X5B,0X4F,0X66,0X6D,0X7C,0X07,0X7F,0X67};
   
  /* GPIOA 클럭 활성화 (FND 데이터 출력용) */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  /* PA0~PA7 (FND 연결 핀) Push-Pull 출력 설정 */
  GPIO_InitStructure.GPIO_Pin = 0x00FF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /*
   * NVIC 설정
   * TIM2 인터럽트 채널(IRQChannel)을 활성화
   */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* 
   * TIM2 Update 인터럽트 활성화 및 타이머 시작
   * 이 시점부터 하드웨어 타이머가 백그라운드에서 카운팅을 시작하며, 1초마다 ISR을 호출함
   * TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); → TIM2 DIER의 UIE(Update Interrupt Enable) bit SET
   */
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM2,ENABLE);
  
  while (1){
    /*
     * ISR에서 1초마다 time_1s 변수값을 변경해주면,
     * 메인 루프는 그저 변경된 값을 FND에 출력하기만 한다.
     */
    GPIO_ResetBits(GPIOA, 0xFF);
    GPIO_SetBits(GPIOA, FND_DATA_TBL[time_1s]);
  }
}
