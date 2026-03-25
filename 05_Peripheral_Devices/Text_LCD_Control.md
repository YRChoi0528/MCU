# Text LCD Control.md

## 1. 요약

해당 문서는 STM32F103 마이크로컨트롤러의 GPIO를 사용하여 Text LCD를 제어하는 펌웨어 로직을 분석한 문서이다. <br>
8bit 데이터 버스 인터페이스를 기반으로 한 초기화 시퀀스, Busy Flag 폴링 기법, 그리고 CGRAM을 활용한 사용자 정의 문자 출력 및 애니메이션 구현 방법을 다룬다.

---

## 2. 하드웨어 핀 매핑

LCD 제어를 위해 MCU의 범용 포트를 제어 버스와 데이터 버스로 나누어 연결한다.

- **Control Bus** (`GPIOB` 포트 사용):
  - `PB0` → `RS` (Register Select): 명령(0)과 데이터(1)를 구분
  - `PB1` → `R/W` (Read/Write): 쓰기(0)와 읽기(1) 동작을 결정
  - `PB2` → `E` (Enable): 신호를 LCD 내부로 전달하는 트리거

- **Data Bus** (`GPIOA` 포트 사용):
  - `PA0`-`PA7` → `DB0`-`DB7` (8bit interface Mode)
  - `PA4`-`PA7` → `DB4`-`DB7` (4bit interface Mode)
```c
/* lcdconf.h */
#ifndef LCDCONF_H
#define LCDCONF_H

#define LCD_PORT_INTERFACE
//#define LCD_DATA_4BIT

#ifdef LCD_PORT_INTERFACE
	#ifndef LCD_CTRL_PORT
		#define LCD_CTRL_PORT_CLK	RCC_APB2Periph_GPIOB
                #define LCD_CTRL_PORT	        GPIOB
		#define LCD_CTRL_RS		GPIO_Pin_0
		#define LCD_CTRL_RW		GPIO_Pin_1
		#define LCD_CTRL_E		GPIO_Pin_2
	#endif
	#ifndef LCD_DATA 

		#define LCD_DATA	GPIOA
		#define LCD_DATA_CLK	RCC_APB2Periph_GPIOA
	#endif
#endif
```

---

## 3. 펌웨어 로직 분석

### 3.1 하드웨어 제어 및 상태 확인

`lcdDelayLoop()` 함수는 LCD 컨트롤러의 각 명령 처리 시 요구되는 최소한의 **준비 및 유지 시간(Setup/Hold Time)** 을 보장하기 위해 소프트웨어적으로 미세 지연을 생성한다.

```c
static void lcdDelayLoop(volatile u32 count)
{
    while(count--) {
        /* 
         * asm("nop") ~ 13.9ns (72MHz 기준)
         * LCD_DELAY는 asm("nop") 12번 사용 ~167ns 
         * while 루프까지 고려하면 1회 반복이 대략 0.2~0.3us
         */
        LCD_DELAY;
    }
}
```

`lcdSetDataOutput()` 함수는 MCU가 LCD에 명령이나 데이터를 기록(Write)할 수 있도록 데이터 버스 핀의 동작 모드를 **Push-Pull 출력 모드**로 전환한다.

```c

static void lcdSetDataOutput(void)
{
#ifdef LCD_DATA_4BIT
    GPIO_LCD.GPIO_Pin = 0x00F0;
#else
    GPIO_LCD.GPIO_Pin = 0x00FF;
#endif
    GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_DATA, &GPIO_LCD);
}
```

`lcdSetDataInput()` 함수는 LCD로부터 상태 정보(Busy Flag)나 내부 데이터를 읽기(Read) 위해 데이터 버스 핀의 동작 모드를 **Floating 입력 모드**로 전환한다.

```c
GPIO_InitTypeDef GPIO_LCD;

static void lcdSetDataInput(void)
{
#ifdef LCD_DATA_4BIT
    GPIO_LCD.GPIO_Pin = 0x00F0;
#else
    GPIO_LCD.GPIO_Pin = 0x00FF;
#endif
    GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_LCD.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(LCD_DATA, &GPIO_LCD);
}
```

`lcdPulseEnable()` 함수는 Enable 핀에 펄스를 인가하여 데이터 버스 상에 인가된 신호를 LCD 내부 레지스터로 **래치 (Latch)** 시키고 실제 동작을 트리거한다.

```c
static void lcdPulseEnable(void)
{
    LCD_DELAY;
    GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);
    LCD_DELAY;
    LCD_DELAY;
    GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);
    LCD_DELAY;
}
```

