#include "stm32f10x_lib.h"
#include "System_func.h"
#include "user_delay.h"

/* 제어 핀 및 포트 매크로 정의 (직렬 DAC081S101) */
#define DIN  GPIO_Pin_0  /* 직렬 데이터 입력 핀 */
#define CLK  GPIO_Pin_1  /* 동기화 클럭 핀 */
#define SYNC GPIO_Pin_2  /* 프레임 동기화(시작/종료) 핀 */

#define S_DAC_PORT GPIOC
#define S_DAC_PORT_CLK  RCC_APB2Periph_GPIOC

/* 함수 선언 */
void init_S_DAC(void);                   /* Serial DAC 통신 핀 초기화 */
void Convert_sDAC(unsigned char value);  /* 16bit 프레임 전송 및 DAC 변환 제어 */

int main(void){

  Init_STM32F103();

  unsigned char VolC = 0x00;
  
  Timer3_Delay_init();
  
  /* 직렬 DAC 제어용 GPIO 초기화 */
  init_S_DAC();
  
  /* 10ms 주기로 전압을 상승시키는 톱니파 생성 루프 */
  while(1){
    Convert_sDAC(VolC);
 
    VolC += 4; /* 출력 전압 단계적 상승 */
 
    VolC = (VolC == 0 ? 0x01 : VolC); /* 최대치 도달 시 최저 전압(0x01)으로 초기화 */
    delay_ms(10);
  }
}

void init_S_DAC(void){
  
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOC 클럭 활성화 */
  RCC_APB2PeriphClockCmd(S_DAC_PORT_CLK, ENABLE);

  /* DIN, CLK, SYNC 핀을 범용 출력 Push-Pull 모드로 설정 */
  GPIO_InitStructure.GPIO_Pin = CLK | DIN | SYNC;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(S_DAC_PORT, &GPIO_InitStructure);

  /* 초기 상태는 모두 Low로 설정 */
  GPIO_ResetBits(S_DAC_PORT,CLK | DIN | SYNC);
}

/* 소프트웨어 Bit-Banging 방식의 16bit 프레임 전송 함수 */
void Convert_sDAC(unsigned char value){
 
  unsigned int data=0,mask;
  unsigned char i;

  mask=0x8000; /* 16bit MSB 추출용 마스크 (1000_0000_0000_0000) */
 
  data |= value<<4; /* 프레임 규격 정렬  (Dummy(2bit) + Mode(2bit) + Data(8bit) + Dummy(4bit)) */

  /* 통신 초기화 시퀀스 */
  GPIO_SetBits(S_DAC_PORT,SYNC);
  asm("nop");
  GPIO_SetBits(S_DAC_PORT,CLK);
  asm("nop");
  GPIO_ResetBits(S_DAC_PORT,CLK);
  asm("nop");

  /* SYNC를 Low로 전환하여 새로운 데이터 프레임 전송 개시 */
  GPIO_ResetBits(S_DAC_PORT,SYNC);
  asm("nop"); /* t_SUCL (Setup Time for SCLK) 확보 */

  /* 16bit 직렬 데이터 전송 루프 */
  for(i=0;i<16;i++){ 
    GPIO_SetBits(S_DAC_PORT,CLK); /* SCLK High 전환 */
    asm("nop");

    /* 마스크 bit 연산을 통해 현재 전송할 비트 추출 및 DIN 핀 출력 */
    if(data & mask) 
      GPIO_SetBits(S_DAC_PORT,DIN);   
    else 
      GPIO_ResetBits(S_DAC_PORT,DIN);
   
    asm("nop"); /* t_SUD (Data Setup Time) 확보: SCLK 하강 에지 전 데이터 안정화 대기 */
  
    GPIO_ResetBits(S_DAC_PORT,CLK); /* SCLK Low 전환 (하강 에지 발생 → DAC 내부 시프트 레지스터 래치) */
    asm("nop"); /* t_DHD (Data Hold Time) 확보: 데이터가 안정적으로 기록되도록 래치 직후 데이터 유지 */
   
    data<<=1; /* 다음 bit 전송을 위해 데이터를 좌측으로 1bit 시프트 */
  }
}
