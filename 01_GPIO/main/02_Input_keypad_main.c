#include "stm32f10x_lib.h"
#include "System_func.h"

/* 소프트웨어 딜레이 함수 */
void delay(u32 count){
  while(count--);
}

int main(void){
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 시스템 초기화 */
  Init_STM32F103();

  /* 출력(GPIOA, FND 연결)과 제어/입력(GPIOC, 키패드 연결) 포트 클럭 활성화 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);  
  
  /* FND 데이터 테이블: 0~9 숫자를 표시하기 위한 7비트 패턴 정의 */
  unsigned char FND_DATA_TBL[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7C,0x07,0x7F,0x67}; 
  
  /* GPIOA (0~7번): FND의 각 세그먼트 제어용 출력 설정 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOC (0~2번): 키패드의 열(Column) 감지용 입력 설정 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* GPIOC (3~6번): 키패드의 행(Row) 구동용 출력 설정 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  while(1){
    /* 첫 번째 행(PC3) 스캔 */
    GPIO_ResetBits(GPIOC, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6); // 모든 행 초기화
    GPIO_SetBits(GPIOC, GPIO_Pin_3); // 1번 행에 전압 인가
    
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)){ // 1번 행 & 1번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF); // FND 초기화
      GPIO_SetBits(GPIOA, FND_DATA_TBL[1]); // '1' 표시
      while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)); // 버튼을 뗄 때까지 대기
    }
    else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)){ // 1번 행 & 2번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[2]); // '2' 표시
      while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1));
    }
    else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)){ // 1번 행 & 3번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[3]); // '3' 표시
      while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2));
    }
    delay(0x80); // 스캔 안정화를 위한 짧은 지연

    /* 두 번째 행(PC4) 스캔 */
    GPIO_ResetBits(GPIOC,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
    GPIO_SetBits(GPIOC,GPIO_Pin_4); // 2번 행에 전압 인가
    
    if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)){ // 2번 행 & 1번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[4]); // '4' 표시
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0));
    }
    else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)){ // 2번 행 & 2번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[5]); // '5' 표시
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1));
    }
    else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2)){ // 2번 행 & 3번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[6]); // '6' 표시
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2));
    }
    delay(0x80);	

    /* 세 번째 행(PC5) 스캔 */
    GPIO_ResetBits(GPIOC,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
    GPIO_SetBits(GPIOC,GPIO_Pin_5); // 3번 행에 전압 인가
    if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)){ // 3번 행 & 1번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[7]); // '7' 표시
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0));
    }
    else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)){ // 3번 행 & 2번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[8]); // '8' 표시
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1));
    }
    else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2)){ // 3번 행 & 3번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[9]); // '9' 표시
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2));
    }
    delay(0x80);
    
    /* 마지막 행(PC6) 스캔 */
    GPIO_ResetBits(GPIOC, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);
    GPIO_SetBits(GPIOC, GPIO_Pin_6); // 4번 행에 전압 인가
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)){ // 4번 행 & 2번 열 확인
      GPIO_ResetBits(GPIOA, 0xFF);
      GPIO_SetBits(GPIOA, FND_DATA_TBL[0]); // '0' 표시
      while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1));
    }
    delay(0x80);
  }
}
