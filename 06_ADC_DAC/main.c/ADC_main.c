#include "stm32f10x_lib.h"
#include "System_func.h"

/* 함수 선언 */
void Timer_Delay_init(void);
void delay_ms(u16 time);

int main(void){

  Init_STM32F103(); 
  
  u8 LED_num = 0;   
  u8 LED_data = 0x00;
  
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  Timer_Delay_init();
  
  /* ADC1, GPIOA(ADC 입력), GPIOC(LED 출력) 클럭 활성화 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

  /* PC0~PC7 핀 초기화: LED 출력용 Push-Pull 설정 */
  GPIO_InitStructure.GPIO_Pin = 0xFF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* PA0 핀 초기화: ADC 채널 0 아날로그 입력 설정 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* 
   * STM32F10x 규격 상 ADC 최대 클럭은 14MHz이다.
   * 시스템 클럭(PCLK2) 72MHz 기준, 기존 Div2(36MHz)는 심각한 오버클럭이다.
   * 이를 Div6으로 수정하여 12MHz의 안정적인 동작 클럭을 확보한다.
   */
  RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
  
  /* ADC1 기본 모드 설정 */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  /* 독립 모드 (Dual 모드 미사용) */
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       /* 단일 채널 변환 */
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 /* 단일 변환 모드 (연속 변환 비활성화) */
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* 소프트웨어 트리거 사용 */
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              /* 우측 정렬 */
  ADC_InitStructure.ADC_NbrOfChannel = 1;                             /* 변환 채널 수 1개 */
  ADC_Init(ADC1, &ADC_InitStructure);
     
  /*
   * 12MHz 클럭 속도에 55.5 Cycles 설정 시 샘플링 타임은 
   * 1/12,000,000 * (55.5 + 12.5) ≒ 약 5.67μs가 된다.
   */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

  /* ADC1 활성화 */
  ADC_Cmd(ADC1, ENABLE);
 
  /* ADC 내부 캘리브레이션 (오차 보정) 수행 */
  ADC_ResetCalibration(ADC1);
  while(ADC_GetResetCalibrationStatus(ADC1));        
  ADC_StartCalibration(ADC1);
  while(ADC_GetCalibrationStatus(ADC1)); 
  
  while (1){       
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);         /* 소프트웨어 트리거로 ADC 변환 개시 */
    
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));  /* EOC(End of Conversion) 플래그 대기 (변환 완료 확인) */
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);              /* 플래그 초기화 */
    
    /* 
     * ADC 변환 데이터 취득 (12bit 분해능: 0 ~ 4095)
     * 500 단위로 나누어 0~8 단계의 레벨 지표(LED_num) 생성
     */
    LED_num = ADC_GetConversionValue(ADC1) / 500;
    
    /* bit 누적 LED 점등 */
    if(LED_num == 0)      LED_data = 0x00; /* 0000 0000 */
    else if(LED_num == 1) LED_data = 0x01; /* 0000 0001 */
    else if(LED_num == 2) LED_data = 0x03; /* 0000 0011 */
    else if(LED_num == 3) LED_data = 0x07; /* 0000 0111 */
    else if(LED_num == 4) LED_data = 0x0F; /* 0000 1111 */
    else if(LED_num == 5) LED_data = 0x1F; /* 0001 1111 */
    else if(LED_num == 6) LED_data = 0x3F; /* 0011 1111 */
    else if(LED_num == 7) LED_data = 0x7F; /* 0111 1111 */
    else                  LED_data = 0xFF; /* 1111 1111 */
    
    /* LED 포트 출력 갱신 */
    GPIO_ResetBits(GPIOC, 0xFF);
    GPIO_SetBits(GPIOC, LED_data);
    
    delay_ms(500);
  }
}

void Timer_Delay_init(){
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 
  TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1;  
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStruct.TIM_Period = 10 - 1;  
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
}

void delay_ms(u16 time){
  TIM_Cmd(TIM3, ENABLE);

  while(--time){
     while(TIM_GetFlagStatus(TIM3, TIM_IT_Update) == RESET);
     TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  }
  TIM_Cmd(TIM3, DISABLE);
}
