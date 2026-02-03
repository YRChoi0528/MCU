#include <stdio.h>
#include "04_structure.h"

int sum(S s)
{
    /*
    s.a : s 의 멤버 a
    s.b : s 의 멤버 b
    */
    return s.a + s.b;
}

void inc(S *p)
{
    p->a++;
    p->b++;
}

int main(void)
{
    S s = {1, 2};
    int x;

    x = sum(s);
    printf("x=%d\n", x);

    inc(&s);
    printf("s.a=%d s.b=%d\n", s.a, s.b);

    printf("s의 주소: %p\n", &s);
    printf("s.a의 주소: %p\n", &s.a);
    printf("s.b의 주소: %p\n", &s.b);

    S t[3] = {{1,2},{3,4},{5,6}};
    for (int i = 0; i < 3; i++)
        printf("%d\n", sum(t[i]));

    return 0;

}
