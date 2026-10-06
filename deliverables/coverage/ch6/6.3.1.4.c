/*
 * 验证 C99 6.3.1.4 Real floating and integer
 * 预期行为：正向测试运行通过，负向测试编译报错（如有）。
 * 本条款主要描述语义，无显式约束段落，故负向测试部分为空。
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 浮点转整数：向零截断（正数） */
    {
        double d = 3.14;
        int i = (int)d;
        assert(i == 3);

        d = 3.99;
        i = (int)d;
        assert(i == 3);
    }

    /* [1] 浮点转整数：向零截断（负数） */
    {
        double d = -3.14;
        int i = (int)d;
        assert(i == -3);

        d = -3.99;
        i = (int)d;
        assert(i == -3);
    }

    /* [1] 浮点转整数：零 */
    {
        double d = 0.0;
        int i = (int)d;
        assert(i == 0);

        d = -0.0;
        i = (int)d;
        assert(i == 0);
    }

    /* [1] 浮点转整数：float 类型 */
    {
        float f = 5.5f;
        int i = (int)f;
        assert(i == 5);

        f = -5.5f;
        i = (int)f;
        assert(i == -5);
    }

    /* [2] 整数转浮点：精确表示 */
    {
        int i = 123;
        double d = (double)i;
        assert(d == 123.0);

        i = -456;
        float f = (float)i;
        assert(f == -456.0f);
    }

    /* [2] 整数转浮点：大整数精确表示（假设 double 精度足够） */
    {
        long li = 123456789L;
        double d = (double)li;
        assert(d == 123456789.0);
    }

    /* 脚注50：浮点转 unsigned，范围 (-1, Utype_MAX+1) */
    /* -0.5 截断为 0，在 unsigned 范围内 */
    {
        double d = -0.5;
        unsigned u = (unsigned)d;
        assert(u == 0U);

        d = 0.5;
        u = (unsigned)d;
        assert(u == 0U);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 6.3.1.4 条款本身没有定义 Constraints 段落，因此没有负向测试。
 * 该条款描述的是类型转换的语义，未定义行为（如超出范围）不属于约束。
 */
#endif