#include "stm32f10x_lib.h"
#include "lcd.h"
#include "at25fxx.h"
#include "System_func.h"

int main(void){
  
  Init_STM32F103(); /* 시스템 클럭 및 기본 하드웨어 초기화 */
  
  u8 buf[20];
  unsigned char tmp;
  
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  lcdInit(); /* 상태를 출력할 LCD 초기화 */
  
  /* SPI1 모듈과 해당 핀들이 위치한 GPIOA의 APB2 버스 클럭을 활성화한다. */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);  
  
  /* 
   * PA5(SCK), PA6(MISO), PA7(MOSI) 핀을 SPI 통신용으로 설정한다.
   * MISO는 입력을 사용하나, STM32 Master 모드 특성상 AF_PP로 일괄 설정해도 하드웨어적으로 입력 처리함.
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*
   * PA4 핀을 하드웨어 NSS가 아닌 범용 출력(Out_PP)으로 설정하여 소프트웨어적으로 제어한다.
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  AT25FXX_CS_HIGH(); /* 초기 상태는 CS를 High로 두어 통신 대기(비활성) 상태로 유지 */

  /* SPI1 하드웨어 모듈 초기화 (SPI_CR1 레지스터 설정) */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  /* 2선식 전이중 통신 */
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       /* MCU를 Master로 설정 (MSTR=1) */
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   /* 8비트 데이터 프레임 (DFF=0) */
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                          /* 클럭 극성: 대기 시 Low (CPOL=0) */
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                        /* 클럭 위상: 첫 번째 엣지에서 캡처 (CPHA=0) */
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           /* Software NSS 사용 (SSM=1, SSI=1) */
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; /* APB2 클럭(72MHz) 32분주 -> 2.25MHz 통신 속도 */
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  /* 최상위 비트(MSB)부터 전송 */
  SPI_InitStructure.SPI_CRCPolynomial = 7;                            /* CRC 다항식 초기값 (실제 CRC 기능은 끄고 사용함) */
  SPI_Init(SPI1, &SPI_InitStructure);
  
  SPI_Cmd(SPI1, ENABLE); /* SPI 모듈 전원 인가 및 동작 시작 (SPE=1) */
  
  /* 모든 통신은 CS Low → 명령 전송 → 주소/데이터 처리 → CS High의 프레임으로 이루어진다. */
  
  /* 1. 메모리 칩 전체 지우기 (모든 비트를 0xFF로 초기화) */
  at25fxx_CHIP_ERASE();    
  
  /* 2. 단일 바이트 쓰기 및 읽기 */
  at25fxx_Write_Byte(0x0100, 9);       /* 주소 0x0100에 숫자 9를 기록 */
  tmp = at25fxx_Read_Byte(0x0100);     /* 동일한 주소에서 값을 읽어옴 */
  lcdDataWrite(tmp + '0');             /* 읽어온 정수(9)를 아스키코드 문자('9')로 변환하여 LCD 출력 */
  
  /* 3. 배열(문자열) 연속 쓰기 및 읽기 */
  lcdGotoXY(1, 1);                     /* LCD 커서를 다음 줄로 이동 */
  
  /* 주소 0x0200부터 10바이트 크기의 "Hello SPI!!" 문자열 쓰기 */
  at25fxx_Write_Arry(0x0200, "Hello SPI!!", 10); 
  
  /* 주소 0x0200부터 다시 10바이트를 읽어와 buf 배열에 순차적으로 저장 */
  at25fxx_Read_Arry(0x0200, buf, 10);
  
  /* 수신한 buf 배열의 데이터를 LCD에 출력 */
  lcdPrintData(buf, 10);
  
  while(1) {
    /* no work. 무한 루프 대기 */
  }
}
