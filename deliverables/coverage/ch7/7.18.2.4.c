/*
 * 测试 C99 7.18.2.4 —— Limits of integer types capable of holding object pointers
 * 涉及宏：INTPTR_MIN, INTPTR_MAX, UINTPTR_MAX
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（放在 #if 0 中，仅作展示）。
 *
 * 条款要点（[1]）：
 *   INTPTR_MIN  = -(2^(N-1))          （intptr_t 的最小值）
 *   INTPTR_MAX  =  2^(N-1) - 1        （intptr_t 的最大值）
 *   UINTPTR_MAX =  2^N - 1            （uintptr_t 的最大值）
 *   其中 N 为 intptr_t/uintptr_t 的位宽（通常等于指针宽度）。
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 三个宏必须由 <stdint.h> 定义 */
#ifndef INTPTR_MIN
#error "INTPTR_MIN must be defined by <stdint.h>"
#endif
#ifndef INTPTR_MAX
#error "INTPTR_MAX must be defined by <stdint.h>"
#endif
#ifndef UINTPTR_MAX
#error "UINTPTR_MAX must be defined by <stdint.h>"
#endif

/* [1] 宏必须是整型常量表达式，可用于 #if 预处理条件 */
#if !(INTPTR_MAX > 0)
#error "INTPTR_MAX must be positive"
#endif
#if !(UINTPTR_MAX > 0)
#error "UINTPTR_MAX must be positive"
#endif
#if !(INTPTR_MIN < 0)
#error "INTPTR_MIN must be negative"
#endif

int main(void)
{
    /* [1] 类型存在性：intptr_t / uintptr_t 必须存在 */
    intptr_t  ip = 0;
    uintptr_t up = 0;
    (void)ip; (void)up;

    /* [1] INTPTR_MIN 是 intptr_t 的最小值 */
    assert(INTPTR_MIN == (intptr_t)INTPTR_MIN);
    assert(INTPTR_MIN < 0);

    /* [1] INTPTR_MAX 是 intptr_t 的最大值 */
    assert(INTPTR_MAX == (intptr_t)INTPTR_MAX);
    assert(INTPTR_MAX > 0);

    /* [1] UINTPTR_MAX 是 uintptr_t 的最大值 */
    assert(UINTPTR_MAX == (uintptr_t)UINTPTR_MAX);
    assert(UINTPTR_MAX > 0);

    /* [1] 关系：INTPTR_MAX == UINTPTR_MAX / 2 （当位宽 N 相同时） */
    assert((uintptr_t)INTPTR_MAX == UINTPTR_MAX / 2);

    /* [1] 关系：INTPTR_MIN == -INTPTR_MAX - 1 （二进制补码） */
    assert(INTPTR_MIN == -INTPTR_MAX - 1);

    /* [1] 关系：UINTPTR_MAX == 2 * (uintptr_t)INTPTR_MAX + 1 */
    assert(UINTPTR_MAX == 2u * (uintptr_t)INTPTR_MAX + 1u);

    /* [1] 边界值可被 intptr_t / uintptr_t 表示 */
    {
        intptr_t  minv = INTPTR_MIN;
        intptr_t  maxv = INTPTR_MAX;
        uintptr_t umax = UINTPTR_MAX;
        assert(minv == INTPTR_MIN);
        assert(maxv == INTPTR_MAX);
        assert(umax == UINTPTR_MAX);
    }

    /* [1] 指针可无损转换为 intptr_t 再转回（对象指针） */
    {
        int obj = 42;
        int *p = &obj;
        intptr_t  i = (intptr_t)p;
        int *q = (int *)i;
        assert(q == p);
        assert(*q == 42);
    }

    /* [1] 指针可无损转换为 uintptr_t 再转回 */
    {
        int obj = 7;
        int *p = &obj;
        uintptr_t u = (uintptr_t)p;
        int *q = (int *)u;
        assert(q == p);
        assert(*q == 7);
    }

    /* [1] 打印实际值（便于人工核对） */
    printf("INTPTR_MIN  = %" PRIdPTR "\n", (intptr_t)INTPTR_MIN);
    printf("INTPTR_MAX  = %" PRIdPTR "\n", (intptr_t)INTPTR_MAX);
    printf("UINTPTR_MAX = %" PRIuPTR "\n", (uintptr_t)UINTPTR_MAX);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「INTPTR_MIN 必须是 intptr_t 可表示的最小值」：
 * 若把 INTPTR_MIN 当作无符号量使用，会触发有符号/无符号比较告警，
 * 且语义错误。此处演示对 INTPTR_MIN 取负后与自身比较的非法用法。
 * 期望：gcc -std=c99 -Wall 报出有符号/无符号比较告警或错误。
 */
#include <stdint.h>
int bad1(void) {
    return -INTPTR_MIN;   /* 对最小值取负是 UB，且类型不匹配 */
}

/* 违反约束「UINTPTR_MAX 是 uintptr_t 的最大值」：
 * 试图把 UINTPTR_MAX 赋给 intptr_t 并期望保持正值，
 * 会触发有符号/无符号转换告警。
 * 期望：gcc -std=c99 -Wconversion 报出转换告警。
 */
int bad2(void) {
    intptr_t x = UINTPTR_MAX;   /* 溢出/符号转换 */
    return (int)x;
}

/* 违反约束「INTPTR_MAX 必须为正」：
 * 若把 INTPTR_MAX 与 0 比较并断言其小于 0，逻辑上不可能成立，
 * 静态断言会失败。
 * 期望：编译期 _Static_assert 失败。
 */
_Static_assert(INTPTR_MAX < 0, "INTPTR_MAX must be positive");

/* 违反约束「INTPTR_MIN 必须为负」：
 * 静态断言 INTPTR_MIN >= 0 会失败。
 * 期望：编译期 _Static_assert 失败。
 */
_Static_assert(INTPTR_MIN >= 0, "INTPTR_MIN must be negative");

#endif