#ifndef BASIC_STRUCTURE_H
#define BASIC_STRUCTURE_H

typedef struct {
    int a;
    int b;
} S; // S 는 int a 와 int b 로 구성된 구조체 이다(타입 정의).

int  sum(S s);
void inc(S *p);

#endif