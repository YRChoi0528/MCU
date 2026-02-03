#include <stdio.h>

void inc(int *p)
{
    (*p)++;
}

void swp(int *p, int *q)
{
    /*
    *p : p 가 가리키는 값.
    *q : q 가 가리키는 값.
    예를 들어 설명하겠다.
    int a = 3, b = 4; 에서
    swp(&a, &b); 를 하면, 
    p는 a의 주소이므로 *p는 a의 값이며,
    q는 b의 주소이므로 *q는 b의 값이다.
     */
    int t = *p;
    *p = *q;
    *q = t;
}

int sum(const int *p, int n)
{
    int i, s = 0;
    for (i = 0; i < n; i++)
        s += p[i];         /* *(p+i)와 같은 의미로 볼 수 있음 */
    return s;
}

int main(void)
{
    int a = 3, b = 4;
    int *p = &a;

    printf("a=%d\n", a);
    *p = 10;               /* a를 10으로 바꿈 */
    printf("a=%d\n", a);

    printf("%p\n", &a); // a 주소
    printf("%d\n", *p); // a 값
    printf("%p\n", p);  // a 주소
    printf("%p\n", &p); // p 주소

    inc(&a);               /* 주소 전달: inc() 함수가 반환(return)이 없는데 a 값을 바꿈 */
    printf("a=%d\n", a);

    swp(&a, &b);
    printf("a=%d b=%d\n", a, b);

    {
        int x[5] = {1,2,3,4,5};
        int *q = x;        /* x는 식에서 &x[0]처럼 동작 */
        printf("x[0]=%d\n", *q);
        printf("x[1]=%d\n", *(q + 1));
        printf("sum=%d\n", sum(x, 5));
    }

    return 0;
}
