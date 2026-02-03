#ifndef BASIC_POINTER_STRUCTURE_H
#define BASIC_POINTER_STRUCTURE_H

typedef struct {
    int a;
    int b;
} S;

void set(S *p, int a, int b);
int  sum(const S *p);

#endif