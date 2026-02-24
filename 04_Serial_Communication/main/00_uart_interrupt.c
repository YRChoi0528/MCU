/* 
 * 변경된 부분만 작성하고 나머지는 생략하겠다. 
 * USART_Init() 함수 내부이다.
 */

/* ... (생략) ... */

/*  USART CR1 설정 -*/
  tmpreg = 0x00;
  tmpreg = USARTx->CR1;
  /* M, PCE, PS, TE, RE bits 클리어 */
  tmpreg &= CR1_CLEAR_Mask;

  /* USART 데이터 길이, 패리티, 모드 설정 ----------------------- */
  tmpreg |= (u32)USART_InitStruct->USART_WordLength | USART_InitStruct->USART_Parity |
            USART_InitStruct->USART_Mode;
            
  /* 
   * 초기화와 동시에 수신 인터럽트(RXNE Interrupt Enable)를 활성화한다.
   * - 0x0020은 이진수로 0000 0000 0010 0000 이며, CR1 레지스터의 5번 bit(RXNEIE)에 해당한다
   * - 해당 비트가 1이 되면, 수신 버퍼에 데이터가 도착하여 RXNE=1이 될 때마다
   * NVIC를 통해 CPU에게 인터럽트 신호를 보내 USART1_IRQHandler()를 실행하게 만든다.
   * 수업에서 이해를 돕고자 라이브러리를 직접 수정했지만, main.c에서 USART_Init(); 이후 USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 함수를 호출하여 활성화하는 것을 권장한다.
   */
  tmpreg |= 0x0020; 

  /* Write to USART CR1 */
  USARTx->CR1 = (u16)tmpreg;

/* ... (생략) ... */
