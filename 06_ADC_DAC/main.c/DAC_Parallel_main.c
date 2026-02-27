#include "stm32f10x_lib.h"
#include "user_delay.h"
#include "System_func.h"

/* 
 * 제어 핀 및 포트 매크로 정의 (병렬 AD7302) 
 * 데이터 버스 : PA0 ~ PA7
 * 제어   버스 : PC0 ~ PC3
 */
#define AD7302_DATA       GPIOA
#define AD7302_DATA_CLK   RCC_APB2Periph_GPIOA

#define AD7302_CTRL       GPIOC
#define AD7302_CTRL_CLK   RCC_APB2Periph_GPIOC

#define P_CS    GPIO_Pin_0  /* Chip Select (Active Low) */
#define P_WR    GPIO_Pin_1  /* Write Enable (Active Low, 상승 에지에서 래치) */
#define P_LDAC  GPIO_Pin_2  /* Load DAC (Active Low, DAC 출력 갱신) */
#define P_AB    GPIO_Pin_3  /* DAC 채널 선택 (Low: DAC A, High: DAC B) */

#define CTRL_PIN  (P_CS|P_WR|P_LDAC|P_AB)

/* 채널 선택용 상수 */
#define OUTA 0x00
#define OUTB P_AB

/* 함수 선언 */
void DAC_init();
void DAC_vol_gen(unsigned int selAB,unsigned char voltage);

int main(void){
  Init_STM32F103();
  Timer3_Delay_init();

  /* 출력 전압 변수 초기화 (VolA: 증가, VolB: 감소) */
  unsigned char VolA=0x01,VolB=0xFF;

  /* 병렬 DAC 제어용 GPIO 포트 초기화 */
  DAC_init();

  /* 초기 출력 전압을 0V로 설정 */
  DAC_vol_gen(OUTA,0);
  DAC_vol_gen(OUTB,0);

  /* OUTA는 전압 상승, OUTB는 전압 하강 구현 */
  while(1){
    /* 각 채널에 8bit 디지털 데이터 인가 및 출력 변환 */
    DAC_vol_gen(OUTA,VolA); 
    DAC_vol_gen(OUTB,VolB); 
 
    /* 데이터 갱신 (톱니파 형태 생성) */
    VolA+=4;
    VolB-=4;

    /* 오버플로우/언더플로우 방지 및 초기화 조건 */
    VolA =(VolA == 0 ? 0x01 : VolA);
    VolB =(VolB == 0 ? 0xFF : VolB);

    delay_ms(10);    
  }
}

/* 병렬 DAC(AD7302) 핀 초기화 및 통신 대기 상태 설정 */
void DAC_init(){
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOA, GPIOC 클럭 활성화 */
  RCC_APB2PeriphClockCmd(AD7302_DATA_CLK|AD7302_CTRL_CLK, ENABLE);

  /* 데이터 핀(PA0~PA7) 범용 출력 Push-Pull 모드 설정 */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
  GPIO_InitStructure.GPIO_Pin =  0x00FF ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(AD7302_DATA, &GPIO_InitStructure);

  /* 제어 핀(PC0~PC3) 범용 출력 Push-Pull 모드 설정 */
  GPIO_InitStructure.GPIO_Pin = CTRL_PIN ;
  GPIO_Init(AD7302_CTRL, &GPIO_InitStructure);

  /* 포트 초기 상태 확립 */
  GPIO_ResetBits(AD7302_DATA,0x00FF);       /* 데이터 버스 초기화 */
  GPIO_ResetBits(AD7302_CTRL,P_LDAC|P_AB);  /* LDAC를 Low로 고정하여 자동 갱신 활성화 */
  
  GPIO_SetBits(AD7302_CTRL,P_CS | P_WR);    /* CS, WR 핀을 High로 설정하여 통신 비활성화 */
}

/*
 * 지정된 DAC 채널에 8bit 데이터를 출력하여 아날로그 전압 갱신
 * selAB : 출력할 DAC 채널 (OUTA 또는 OUTB)
 * voltage : 0~255 범위의 8bit 디지털 데이터
 */
void DAC_vol_gen(unsigned int selAB,unsigned char voltage){

  /* 채널 선택 (A/B 제어 신호 인가) */
  if(selAB != OUTA)
    GPIO_SetBits(AD7302_CTRL,selAB);    /* B채널 요청 시 High 인가 */
  else
    GPIO_ResetBits(AD7302_CTRL,selAB);  /* A채널 요청 시 Low 인가 */

  /* 통신 시작 및 데이터 입력 개시 (CS, WR 핀을 Low 상태로 전환) */
  GPIO_ResetBits(AD7302_CTRL,P_CS);
  GPIO_ResetBits(AD7302_CTRL,P_WR);

  /* 데이터 버스에 8bit 값 인가 (하드웨어 Data Setup Time 확보 구간) */
  GPIO_ResetBits(AD7302_DATA,0x00FF);  /* 버스 클리어 */ 
  GPIO_SetBits(AD7302_DATA,voltage);   /* 전압 데이터 출력 */

  /* 
   * 데이터 래치 및 통신 종료 
   * WR 신호가 High로 전환되는 상승 에지 순간 데이터가 레지스터에 고정됨.
   * 초기화 시 LDAC가 Low로 고정되어 있으므로 래치와 동시에 아날로그 출력이 갱신됨.
   */
  GPIO_SetBits(AD7302_CTRL,P_WR);
  GPIO_SetBits(AD7302_CTRL,P_CS);
}