`lcdInitHW()` 함수는 제어 핀(PB0-2)과 데이터 핀(PA0-7)을 Push-Pull 출력 모드로 초기화 한다.

```c
void lcdInitHW(void)
{
#ifdef LCD_PORT_INTERFACE
    RCC_APB2PeriphClockCmd(LCD_CTRL_PORT_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(LCD_DATA_CLK, ENABLE);

    /* RS, RW, E 핀 초기화 */
    GPIO_LCD.GPIO_Pin = LCD_CTRL_RS | LCD_CTRL_RW | LCD_CTRL_E;
    GPIO_LCD.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_LCD.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_CTRL_PORT, &GPIO_LCD);

    /* 초기 상태: RS=0, RW=0, E=0 */
    GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS | LCD_CTRL_RW | LCD_CTRL_E);

    /* 데이터 버스 초기화 */
    lcdSetDataOutput();
#ifdef LCD_DATA_4BIT
    GPIO_SetBits(LCD_DATA, 0x00F0);
#else
    GPIO_SetBits(LCD_DATA, 0x00FF);
#endif
#endif
}
```

LCD에 새로운 명령을 내리기 전에는 반드시 컨트롤러가 이전 명령을 완료했는지 확인해야 한다. <br>
`lcdBusyWait()` 함수는 데이터 포트(`GPIOA`)를 Floating Input 모드로 변경한 다음, `RS=0`, `R/W=1` 상태에서 Enable 핀을 토글하여 `DB7`(BF)이 0이 될 때까지 대기하는 역할을 한다.

```c
void lcdBusyWait(void)
{
#ifdef LCD_PORT_INTERFACE
    /* Instruction Register 선택, Read 모드 */
    GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS);
    GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_RW);
    lcdSetDataInput();

#ifdef LCD_DATA_4BIT
    while(1)
    {
        u8 data;

        /* 상위 4bit 읽기 (BF 포함) */
        GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);
        LCD_DELAY;
        LCD_DELAY;
        data = (u8)(GPIO_ReadInputData(LCD_DATA) & 0xF0);
        GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);
        LCD_DELAY;
        LCD_DELAY;

        /* 4bit 모드에서는 하위 4bit도 한 번 더 읽어 사이클을 맞춘다 */
        GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);
        LCD_DELAY;
        LCD_DELAY;
        GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);
        LCD_DELAY;
        LCD_DELAY;

        /*
         * DB7(Busy Flag)가 0이면 대기 종료
         * LCD_BUSY = 7
         */
        if((data & (1 << LCD_BUSY)) == 0)
            break;
    }
#else
    while(1)
    {
        u8 data;

        GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_E);
        LCD_DELAY;
        LCD_DELAY;
        data = (u8)(GPIO_ReadInputData(LCD_DATA) & 0x00FF);
        GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_E);

        if((data & (1 << LCD_BUSY)) == 0)
            break;

        LCD_DELAY;
        LCD_DELAY;
    }
#endif
#endif
}
```

### 3.2 제어 명령 및 데이터 기록

LCD 와의 통신은 `RS`핀의 상태에 따라 두 가지로 나뉜다.

- `lcdControlWrite()` (명령어 쓰기):
  `RS`를 0(Low), `R/W`를 0(Low)으로 설정한 후, 데이터 버스에 명령 값을 출력하고 `E`핀을 High에서 Low로 전환(하강 에지)하여 명령을 래치시킨다.

```c
void lcdControlWrite(u8 data)
{
#ifdef LCD_PORT_INTERFACE
    lcdBusyWait();

    /* Instruction Register 선택, Write 모드 */
    GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RS | LCD_CTRL_RW);

#ifdef LCD_DATA_4BIT
    /* 상위 4bit 전송 */
    lcdSetDataOutput();
    GPIO_ResetBits(LCD_DATA, 0x00F0);
    GPIO_SetBits(LCD_DATA, data & 0xF0);
    lcdPulseEnable();
    
    /* 하위 4bit 전송 */
    lcdSetDataOutput();
    GPIO_ResetBits(LCD_DATA, 0x00F0);
    GPIO_SetBits(LCD_DATA, ((data << 4) & 0xF0));
    lcdPulseEnable();
#else
    /* 8bit 전체 전송 */
    lcdSetDataOutput();
    GPIO_ResetBits(LCD_DATA, 0x00FF);
    GPIO_SetBits(LCD_DATA, data);
    lcdPulseEnable();
#endif
#endif
}
```

