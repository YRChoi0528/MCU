#include "stm32f10x_lib.h"
#include "System_func.h"

/* TIM2를 이용한 폴링 딜레이 함수 선언 (구현은 main 아래를 확인) */
void delay_ms(u16 time);

int main(void){

  /* 타이머 기본 설정 구조체(PSC/ARR/CR1 등) */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  /* PWM(OC) 설정 구조체(CCMR/CCER/CCR 등) */
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  /* PWM 출력 핀 설정용 GPIO 구조체 */
  GPIO_InitTypeDef GPIO_InitStructure;

  Init_STM32F103();

  /* 듀티(비교값) 초기값: CCR1에 들어갈 값 */
  u16 CCR1_Val = 10;

  /* TIM2, TIM3 클럭 활성화(APB1) */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

  /*
   * PWM 출력 핀 설정
   * PA6은 TIM3의 채널 1과 연결되어 있기 때문에 사용
   * Timer PWM은 "Alternate Function Output"이어야 타이머 내부 신호가 핀으로 나감
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      // AF Push-Pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*
   * [TIM2 설정] delay_ms용 1ms 타임베이스 생성
   * PSC=7200-1 → 10kHz(0.1ms)
   * ARR=10-1   → 1ms Update
   */
  TIM_TimeBaseInitStruct.TIM_Period = 10 - 1;          // ARR
  TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1;     // PSC
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

  /*
   * [TIM3 설정] PWM 주기 설정
   * PSC=0: 타이머 입력클럭(기본) 그대로 사용
   * ARR=999: PWM 주기 결정(0~999 카운트)
   * 실제 PWM 주파수는 타이머 클럭과 ARR/PSC에 의해 결정됨
   */
  TIM_TimeBaseInitStruct.TIM_Period = 999;             // ARR (PWM 주기)
  TIM_TimeBaseInitStruct.TIM_Prescaler = 0;            // PSC (분주 없음)
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

  /*
   * [TIM3 채널1 PWM 설정]
   * PWM1 모드: CNT < CCR1 구간에서 Active(일반적으로 High), 이후 Inactive
   * Channel_1: CCR1/CCMR1/CCER의 CH1 필드를 사용
   * Pulse: CCR1 값(듀티 결정)
   * Polarity High: 비반전(Active High)
   */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;            // CCR1에 기록될 값
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInit(TIM3, &TIM_OCInitStructure);

  /*
   * Preload 활성화
   * TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); → TIM3.CCMR1 의 OC1PE(Output Compare 1 Preload Enable) 비트 1로 설정 → CCR 값 변경이 다음 업데이트 이벤트 때 반영
   * TIM_ARRPreloadConfig(TIM3, ENABLE); → TIM3.CR1 의 ARPE(Auto-Reload Preload Enable) 비트 1로 설정 → ARR 변경 시 다음 업데이트 이벤트 때 반영
   * Update 이벤트 시점에 shadow -> active로 반영되어 글리치 감소
     (강의자료에서도 preload/ARPE 사용을 요구하는 내용이 있음) :contentReference[oaicite:0]{index=0}
   */
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); 
  TIM_ARRPreloadConfig(TIM3, ENABLE);

  /* TIM3 시작: PWM 출력이 동작하기 시작 */
  TIM_Cmd(TIM3, ENABLE);

  while (1){
    /*
     * 1. Preload 매커니즘 적용 (OC1PE=1, ARPE=1)
       - Preload 활성화를 통해 CCR1 값을 변경하더라도 즉시 반영되지 않고 Update Event 시점에 동기화됨
       - 따라서 듀티 변경 시 타이머를 정지(TIM_Cmd DISABLE/ENABLE)할 필요가 없음
     * 2. 시간 파라미터 계산 (System Clock = 72MHz 기준)
       - 카운팅 속도는 분주 없이(PSC = 0) 72MHz(13.89ns) 마다 CNT 증가
       - 전체 주기(Period)는 ARR = 999 설정으로 (ARR+1)개 클럭 소요
         → 1000/72MHz ≒ 13.89μs
     * 3. PWM 출력 타이밍 
       (1) CCR1 = 10 가정, PWM Mode 1
         - High 구간 (Active): 10/7200MHz ≒ 0.139μs (PA6핀 3.3V 출력)
         - Low 구간 (Inactive): 13.89μs - 0.139μs ≒ 13.751μs (PA6핀 0V 출력)
       (2) CCR1 = 500 가정, PWM Mode 1
         - High 구간 (Active): 500/7200MHz ≒ 6.94μs (PA6핀 3.3V 출력)
         - Low 구간 (Inactive): 13.89μs - 6.94μs ≒ 6.95μs (PA6핀 0V 출력)
     * CCR1 값의 변경으로 PA6핀과 연결된 LED는 깜박이는 속도가 달라진다. 
     * 하지만 인간의 눈으로는 위와 같은 속도의 깜빡임은 인지하지 못하고 평균적인 밝기만 인지하게 되어서 CCR1=10 일 때는 LED가 어둡고, CCR1=500 일 때는 LED가 밝게 보인다.
     */
    // TIM_Cmd(TIM3, DISABLE);

    /* CCR1 값 변경(듀티 변경) */
    TIM_SetCompare1(TIM3, CCR1_Val);

    // TIM_Cmd(TIM3, ENABLE);

    /* 듀티를 점진적으로 증가시켜 밝기 변화(LED가 PWM에 연결되어 있다고 가정) */
    CCR1_Val += 5;

    /* ARR=999이므로 CCR1은 0~999 범위에서 의미가 있음 */
    if(CCR1_Val >= 999){
      CCR1_Val = 0;
    }

    /* TIM2 기반 10ms 지연 */
    delay_ms(10);
  }
}

/*
 * TIM2 Update(1ms)를 time 번 대기하여 ms 딜레이 구현
 * TIM2 설정은 main에서 ARR=10-1, PSC=7200-1 기반으로 1ms Update 발생
 */
void delay_ms(u16 time){
  u16 i;

  TIM_Cmd(TIM2, ENABLE);
  for(i = 0; i < time; i++){
    while(TIM_GetFlagStatus(TIM2, TIM_IT_Update) == RESET);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
  }
  TIM_Cmd(TIM2, DISABLE);
}
