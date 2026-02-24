/* ... (생략) ... */

/*
 * USART1_IRQHandler
 * USART1의 글로벌 인터럽트 서비스 루틴 (ISR)
 * 데이터가 수신되어 RXNE 플래그가 1이 되거나, 전송 관련 플래그가 켜질 때 호출됨.
 */

void putch(unsigned char c){
  USART_SendData(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

unsigned char getch(){
  /*
   * ISR로 들어오면 RXNE 플래그가 1이 되어있으므로 기존에 있던 while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET); 는 무의미하다.
   */
  return USART_ReceiveData(USART1);
}

void USART1_IRQHandler(void){
  /*
   * 인터럽트 기반의 통신!
   * 1. PC에서 글자를 보내면 하드웨어가 수신을 완료하고 RXNE=1을 만든 뒤 해당 ISR을 호출한다.
   * 2. getch()가 호출되어 DR 레지스터에서 값을 읽어온다. (이때 하드웨어가 알아서 RXNE=0으로 클리어함 )
   * 3. 읽어온 값을 곧바로 putch()를 통해 다시 PC로 돌려보낸다.
   * 4. 처리가 끝나면 다시 메인 루프(while(1))로 돌아간다.
   */
  putch(getch());
}

/* ... (생략) ... */