- `lcdDataWrite()` (데이터 쓰기):
  `RS`를 1(High), `R/W`를 0(Low)으로 설정한 후, 데이터 버스에 ASCII 코드 값을 출력하고 `E`핀을 토글하여 DDRAM 또는 CGRAM에 데이터를 기록한다.

```c
void lcdDataWrite(u8 data)
{
#ifdef LCD_PORT_INTERFACE
    lcdBusyWait();

    /* Data Register 선택, Write 모드 */
    GPIO_SetBits(LCD_CTRL_PORT, LCD_CTRL_RS);
    GPIO_ResetBits(LCD_CTRL_PORT, LCD_CTRL_RW);

#ifdef LCD_DATA_4BIT
    /* 상위 4bit 전송 */
    lcdSetDataOutput();
    GPIO_ResetBits(LCD_DATA, 0x00F0);
    GPIO_SetBits(LCD_DATA, data & 0xF0);
    lcdPulseEnable();
    
    /* 하위 4bit 전송 */
    lcdSetDataOutput();
    GPIO_ResetBits(LCD_DATA, 0x00F0);
    GPIO_SetBits(LCD_DATA, (data << 4) & 0xF0);
    lcdPulseEnable();
#else
    /* 8bit 전체 전송 */
    lcdSetDataOutput();
    GPIO_ResetBits(LCD_DATA, 0x00FF);
    GPIO_SetBits(LCD_DATA, data);
    lcdPulseEnable();
#endif
#endif
}
```

### 3.3 사용자 정의 문자 (CGRAM) 활용

표준 ASCII 테이블에 없는 특수 문자를 출력하려면 **CGRAM** 영역을 사용해야 한다.

- `custom_ver1/2` 함수는 8byte의 도트 패턴 배열을 CGRAM 주소에 기록한다.
- 기록이 완료된 후 다시 **DDRAM 영역으로 복귀하는 명령(`1 << LCD_DDRAM`)을 내려야 이후 문자 출력이 정상적으로 이루어진다.
- 이렇게 생성된 코드는 아스키 코드 `0x00`~`0x07` 번호에 매핑되어 출력 가능하다.

```c
/*
 * CGRAM 0번지에 사용자 정의 문자 패턴 저장
 * 이후 0x00 데이터를 출력하면 해당 문자가 표시된다.
 */
void custom_ver1(void)
{
    unsigned char map[8] = {0x07, 0x09, 0x11, 0x12, 0x1C, 0x04, 0x0E, 0x04};

    /* CGRAM 주소 0번지 선택 */
    lcdControlWrite(0x40);

    /* 8바이트 도트 패턴 기록 */
    for(u8 i = 0; i < 8; i++) lcdDataWrite(map[i]);

    /* 다시 DDRAM 영역으로 복귀 */
    lcdControlWrite(1 << LCD_DDRAM | 0x00);
}

/* CGRAM 0번지에 꽉 찬 블록 형태의 사용자 정의 문자 저장 */
void custom_ver2(void)
{
    unsigned char map[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};

    lcdControlWrite(0x40);

    for(u8 i = 0; i < 8; i++) lcdDataWrite(map[i]);

    lcdControlWrite(1 << LCD_DDRAM | 0x00);
}
```
위 코드를 실행하면 DDRAM에 `0x00`데이터(CGRAM 0번지 매핑)를 출력할 때 배열로 설계한 특수 문자가 화면에 나타나게 된다.

### 3.4 메인 제어 루프

메인 루프에서는 플래그 변수(t)를 활용하여 매 프레임마다 CGRAM의 0번지 모양을 교체한다. <br>
이를 통해 하나의 문자 번호(0x00)만 사용하더라도 화면상에서는 모양이 계속 변하는 효과를 얻는다. <br>
`custom_ver1()`과 `custom_ver2()`는 하나의 문자에 대해서만 작성했는데, 각 함수가 만들 수 있는 사용자 정의 문자는 최대 8개이다. <br>
프로젝트 규모가 커질 경우, 해당 기법을 기억해 두자. 상황에 따라 8개씩의 문자 세트를 통째로 갈아 끼우며 사용하면, 하드웨어의 물리적 한계를 넘어 화려한 애니메이션을 구현할 수 있다.

