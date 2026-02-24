#include "stm32f10x_lib.h"
#include"system_func.h"

/*
 * putch(unsigned char c)
 * 목적: 1바이트의 문자(데이터)를 USART를 통해 송신(Transmit)한다.
 * 동작 원리 (폴링 방식):
 * 1. USART_SendData()를 통해 데이터를 데이터 레지스터(DR)에 기록한다.
 * 2. 하드웨어가 데이터를 시프트 레지스터로 옮겨 전송을 시작한다.
 * 3. TXE(Transmit Data Register Empty) 플래그가 SET(1)이 될 때까지 while문에서 무한 대기한다.
 * - TXE가 1이 되었다는 것은 "버퍼가 비워졌으니 다음 데이터를 써도 좋다"는 의미이다.
 */
void putch(unsigned char c){
  USART_SendData(USART1, c);
  /* TXE 플래그가 1(SET)이 될 때까지 대기 (폴링) */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/*
 * puts(u8 *s)
 * 목적: 문자열(String)을 끝('\0', 널 문자)을 만날 때까지 연속으로 송신한다.
 */
void puts(u8 *s){
  while (*s != '\0'){
    putch(*s);
    s ++;
  }
}

/*
 * getch()
 * 목적: USART를 통해 들어오는 1바이트의 문자를 수신(Receive)하여 반환한다.
 * 동작 원리 (폴링 방식):
 * 1. RXNE(Read Data Register Not Empty) 플래그가 SET(1)이 될 때까지 while문에서 무한 대기한다.
 * - RXNE가 1이 되었다는 것은 "수신 버퍼에 새로운 데이터가 도착했다"는 의미이다.
 * 2. USART_ReceiveData()를 통해 데이터 레지스터(DR)에서 값을 읽어온다.
 * (값을 읽어오는 순간 하드웨어가 자동으로 RXNE 플래그를 0으로 클리어한다.)
 */
unsigned char getch(){
  unsigned char key = 0;
  /* RXNE 플래그가 1(SET)이 될 때까지 대기 (폴링) */
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
  
  /* 데이터가 도착하면 DR 레지스터에서 값을 읽어서 반환 */
  key = USART_ReceiveData(USART1);
  return key;
}

int main(void){
  /* 시스템 초기화 (클럭 설정 등) */
  Init_STM32F103();

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 
   * 클럭 활성화 (APB2)
   * GPIOA: TX(PA9), RX(PA10) 핀을 사용하기 위해 켬
   * USART1: 통신 모듈을 사용하기 위해 켬
   */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
  
  /*
   * 송신 핀(TX) 설정: PA9
   * - MCU 내부의 USART 하드웨어가 핀을 직접 제어해야 하므로 
   * 일반 출력이 아닌 '특수 기능(Alternate Function)'인 AF_PP로 설정한다.
   */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*
   * 수신 핀(RX) 설정: PA10
   * - 외부에서 들어오는 신호를 그대로 받아들여야 하므로
   * 아무런 저항이 안 걸린 Input Floating 상태로 설정한다.
   */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /*
   * USART1 초기화 구조체 설정 (8-N-1 비동기 통신 표준 설정)
   * - BaudRate: 9600 bps
   * - WordLength: 8비트 데이터
   * - StopBits: 1비트
   * - Parity: 없음(No Parity)
   * - FlowControl: 사용 안 함(None)
   * - Mode: 송신(Tx) 및 수신(Rx) 모두 활성화
   */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  /* 
   * 아래 4개는 동기식(Synchronous) 통신 설정이므로 비동기 UART에서는 Disable로 둔다 
   * Q. 그럼 작성 안해도 괜찮은거 아닌가? 하는 의문이 들 수 있다.
   * A. 생략하면 안된다. 우리는 main 함수 내에서 선언한 지역 변수 USART_InitStructure는 메모리의 '스택(Stack)'영역에 할당된다.
   * 이는 C 언어 특성상 자동으로 메모리 공간을 0으로 초기화해주지 않고 이전에 사용하다 남은 쓰레기값(Garbage Value)이 들어있게 된다.
   * 만약 설정을 생략하면 이 쓰레기값이 레지스터에 그대로 들어가 오동작을 유발할 수 있다.
   * 사용하지 않는 코드를 매번 적는 것이 번거롭다면, 구조체 세팅을 시작하기 전에 USART_StructInit() 함수를 사용하여 기본값으로 초기화하는 것을 권장한다.
   */
  USART_InitStructure.USART_Clock = USART_Clock_Disable;
  USART_InitStructure.USART_CPOL = USART_CPOL_Low;
  USART_InitStructure.USART_CPHA = USART_CPHA_2Edge;
  USART_InitStructure.USART_LastBit = USART_LastBit_Disable;

  /* 설정된 구조체 값을 실제 CR1, CR2, CR3, BRR 레지스터에 기록 */
  USART_Init(USART1, &USART_InitStructure);  

  /* USART1 전체 전원 스위치 ON (CR1 레지스터의 UE 비트 = 1) */
  USART_Cmd(USART1, ENABLE);
  
  /* 부팅 확인용 테스트 메시지 출력 */
  puts("Hello World! \r\n");
  
  while (1){
    /* 
     * 1. getch()가 호출되어 사용자가 키보드를 칠 때까지(RXNE=1) 무한 대기
     * 2. 키보드 입력이 들어오면 해당 문자를 반환
     * 3. 반환된 문자를 곧바로 putch()의 인자로 넣어 화면에 다시 출력(TXE=1 대기)
     */
    putch(getch());
  }
}
