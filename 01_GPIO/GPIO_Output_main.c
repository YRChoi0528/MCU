#include "stm32f10x_lib.h"
#include "System_func.h"

/*
- 소프트웨어 딜레이 함수
- count: 반복 횟수 (u32 타입으로 약 42억까지 가능)
*/
void delay(u32 count){
  while(count--);
}

int main(void){
  /* GPIO 초기화를 위한 구조체 변수 선언 */
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 시스템 초기화: 외부 8MHz HSE를 PLL 9배 체배하여 72MHz 시스템 클럭(SYSCLK) 생성
     Flash Latency(72MHz) 및 버스(APB1(36MHz), APB2(72MHz)) 분주비 설정을 포함함 */
  Init_STM32F103();

  /* LED 상태를 저장하는 변수 (0x0080: 7번 핀 활성화 상태로 시작) */
  unsigned int LED_data = 0x0080;
  
  /* 주변장치 클럭 활성화: GPIOA 레지스터 조작을 위해 APB2 버스의 클럭을 공급
     이 과정이 생략되면 이후 GPIO 설정은 하드웨어에 반영되지 않음 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  /* 
  - GPIOA 포트 설정 (0~7번 핀) 
  - 50MHz 속도
  - Push-Pull 출력 모드 → 해당 모드 덕에 LED에 0V 와 3.3V를 
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3| 
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  
  /* 설정된 구조체 값을 바탕으로 실제 CRL/CRH 레지스터 비트(CNF, MODE)를 수정 */
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  while(1){
    /* 이전 위치의 LED를 끔 (0 출력) */
    GPIO_ResetBits(GPIOA, LED_data);
  
    /* LED 위치 이동 로직 */
    if(LED_data == 0x0080)
      LED_data = 0x0001; // 7번 핀까지 갔으면 다시 0번 핀으로 복귀
    else
      LED_data <<= 1;   // 왼쪽으로 한 칸 시프트 (핀 위치 이동)

    /* 새로운 위치의 LED를 켬 (1 출력) */
    GPIO_SetBits(GPIOA, LED_data);
    
    /* 사람이 인지할 수 있도록 일정한 딜레이 부여 */
    delay(0xAFFFF);
  }
}
