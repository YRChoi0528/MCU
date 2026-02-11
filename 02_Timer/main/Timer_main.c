#include "stm32f10x_lib.h"
#include "System_func.h"

/*
 * Timer_init()
 * 목적: TIM3를 "고정된 시간 단위(여기서는 약 1ms)"를 만들어주는 타이머로 설정
 * 핵심 설정 값:
   - PSC : 7200-1  → 72MHz / 7200 = 10kHz (0.1ms)
   - ARR :   10-1    → 0.1ms * 10 = 1ms마다 Update(오버플로우) 발생
 * 즉, TIM3는 "Update Event가 1ms마다 발생하는 타이머"가 된다.
 */
void Timer_init(){  
  /* 타이머 기본 설정을 위한 구조체 */
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  /* TIM3 주변장치 클럭 활성화(APB1) */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* 
   * PSC 레지스터에 들어갈 값
     - -1 을 하는 이유는 컴퓨터는 분주비를 0으로 설정하는 것을 1로 보기 때문이다.
     - 분주비 = (PSC + 1)
     - 72MHz / 7200 = 10kHz → 1 tick = 0.1ms 
   */
  TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1;

  /* 
   * CR1의 카운트 방향/모드 설정
     - UpCount: CNT가 0 -> ARR까지 증가 
   */
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

  /* 
   * ARR에 들어갈 값
     - ARR=9이면 0~9까지 10번 카운트 후 Update 발생
     - 0.1ms * 10 = 1ms 
   */
  TIM_TimeBaseInitStruct.TIM_Period = 10 - 1;

  /* 
   * CR1의 CKD 설정(클럭 분주)
     - DIV1: 내부 클럭을 추가 분주 없이 사용
   */
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

  /* 위 구조체 값들을 실제 TIM3의 PSC/ARR/CR1에 반영 */
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
}

/*
 * delay_ms(time)
 * 목적: TIM3 Update Event를 매개변수(time)만큼 기다려서 ms 단위 딜레이 구현
 * 동작 개요:
   1) TIM3 카운터 시작(CR1.CEN=1)
   2) Update Interrupt Flag(UIF)가 SET될 때까지 대기 → Up-count 모드에서는 CNT가 ARR에 도달(오버플로우)할 때 이벤트 발생하요 SET 됨.
   3) UIF를 Clear하고 다음 1ms를 기다림
   4) 총 time 번 반복 후 타이머 정지
 * 인터럽트가 아니라 "폴링(polling)" 방식으로 플래그를 기다린다.
 */
void delay_ms(u16 time){
  u16 i;

  /* 타이머 시작: CR1.CEN = 1 */
  TIM_Cmd(TIM3, ENABLE);

  for(i = 0; i < time; i++){
    /* 
     * Update 이벤트 발생 대기
       - 내부적으로 SR(UIF)를 확인하는 로직 
       - CNT가 ARR에 도달(1ms)할 때 TIM_GetFlagStatus(TIM3, TIM_IT_Update) = SET 되므로 while 탈출
     */
    while(TIM_GetFlagStatus(TIM3, TIM_IT_Update) == RESET);

    /* Update 플래그 Clear (다음 주기를 정상적으로 기다리기 위함) */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  }

  /* 타이머 정지: CR1.CEN = 0 */
  TIM_Cmd(TIM3, DISABLE);
}

int main(void){
  /* GPIO 초기화를 위한 구조체 */
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 시스템 초기화(클럭/Flash/버스 분주 등) */
  Init_STM32F103();

  /* 
   * GPIOA, GPIOC 클럭 활성화(APB2)
     - FND(GPIOA) + 자리 선택(GPIOC)
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

  /* 0~9 숫자 표시용 7-seg(또는 FND) 데이터 테이블 */
  unsigned char FND_DATA_TBL[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7C,0x07,0x7F,0x67};

  /* 시간 카운터(표시용 자리수) */
  unsigned char time_4ms=0, time_100ms=0, time_1s=0, time_10s=0, time_1m=0;

  /*
   * PA0~7: FND 세그먼트 데이터 출력
   * Push-Pull 출력 / 50MHz
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3|
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*
   * PC0~3: FND 자리 선택
   * Push-Pull 출력 / 50MHz
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* TIM3를 1ms 딜레이 타이머로 초기화 */
  Timer_init();

  while(1){
    /*
     * 아래 4블록은 Array FND 동작
     * 각 자리를 짧게(1ms) 켰다가 끄는 것을 빠르게 반복하면
       사람이 보기엔 4자리가 동시에 켜진 것처럼 보인다.
     * Array FND는 일반 FND 4개를 사용하는 것보다 적은 핀을 사용할 수 있는 장점이 있지만 4개의 FND들을 동시에 키는 것은 불가능하다.
     * 실습에서 사용하는 Array FND의 자리를 활성화 하기 위해서는 해당 자리와 연결된 핀을 Low(0) 해준다.
     */

    /* [가장 오른쪽 자리] 선택 예시: PC3만 활성화(Active Low) */
    GPIO_SetBits(GPIOC, 0x0F);          // 모든 자리 OFF(또는 비활성)
    GPIO_ResetBits(GPIOC, GPIO_Pin_3);  // 특정 자리 ON
    GPIO_ResetBits(GPIOA, 0xFF);        // 세그먼트 데이터 클리어
    GPIO_SetBits(GPIOA, FND_DATA_TBL[time_100ms]); // 숫자 출력
    delay_ms(1);                        // 1ms 유지

    /* 다음 자리: PC2 */
    GPIO_SetBits(GPIOC, 0x0F);
    GPIO_ResetBits(GPIOC, GPIO_Pin_2);
    GPIO_ResetBits(GPIOA, 0xFF);
    GPIO_SetBits(GPIOA, FND_DATA_TBL[time_1s]);
    delay_ms(1);

    /* 다음 자리: PC1 */
    GPIO_SetBits(GPIOC, 0x0F);
    GPIO_ResetBits(GPIOC, GPIO_Pin_1);
    GPIO_ResetBits(GPIOA, 0xFF);
    GPIO_SetBits(GPIOA, FND_DATA_TBL[time_10s]);
    delay_ms(1);

    /* 다음 자리: PC0 */
    GPIO_SetBits(GPIOC, 0x0F);
    GPIO_ResetBits(GPIOC, GPIO_Pin_0);
    GPIO_ResetBits(GPIOA, 0xFF);
    GPIO_SetBits(GPIOA, FND_DATA_TBL[time_1m]);
    delay_ms(1);

    /* 4ms 단위로 시간 카운트 증가(자리 갱신용) */
    time_4ms++;
    if(time_4ms == 25){        // 4ms * 25 = 100ms
      time_100ms++;
      time_4ms = 0;
    }

    if(time_100ms == 10){      // 100ms * 10 = 1s
      time_1s++;
      time_100ms = 0;
    } else;

    if(time_1s == 10){         // 1s * 10 = 10s
      time_10s++;
      time_1s = 0;
    } else;

    if(time_10s == 6){         // 10s * 6 = 60s(1m)
      time_1m++;
      time_10s = 0;
    } else;

    if(time_1m == 10){
      time_1m = 0;
    } else;
  }
}
