/*
해당 코드에서 헤더 가드는 다음과 같다.
#ifndef BASIC_MACRO_PREPROCESSOR_H
#define BASIC_MACRO_PREPROCESSOR_H
~
#endif
*/
#ifndef BASIC_MACRO_PREPROCESSOR_H // 헤더 가드
#define BASIC_MACRO_PREPROCESSOR_H // 헤더 가드

/* 단순 치환 */
#define N 10

/* 괄호를 넣어 우선순위 문제를 줄인 매크로 */
#define SQ(x) ((x) * (x))

#ifndef DBG
#define DBG 1
#endif

#if DBG
#define P(x) printf("%s=%d\n", #x, (x))
#else
#define P(x) ((void)0)
#endif

#endif // 헤더 가드
