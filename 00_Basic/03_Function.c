#include <stdio.h>

/*
따로 03_Function.h 파일을 만들어 선언할 수 있지만
아래와 같이 main() 아래에 정의된 함수를 선언할 수도 있다.
*/

int add(int a, int b)
{
    return a + b;
}

void f1(int a);
void f2(int *p);

int main(void)
{
    int a = 3, b = 4, c;

    c = add(a, b);
    printf("c=%d\n", c);

    f1(a);
    printf("a=%d\n", a);      /* 값 전달: 안 바뀜 */

    f2(&a);
    printf("a=%d\n", a);      /* 주소 전달: 바뀜 */

    return 0;
}

void f1(int a)
{
    a = 100;
}

void f2(int *p)
{
    *p = 100;
}