```c
int main(void){

  Init_STM32F103();
  
  /*
   * 사용자 정의 문자 모양 전환용 플래그
   * (t = 0) → custom_ver1() 사용
   * (t = 1) → custom_ver2() 사용
   */
  u8 t = 0;
  
  /* LCD 초기화 */
  lcdInit(); 
  
  /* 지연용 타이머 초기화 */
  Timer_Delay_init();
  
  while (1){
    
    /*
     * CGRAM 0번 주소에 사용자 정의 문자를 번갈아 저장
     * LCD에 출력할 데이터는 항상 0x00이지만,
     * CGRAM 0번지의 도트 패턴을 바꿔가며 다른 모양이 나타나게 한다.
     */
    if( t == 0 ){ 
      t = 1; 
      custom_ver1(); 
    }
    else{ 
      t = 0; 
      custom_ver2(); 
    }
    
    /*
     * 사용자 정의 문자(0x00)를 LCD 화면 위에서 이동시켜 표시
     * (i = 0~15)  → (첫 번째 줄 왼쪽 → 오른쪽 이동)
     * (i = 16~31) → (두 번째 줄 오른쪽 → 왼쪽 이동)
     */
    for(u8 i=0; i<32; i++){
      
      /* 이전 위치의 문자 지우기 위해 화면 전체 Clear */
      lcdClear();
      delay_ms(2);

      if(i<16){
        /* 첫 번째 줄(y=0)에서 왼쪽 → 오른쪽 방향으로 이동 */
        lcdGotoXY(i, 0);

        /* CGRAM 0번 사용자 정의 문자 출력 */
        lcdDataWrite(0x00);
      }
      else{
        /* 두 번째 줄(y=1)에서 오른쪽 → 왼쪽 방향으로 이동 */
        lcdGotoXY(31-i, 1);

        /* CGRAM 0번 사용자 정의 문자 출력 */
        lcdDataWrite(0x00);
      }

      /* 문자 이동 속도 조절 */
      delay_ms(200);
    }
  }
}
```

`lcdInit()`함수는 LCD 모듈에 전원이 인가된 후, 데이터시트에서 요구하는 명령어 순서(Function Set → Display Control → Clear → Entry Mode)를 구현한 로직이다.

아래 그림은 8bit interface mode에서 명령어 순서이다.
![LCD_Instruction_Set](../images/LCD_Instruction_Set.png)

```c
void lcdInit(void)
{    
    /*하드웨어 핀 초기화: 제어 버스(RS, R/W, E) 및 데이터 버스(DB) GPIO 설정 */
    lcdInitHW(); 
	/* 전원 인가 후 초기 안정화 대기 */
    lcdDelayLoop(150000); /* 30~45ms */

    /*
     * Function Set: 인터페이스 모드(8bit/4bit), 표시 행 수(2줄/1줄), 폰트 크기 설정
     * LCD_FUNCTION_DEFAULT = 0x38(0011 1000) , 8bit mode , 2 line, 5x7 font
     * LCD_FUNCTION_DEFAULT = 0x28(0010 1000) , 4bit mode , 2 line, 5x7 font
     */
    lcdControlWrite(LCD_FUNCTION_DEFAULT);
	lcdDelayLoop(500); /* 50μs ~ 75μs */

	/* 
     * Display ON/OFF Control: 화면 표시 상태 제어
     * 화면 출력(Display)은 활성화(ON)하고, 커서(Cursor) 표시
     * LCD_ON_CTRL = 3
     * LCD_ON_DISPLAY = 2
     */
    lcdControlWrite(1 << LCD_ON_CTRL | 1 << LCD_ON_DISPLAY);
    lcdDelayLoop(500); /* 50us ~ 75us */

    /*
     * Clear Display: 전체 화면을 지우고 내부 Address Counter(AC)를 0으로 초기화
     * LCD_CLR = 0
     */
    lcdControlWrite(1 << LCD_CLR);
    lcdDelayLoop(15000); /* 1.5ms ~ 2ms */

    /* 
     * Entry Mode Set: 데이터를 읽고 쓸 때 커서의 이동 방향 설정
     * LCD_ENTRY_INC 플래그를 통해 데이터 기록 후 커서가 우측(주소 증가 방향)으로 자동 이동하도록 설정
     * LCD_ENTRY_MODE = 2
     * LCD_ENTRY_INC = 1
     */
    lcdControlWrite((1 << LCD_ENTRY_MODE) | (1 << LCD_ENTRY_INC));

    /*
     * Return Home: DDRAM의 데이터는 유지한 채 커서만 초기 위치로 복귀
     * LCD_HOME = 1
     */
    lcdControlWrite(1 << LCD_HOME);

    /*
     * Set DDRAM Address: 데이터 기록을 시작할 메모리 주소를 명시적으로 0x00(1행 1열)으로 설정
     * LCD_DDRAM = 7
     */
    lcdControlWrite((1 << LCD_DDRAM) | 0x00);
}
```
