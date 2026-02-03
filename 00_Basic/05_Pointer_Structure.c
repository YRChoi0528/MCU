#include <stdio.h>
#include <stdlib.h>
#include "05_pointer_structure.h"

void set(S *p, int a, int b)
{
    p->a = a;
    p->b = b;
}

int sum(const S *p) // const : "읽기만 한다"는 의도를 가짐.
{
    return p->a + p->b;
}

int main(void)
{
    S s = {1, 2};
    S *p = &s;

    /* . 과 -> */
    printf("%d\n", s.a);     // 구조체 변수
    printf("%d\n", p->a);    // 구조체 포인터 
    printf("%d\n", (*p).a);  // 구조체 포인터 다른 표현

    set(p, 3, 4);            // 원본 s가 바뀜 
    printf("%d\n", sum(&s));

    /* 
    동적 할당: S n개 
    n : 동적으로 만들 구조체 배열의 요소 개수
    sizeof(S) : 구조체 S 1개가 차지하는 바이트 수
    sizeof(S) * n : 구조체 S n개를 담을 만큼의 연속된 메모리 크기
    malloc(...) : 힙(heap) 영역에서 괄호안의 값의 크기만큼 메모리를 확보한다.
    - 성공하면 할당된 메모리의 시작주소 반환.
    - 실패하면 NULL 반환.
    t : S를 가리키는 포인터
    *t : 구조체 S
    */
    int n = 3;
    S *t = malloc(sizeof(S) * n);
    /*
    동적 할당 실패시 t 는 NULL 이기 때문에 프로그램을 종료하기 위해 return 1; 을 한다.
    return 1; 은 관례적으로 "정상 종료가 아닌 종료"를 의미한다.
    */

    if (!t) return 1; 

    for (int i = 0; i < n; i++)
        set(&t[i], i, i + 10); // t[i] : i 번째 구조체, &t[i] : i 번째 구조체의 주소

    for (int i = 0; i < n; i++)
        printf("%d\n", sum(&t[i]));

    free(t);

    return 0;
}