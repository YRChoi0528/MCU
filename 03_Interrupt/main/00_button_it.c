/* 작성된 ISR 부분만 발췌해서 작성했습니다. */

/* ... (생략) ... */

/*
 * EXTI0_IRQHandler
 * 버튼 0(PB0)이 눌렸을 때 발생하는 인터럽트 서비스 루틴
 * LED 이동을 멈추거나(정지) 다시 시작(재생)하는 토글 역할
 */
extern char stop;
void EXTI0_IRQHandler(void)
{
  /* 해당 라인의 인터럽트 플래그가 셋팅되었는지 확실히 검사 */
  if(EXTI_GetITStatus(EXTI_Line0) != RESET){
    /* 삼항 연산자를 이용한 토글 로직: 0이면 1로, 1이면 0으로 상태 반전 */
    stop = (stop==0)?1:0;
    
    /* 인터럽트 처리가 끝났으므로 Pending 레지스터 비트 클리어 (필수) */
    EXTI_ClearITPendingBit(EXTI_Line0);
  } 
}

/*
 * EXTI1_IRQHandler
 * 버튼 1(PB1)이 눌렸을 때 발생하는 인터럽트 서비스 루틴
 * LED 이동 방향을 오른쪽(dir=1)으로 설정
 */
extern char dir;
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET){
    dir = 1; 
    EXTI_ClearITPendingBit(EXTI_Line1);
  } 
}

/*
 * EXTI2_IRQHandler
 * 버튼 2(PB2)가 눌렸을 때 발생하는 인터럽트 서비스 루틴
 * LED 이동 방향을 왼쪽(dir=0)으로 설정
 */
void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET){
    dir = 0; 
    EXTI_ClearITPendingBit(EXTI_Line2);
  } 
}

/* ... (생략) ... */
