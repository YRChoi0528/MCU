/* 작성된 ISR 부분만 발췌해서 작성했다. */

/* ... (생략) ... */

/*
 * EXTI0_IRQHandler
 * 버튼 0(PB0)이 눌렸을 때 발생하는 인터럽트 서비스 루틴
 */
extern vu16 LED_data;
void EXTI0_IRQHandler(void)
{
  /* 해당 라인의 인터럽트 플래그가 SET 되었는지 검사 */
  if(EXTI_GetITStatus(EXTI_Line0) != RESET){
    
    u16 mask = 0x0001; /* 0번째 bit만 제어하기 위한 마스크 값 */
    
    u16 pin_state = LED_data & mask; /* LED_data의 0번째 bit 상태만 추출 */
    
    if(pin_state) LED_data &= ~mask; /* 해당 bit가 1이면 0으로 Clear */
    else LED_data |= mask; /* 해당 bit가 0이면 1로 SET */

    /* 인터럽트 처리가 끝났으므로 Pending 레지스터 bit Clear (필수) */
    EXTI_ClearITPendingBit(EXTI_Line0);
  } 
}

/*
 * EXTI1_IRQHandler
 * 버튼 1(PB1)이 눌렸을 때 발생하는 인터럽트 서비스 루틴
 */
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET){
    u16 mask = 0x0002;
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line1);
  } 
}

/*
 * EXTI2_IRQHandler
 * 버튼 2(PB2)가 눌렸을 때 발생하는 인터럽트 서비스 루틴
 */
void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET){
    u16 mask = 0x0004;
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

/*
 * EXTI9_5_IRQHandler
 * 버튼 3~7(PB5~PB9)가 눌렸을 때 발생하는 인터럽트 서비스 루틴
 */
void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line5) != RESET){
    u16 mask = 0x0008;
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line5);
  }

  if(EXTI_GetITStatus(EXTI_Line6) != RESET){
    u16 mask = 0x0010;    
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line6);
  }

  if(EXTI_GetITStatus(EXTI_Line7) != RESET){
    u16 mask = 0x0020;
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line7);
  }
  
  if(EXTI_GetITStatus(EXTI_Line8) != RESET){
    u16 mask = 0x0040;
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line8);
  }
  
  if(EXTI_GetITStatus(EXTI_Line9) != RESET){
    u16 mask = 0x0080;
    u16 pin_state = LED_data & mask;
    
    if(pin_state) LED_data &= ~mask;
    else LED_data |= mask;
    
    EXTI_ClearITPendingBit(EXTI_Line9);
  }
}

/* ... (생략) ... */
