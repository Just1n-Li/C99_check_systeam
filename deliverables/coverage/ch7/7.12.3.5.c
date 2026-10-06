/*
 * 测试 C99 7.12.3.5 —— isnormal 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，isnormal(x) 对“正规数”返回非零，
 *             对 0、次正规数、无穷、NaN 返回 0；参数先被转换到其语义类型。
 *             整个程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如未包含 <math.h> 就使用 isnormal、
 *             参数个数错误等）应导致编译报错；这些片段放在 #if 0 中，
 *             保证本文件本身仍可正常编译运行。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：包含 <math.h> 后可使用 isnormal 宏 */
static void test_synopsis(void)
{
    double d = 1.0;
    int r = isnormal(d);          /* [1] 宏调用形式 */
    assert(r != 0);
}

/* [2] Description：判断参数是否为正规数（非零、非次正规、非无穷、非 NaN） */
static void test_normal_values(void)
{
    /* 普通正规数：应返回非零 */
    assert(isnormal(1.0) != 0);
    assert(isnormal(-1.0) != 0);
    assert(isnormal(3.14159265358979) != 0);
    assert(isnormal(DBL_MIN) != 0);      /* DBL_MIN 是最小正规数 */
    assert(isnormal(DBL_MAX) != 0);
    assert(isnormal(1e-300) != 0);
    assert(isnormal(1e300) != 0);
}

static void test_non_normal_values(void)
{
    /* 零：不是正规数 */
    assert(isnormal(0.0) == 0);
    assert(isnormal(-0.0) == 0);

    /* 无穷：不是正规数 */
    assert(isnormal(INFINITY) == 0);
    assert(isnormal(-INFINITY) == 0);

    /* NaN：不是正规数 */
    assert(isnormal(NAN) == 0);

    /* 次正规数（subnormal）：不是正规数 */
    {
        double sub = DBL_MIN / 2.0;      /* 小于最小正规数，为次正规 */
        assert(sub != 0.0);              /* 确认它确实非零 */
        assert(isnormal(sub) == 0);      /* 次正规数不是正规数 */
    }
    {
        float subf = FLT_MIN / 2.0f;
        assert(subf != 0.0f);
        assert(isnormal(subf) == 0);
    }
}

/* [2] 语义类型转换：宽于语义类型的参数先被转换到其语义类型。
 * 这里用 float 参数（语义类型 float）验证基本行为；
 * 用 long double 参数验证宏仍能正确判断。 */
static void test_semantic_type_conversion(void)
{
    float f = 2.0f;
    long double ld = 2.0L;

    assert(isnormal(f) != 0);        /* float 语义类型 */
    assert(isnormal(ld) != 0);       /* long double 语义类型 */

    /* 次正规 float 经语义类型判断仍为非正规 */
    {
        float subf = FLT_MIN / 2.0f;
        assert(isnormal(subf) == 0);
    }
    /* 次正规 long double */
    {
        long double subl = LDBL_MIN / 2.0L;
        assert(subl != 0.0L);
        assert(isnormal(subl) == 0);
    }
}

/* [3] Returns：当且仅当参数为正规值时返回非零 */
static void test_returns_iff(void)
{
    /* 正规 -> 非零 */
    assert(isnormal(42.0) != 0);
    /* 非正规 -> 0 */
    assert(isnormal(0.0) == 0);
    assert(isnormal(INFINITY) == 0);
    assert(isnormal(NAN) == 0);
    assert(isnormal(DBL_MIN / 2.0) == 0);
}

/* 综合：isnormal 与 fpclassify 的一致性检查（仅用条款涉及的概念） */
static void test_consistency(void)
{
    double vals[] = { 1.0, -1.0, 0.0, -0.0, INFINITY, -INFINITY, DBL_MIN, DBL_MIN / 2.0 };
    size_t i;
    for (i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i) {
        int is_norm = isnormal(vals[i]);
        int is_fp_normal = (fpclassify(vals[i]) == FP_NORMAL);
        /* isnormal 为真 <=> fpclassify 为 FP_NORMAL */
        assert((is_norm != 0) == (is_fp_normal != 0));
    }
}

int main(void)
{
    test_synopsis();
    test_normal_values();
    test_non_normal_values();
    test_semantic_type_conversion();
    test_returns_iff();
    test_consistency();

    printf("C99 7.12.3.5 isnormal: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isnormal 是 <math.h> 中声明的宏」：
 * 未包含 <math.h> 就使用 isnormal，gcc -std=c99 应报错
 * （implicit declaration / undeclared identifier）。 */
int no_include_test(void)
{
    return isnormal(1.0);   /* 期望：编译报错，isnormal 未声明 */
}

/* 违反约束「isnormal 为宏，接受一个实浮点参数」：
 * 参数个数错误（0 个或 2 个），应编译报错。 */
int wrong_argc_test(void)
{
    int a = isnormal();          /* 期望：编译报错，参数个数不匹配 */
    int b = isnormal(1.0, 2.0);  /* 期望：编译报错，参数个数不匹配 */
    return a + b;
}

/* 违反约束「参数应为实浮点类型」：
 * 传入结构体类型，应编译报错。 */
struct S { int x; };
int wrong_type_test(void)
{
    struct S s;
    return isnormal(s);   /* 期望：编译报错，参数类型不合法 */
}

#endif /* 负向测试结束 */