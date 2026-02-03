/*
다음과 같이 01_macro_preprocessor.h를 선언함으로써 
해당 헤더 파일에 선언된 N, SQ(x), P(x)를 
01_Macro_Preprocessor.c 에 선언되어 있지 않아도 사용할 수 있다.
*/
#include <stdio.h>
#include "01_macro_preprocessor.h"

/* 나쁜 예: 괄호가 없어 식이 꼬일 수 있음 */
#define BAD(x) x * x

int main(void)
{
    int a = 3;
    int b = 4;

    printf("N=%d\n", N);

    /* 괄호 유무 차이 */
    printf("BAD(a+1)=%d\n", BAD(a + 1));  /*  a + 1 * a + 1 처럼 보일 수 있음 */
    printf("SQ(a+1)=%d\n", SQ(a + 1));    /* (a+1)*(a+1) 형태로 안전해짐 */

    /* 조건부 컴파일 매크로: DBG=1일 때만 출력 */
    P(a);
    P(b);

    return 0;
}
