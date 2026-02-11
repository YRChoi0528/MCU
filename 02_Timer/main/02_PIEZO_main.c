#include "stm32f10x_lib.h"
#include "System_func.h"

/*
 * 이 예제는 "부저(피에조)"를 TIM3 PWM으로 구동한다.
 * 핵심:
   - PWM 주파수는 ARR(주기)로 결정된다.
   - 듀티는 CCR1로 결정된다(보통 50%로 설정해 기본 사각파 생성, 듀티비에 의해서 음색이 바뀜).
   - 스위치 입력(GPIOC 하위 8비트)을 읽어서 도레미 주파수로 매핑한 뒤,
     TIM3의 ARR/CCR1을 갱신하여 출력 주파수를 바꾼다.
 */

TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
TIM_OCInitTypeDef  TIM_OCInitStructure;
GPIO_InitTypeDef GPIO_InitStructure;

unsigned int key2DoReMi(unsigned int key);
void Change_FREQ(unsigned int freq);
void STOP_FREQ();

int main(void){

  Init_STM32F103();

  u16 key_vlaue = 0;
  u16 freq_value = 0;

  /* TIM3 클럭 활성화(APB1) */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOA(출력 핀), GPIOC(키 입력) 클럭 활성화(APB2) */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

  /*
   * GPIOC 입력 설정: 하위 8비트(0x00FF)를 입력으로 사용
   * 입력 방식: Floating(외부 회로(Switch Module-Button)에 의해 High/Low가 결정된다고 가정)
   */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = 0x00FF;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /*
   * [TIM3 기본 타임베이스 설정]
   * Prescaler = 720-1
     → 72MHz/720 = 100kHz (10μs)
   * Period(ARR)은 일단 0으로 초기화(실제 음 출력 시 Change_FREQ에서 재설정)
   */
  TIM_TimeBaseInitStruct.TIM_Period = 0;                    // ARR(초기값)
  TIM_TimeBaseInitStruct.TIM_Prescaler = 720 - 1;           // PSC
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

  /*
   * [TIM3 CH1 PWM 설정]
   * PWM1 모드 + CH1 사용
   * Pulse(CCR1)는 초기 0(실제 음 출력 시 Change_FREQ에서 갱신)
   */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInit(TIM3, &TIM_OCInitStructure);

  /* preload 사용(ARR/CCR 반영을 update 이벤트 기준으로 안정화) */
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(TIM3, ENABLE);

  while (1){
    /*
     * 기본 상태: 소리를 끔(타이머 정지 + 출력핀을 일반 입력처럼 처리)
     * 키가 눌리면(입력이 들어오면) 키값을 읽고, 주파수로 변환하여 PWM 출력
     * 키가 떼어질 때까지 유지 후 다시 정지
     */
    STOP_FREQ();

    /* 키 입력이 들어올 때까지 대기 */
    while((GPIO_ReadInputData(GPIOC) & 0x00FF) == 0);

    /* 입력 포트의 하위 8비트를 읽어 어떤 키가 눌렸는지 확인 */
    key_vlaue = GPIO_ReadInputData(GPIOC) & 0x00FF;

    /* 키값을 도레미 주파수로 변환 */
    freq_value = key2DoReMi(key_vlaue);

    /* 해당 주파수로 PWM 설정(ARR/CCR 변경 후 타이머 구동) */
    Change_FREQ(freq_value);

    /* 키가 떼어질 때까지 대기 */
    while((GPIO_ReadInputData(GPIOC) & 0x00FF) == 1);
  }
}

/*
 * 버저 출력을 확실히 끄는 처리
   1. TIM3 정지
   2. PWM 출력 핀(PA6)을 AF 출력이 아니라 입력(Floating)으로 바꿈
     → 타이머 신호가 핀으로 나가지 않게 차단
*/
void STOP_FREQ(){
  TIM_Cmd(TIM3, DISABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   // AF 해제
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
 * 원하는 주파수(freq)에 맞게 TIM3의 ARR/CCR1을 갱신하여 사각파 출력
 * 계산:
   - tmp = 100000 / freq
   - 이는 TIM3의 속도(100kHz)를 기준으로 한 주기 카운트(ARR 값)로 해석 가능
     → CNT가 0에서 ARR 까지는 가는 시간이 한 주기 이다.
     → 1 주기 시간 = (ARR+1)(PSC+1)/72MHz = (ARR+1)/100000
     → 1 주기 시간은 다음과 같이도 나타난다. 
     → 1 주기 시간 = 1/freq
     → 즉, ARR+1 = 100000 / freq 이다(unsigned int로 형 변환하여 손실되는 값이 있어 원래 freq로 복원하는데 오차를 최소화하기 위해 -1은 하지 않음).
   - CCR1 = tmp/2 로 설정하여 듀티 50% 사각파 생성
*/
void Change_FREQ(unsigned int freq){

  /* 100kHz 기준으로 ARR 값을 계산 */
  u16 tmp = (unsigned int)(100000 / freq);

  /*
   * PWM 출력 핀(PA6)을 Alternate Function Push-Pull로 설정
   * 타이머 채널 출력이 실제 핀으로 연결되게 함
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ARR(주기) 설정 */
  TIM_SetAutoreload(TIM3, tmp);

  /* CCR1(듀티) 설정: 50% */
  TIM_SetCompare1(TIM3, (unsigned int)(tmp / 2));
}

/*
 * key2DoReMi(key)
 * 스위치 입력(One-Hot 형태: 0x01,0x02,0x04...)을 대응하는 주파수로 변환
 * 반환값: Hz 단위의 음계 주파수(예: 1047, 1175, ...)
 */
unsigned int key2DoReMi(unsigned int key){

  unsigned int _ret = 0;

  switch(key){
    case 0x01: _ret = 1047; break;
    case 0x02: _ret = 1175; break;
    case 0x04: _ret = 1319; break;
    case 0x08: _ret = 1397; break;
    case 0x10: _ret = 1568; break;
    case 0x20: _ret = 1760; break;
    case 0x40: _ret = 1976; break;
    case 0x80: _ret = 2093; break;
  }
  return _ret;
}
