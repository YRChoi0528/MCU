#include "stm32f10x_lib.h"
#include "System_func.h"

int main(void){
  /* GPIO 초기화를 위한 구조체 변수 선언 */
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 시스템 초기화 */
  Init_STM32F103();

  /* 입력 데이터를 임시로 저장할 변수 */
  unsigned int input_data = 0;

  /* 주변장치 클럭 활성화: 출력(GPIOA)과 입력(GPIOB) 포트 모두에 클럭 공급 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);  

  /* GPIOA 0번 핀 설정 (출력용): LED 연결 목적 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOB 0번 핀 설정 (입력용): 버튼 연결 목적 */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Floating: 외부 풀업/다운 저항을 사용하는 입력
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  while(1){
    /* GPIOB 포트 전체의 입력 상태를 읽어 변수에 저장 */
    input_data = GPIO_ReadInputData(GPIOB);
    
    /* 읽어온 데이터를 그대로 GPIOA 포트에 출력 */
    /* 버튼이 눌려 HIGH가 입력되면 LED도 HIGH가 되어 켜짐 */
    GPIO_Write(GPIOA, input_data);
  }
}
