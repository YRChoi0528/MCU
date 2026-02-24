#include "stm32f10x_lib.h"
#include"system_func.h"

void putch(unsigned char c){
  USART_SendData(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void puts(u8 *s){
  while (*s != '\0'){
    putch(*s);
    s ++;
  }
}

unsigned char getch(){
  unsigned char key = 0;
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
  key = USART_ReceiveData(USART1);
  return key;
}

int main(void){
  Init_STM32F103();

  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_Clock = USART_Clock_Disable;
  USART_InitStructure.USART_CPOL = USART_CPOL_Low;
  USART_InitStructure.USART_CPHA = USART_CPHA_2Edge;
  USART_InitStructure.USART_LastBit = USART_LastBit_Disable;
  
  USART_Init(USART1, &USART_InitStructure);  

  USART_Cmd(USART1, ENABLE);
  
  while (1){
    /* 
     * 00_uart_main.c와 다르게 메인 루프가 비어있다. (no work)
     * 폴링 방식에서는 putch(getch()) 때문에 CPU가 계속 갇혀 있었지만,
     * 이제 CPU는 통신을 신경 쓰지 않고 다른 작업(LED 제어, 센서 읽기 등)을 할 수 있다.
     * 통신 데이터가 들어오는 순간에만 ISR로 잠시 이동하여 처리한다.
     * 어떻게 이게 가능한지는 00_uart_interrupt.c 에서 설명하겠다.
     */
  }
}
