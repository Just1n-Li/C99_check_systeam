/*
 * 测试条款：C99 7.12.11.2  The nan functions
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明：double nan(const char *tagp); float nanf(const char *tagp);
 *       long double nanl(const char *tagp);
 *   [2] 语义：nan("n-char-sequence") 等价于 strtod("NAN(n-char-sequence)", NULL)；
 *       nan("") 等价于 strtod("NAN()", NULL)；
 *       tagp 不指向 n-char 序列或空串时等价于 strtod("NAN", NULL)；
 *       nanf/nanl 分别等价于 strtof/strtold 的对应调用。
 *   [3] 返回值：返回一个 quiet NaN（若实现支持），内容由 tagp 指示；
 *       若实现不支持 quiet NaN，则返回零。
 *   Forward references: strtod/strtof/strtold (7.20.1.3)
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* 判断一个 double 是否为 NaN（不依赖 C99 的 isnan 宏，避免额外条款依赖） */
static int is_nan_d(double x) { return x != x; }
static int is_nan_f(float x)  { return x != x; }
static int is_nan_l(long double x) { return x != x; }

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型存在且返回类型正确：把返回值赋给对应类型不应有警告/错误 */
    double      d;
    float       f;
    long double l;

    /* [2] nan("n-char-sequence") 等价于 strtod("NAN(n-char-sequence)", NULL) */
    {
        const char *tag = "1234";
        char buf[64];
        double via_nan, via_strtod;

        d = nan(tag);
        /* 构造与条款等价的字符串 "NAN(1234)" 交给 strtod */
        strcpy(buf, "NAN(");
        strcat(buf, tag);
        strcat(buf, ")");
        via_strtod = strtod(buf, (char **)NULL);
        via_nan    = nan(tag);

        /* 两者都应是 NaN；若实现支持 quiet NaN，二者内容一致 */
        assert(is_nan_d(via_nan) == is_nan_d(via_strtod));
        if (is_nan_d(via_nan)) {
            /* 内容由 tagp 指示：同一 tag 应产生相同位模式 */
            assert(memcmp(&via_nan, &via_strtod, sizeof(double)) == 0);
        }
    }

    /* [2] nan("") 等价于 strtod("NAN()", NULL) */
    {
        double via_nan = nan("");
        double via_strtod = strtod("NAN()", (char **)NULL);
        assert(is_nan_d(via_nan) == is_nan_d(via_strtod));
        if (is_nan_d(via_nan)) {
            assert(memcmp(&via_nan, &via_strtod, sizeof(double)) == 0);
        }
    }

    /* [2] tagp 不指向 n-char 序列或空串时，等价于 strtod("NAN", NULL) */
    {
        /* 含非法字符的 tag：不是 n-char 序列 */
        double via_nan = nan("not a valid tag!");
        double via_strtod = strtod("NAN", (char **)NULL);
        assert(is_nan_d(via_nan) == is_nan_d(via_strtod));
        if (is_nan_d(via_nan)) {
            assert(memcmp(&via_nan, &via_strtod, sizeof(double)) == 0);
        }
    }

    /* [2] nanf 等价于 strtof 的对应调用 */
    {
        const char *tag = "42";
        char buf[64];
        float via_nanf, via_strtof;

        strcpy(buf, "NAN(");
        strcat(buf, tag);
        strcat(buf, ")");
        via_strtof = strtof(buf, (char **)NULL);
        via_nanf   = nanf(tag);

        assert(is_nan_f(via_nanf) == is_nan_f(via_strtof));
        if (is_nan_f(via_nanf)) {
            assert(memcmp(&via_nanf, &via_strtof, sizeof(float)) == 0);
        }
    }

    /* [2] nanl 等价于 strtold 的对应调用 */
    {
        const char *tag = "7";
        char buf[64];
        long double via_nanl, via_strtold;

        strcpy(buf, "NAN(");
        strcat(buf, tag);
        strcat(buf, ")");
        via_strtold = strtold(buf, (char **)NULL);
        via_nanl    = nanl(tag);

        assert(is_nan_l(via_nanl) == is_nan_l(via_strtold));
        if (is_nan_l(via_nanl)) {
            assert(memcmp(&via_nanl, &via_strtold, sizeof(long double)) == 0);
        }
    }

    /* [3] 返回值：若实现支持 quiet NaN，则返回 quiet NaN；否则返回零。
     *     这里只断言“要么是 NaN，要么是 0”，两种实现都合法。 */
    {
        d = nan("0");
        f = nanf("0");
        l = nanl("0");

        assert(is_nan_d(d) || d == 0.0);
        assert(is_nan_f(f) || f == 0.0f);
        assert(is_nan_l(l) || l == 0.0L);

        /* 若支持 quiet NaN，则不应是 signaling NaN（quiet NaN 的典型特征：
         * 参与运算不触发异常，且 x != x 为真）。此处仅验证 x != x。 */
        if (is_nan_d(d)) {
            assert(d != d);
        }
    }

    /* [3] 不同 tag 指示不同内容：若实现支持，位模式应可区分（不强制，
     *     因为标准只要求“内容由 tagp 指示”，未要求不同 tag 必不同）。
     *     这里只做弱断言：两者都是 NaN 或都是 0。 */
    {
        double a = nan("aaa");
        double b = nan("bbb");
        assert((is_nan_d(a) && is_nan_d(b)) || (a == 0.0 && b == 0.0));
    }

    printf("C99 7.12.11.2 nan functions: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「nan 的原型为 double nan(const char *tagp)」：
     * 实参类型必须是 const char *（指向字符的指针）。
     * 传入 int 实参，gcc -std=c99 应报错（参数类型不兼容）。 */
    {
        double x = nan(123);   /* 错误：实参应为 const char *，不是 int */
        (void)x;
    }

    /* 违反约束「nan 返回 double」：
     * 不能把返回的 double 直接赋给结构体类型。 */
    {
        struct S { int a; } s;
        s = nan("x");          /* 错误：double 不能赋给 struct S */
        (void)s;
    }

    /* 违反约束「nanf 返回 float」：
     * 不能对返回的 float 取成员（float 不是结构体/联合体）。 */
    {
        float y = nanf("x");
        y.member;              /* 错误：float 无成员 */
    }

    /* 违反约束「nanl 返回 long double」：
     * 不能把 long double 用作数组下标（下标须为整数）。 */
    {
        int arr[4];
        long double idx = nanl("x");
        arr[idx] = 0;          /* 错误：数组下标须为整数类型 */
    }

    /* 违反约束「nan 的参数为 const char *」：
     * 传入 double 实参，类型不兼容。 */
    {
        double x = nan(3.14);  /* 错误：实参应为 const char *，不是 double */
        (void)x;
    }

#endif
}