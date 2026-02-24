/* ... (생략) ... */

/*
 * TIM2_IRQHandler
 * TIM2 Update 이벤트(ARR 도달, 1초 경과) 발생 시 호출되는 ISR
 * FND 출력용 전역 변수를 1씩 증가시킨다.
 */
extern u8 time_1s;
void TIM2_IRQHandler(void){
  /* 인터럽트의 원인이 TIM2 Update(오버플로우)가 맞는지 플래그 확인 */
  if(TIM_GetFlagStatus(TIM2,TIM_IT_Update)==SET){
    
    /* 인터럽트 무한 반복을 막기 위해 Pending 비트를 즉시 클리어 */
    TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update);
    
    /* FND 카운트 로직: 9까지 증가하면 다시 0으로 초기화 */
    if(time_1s == 9)
      time_1s = 0;
    else
      time_1s++;
  }
}

/* ... (생략) ... */
