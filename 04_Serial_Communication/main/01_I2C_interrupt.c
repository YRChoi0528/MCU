/*
 * 해당 코드는 스텝 모터나 디스플레이 등 실시간 제어가 필요한 장치를 방해하지 않도록,
 * EXTI(외부 인터럽트)를 활용해 SHT11 센서를 구동하는 구조를 설명한다.
 * 기존 코드를 변경하거나 추가할 때 뼈대로 참고하시기 바라며, 그대로 복사 시 오류가 발생할 수 있다.
 * 기존 방식 : SHT11 측정이 완전히 끝날 때까지 CPU가 멈춰서 대기함(블로킹 현상 발생)
 * 변경 방식 : CPU가 SHT11에게 측정 명령만 내리고 본래의 작업을 계속 수행하다가,
 *            SHT11이 측정을 완료하여 신호를 보내면 그때 인터럽트로 가져옴.
 */


/**************** lib_sensor.c ****************/

/* 상태를 기억할 전역 변수 선언 */
volatile u8 acq_complete = 0;     /* 측정 완료 플래 */
volatile u8 current_mode = TEMP;  /* 현재 측정 중인 대상 */
volatile u16 raw_temp /* 온도 순수 데이터 */
volatile u16 raw_humi /* 습도 순수 데이터 */

/* STEP 1. CPU가 SHT11에게 측정 지시만 하고 바로 빠져나오는 함수(main.c에서 호출) */
u8 sht11_start_measure_IT(u8 mode)
{
  unsigned short error = 0;
  current_mode = mode;
  acq_complete = 0;

  s_transstart();  /* 통신 시작 */
  if(mode == TEMP) error+=s_write_byte(MEASURE_TEMP);
  else             error+=s_write_byte(MEASURE_HUMI);

  if(error != 0) return error;
  
  MAKE_I2CDATA_INPUT(); /* SDA 라인을 입력으로 변경하여 센서에게 제어권을 넘김 */

  /*
   * 통신 중에 인터럽트가 켜져 있으면 data bit가 0으로 내려갈 때마다 오작동하기 때문에
   * 명령을 다 보낸 이후에 PC1(SDA) 핀의 Falling Edge 외부 인터럽트를 활성화 한다.
   */
  EXTI -> IMR |= EXTI_Line1; /* PC1이 SDA라인과 연결되어 있기 때문에, EXTI1 인터럽트 활성화 (핀 변경시 반드시 EXTI_Line 변경 해야함) */
  
  return error;
}

/* STEP 2. 센서가 SDA라인을 0으로 내렸을 때(인터럽트 발생) 실행될 함수(it.c에서 호출) */
void sht11_read_data_ISR(void)
{
  u16 raw_value, checksum;

  /* 데이터를 읽는 동안 다시 인터럽트가 걸리지 않도록 즉시 비활성화 한다. */
  EXTI -> IMR &= ~EXTI_Line1; /* EXTI1 인터럽트 비활성화 */

  /* SHT11이 변환(아날로그→디지털)을 마쳤으므로 바로 데이터를 읽어온다.(이전에는 while문으로 변환할 때까지 대기함.) */
  u8 msb = s_read_byte(ACK);
  u8 lsb = s_read_byte(ACK);
  raw_value = (msb * 256) + lsb;
  checksum = s_read_byte(noACK);  /* 통신 종료 */

  /* 순수 데이터(Raw Data)만 전역 변수에 저장 */ 
  if(current_mode == TEMP) raw_temp = raw_value;
  else if(current_mode == HUMI) raw_humi = raw_value;

  acq_complete = 1; /* 메인 루프에 데이터를 다 가져왔음을 알림 */
}

/* STEP 3. 데이터 변환 및 반환 함수 (main.c에서 호출) */
u16 get_calculated_sht11_data(u8 type)
{
  calc_sht11(raw_humi, raw_temp);

  if(type == TEMP) return mytemp;
  else if(type == HUMI) return myhumi;
  else return 0;
}


/******************* main.c *******************/
#include "stm32f10x_lib.h"
#include "lib_sensor.h"

/* EXTI 설정 함수 */
void EXTI1_Config(void) 
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; /* 우선순위는 상황게 맞게 설정을 하도록 한다.*/
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
    
  /* EXTI 초기화 (Falling Edge 설정) */
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 1 → 0으로 떨어질 때
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);  

  /* 시작 시점에는 통신 중 오작동을 막기 위해 EXTI1을 잠시 끈다. */
  EXTI -> IMR &= ~EXTI_Line1;
}

int main(void)
{
  Init_STM32F103();
  initialize_sht11_hanback();
  EXTI1_Config();

  /* --- 생략 --- */

  u8 step = 0; /* 상태 머신 변수 (0: 온도 명령, 1: 온도 대기, 2: 습도 명령, 3: 습도 대기) */
  u16 temp, humi;
  while(1)
  {
    Run_Motor(); /* 모터 구동 코드(가정) */

    switch(step)
    {
      case 0: if(sht11_start_measure_IT(TEMP) == 0) step = 1; break;
      case 1: if(acq_complete == 1) acq_complete = 0; step = 2; break;
      case 2: if(sht11_start_measure_IT(HUMI) == 0) acq_complete = 0; step = 3; break;
      case 3:
        if(acq_complete == 1)
        {
          temp = get_calculated_sht11_data(TEMP);
          humi = get_calculated_sht11_data(HUMI);

          Print_To_LCD(temp, humi); /* LCD에 측정한 온/습도 출력해주는 코드(가정) */
          step = 0;
        }
        break;
    }
  }
}

/******************** it.c ********************/
#include "lib_sensor.h" /* 센서 헤더 추가 */

void EXTI1_IRQHandler(void) {
    /* 인터럽트가 PC1(EXTI Line 1)에서 발생한 게 맞는지 확인 */
    if(EXTI_GetITStatus(EXTI_Line1) != RESET) {
        
        /* 센서 데이터 읽기 함수 호출 (대기 없이 바로 읽어옴) */
        sht11_read_data_ISR();
        
        /* 인터럽트 Pending bit clear */
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
