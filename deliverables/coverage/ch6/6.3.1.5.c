/*
 * 验证 C99 条款 6.3.1.5 Real floating types
 * 预期行为：正向测试运行通过，负向测试编译报错（本条款无约束，故无负向测试）
 */

#include <stdio.h>
#include <assert.h>
#include <float.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] float 提升到 double 或 long double，值不变 */
    float f = 1.5f;
    double d = (double)f;
    long double ld = (long double)f;
    assert(f == d); /* f 在比较时提升为 double */
    assert(f == ld); /* f 在比较时提升为 long double */

    /* [1] double 提升到 long double，值不变 */
    double d2 = 2.5;
    long double ld2 = (long double)d2;
    assert(d2 == ld2); /* d2 在比较时提升为 long double */

    /* [2] double 降级到 float，能精确表示时值不变 */
    double d_exact = 3.5;
    float f_exact = (float)d_exact;
    assert(d_exact == f_exact); /* f_exact 提升回 double 比较 */

    /* [2] long double 降级到 double 或 float，能精确表示时值不变 */
    long double ld_exact = 4.5;
    double d_from_ld = (double)ld_exact;
    float f_from_ld = (float)ld_exact;
    assert(ld_exact == d_from_ld);
    assert(ld_exact == f_from_ld);

    /* [2] 显式转换为其自身类型（即使可能有更高精度），值不变 */
    double d_self = 5.5;
    double d_self_cast = (double)d_self;
    assert(d_self == d_self_cast);

    /* [2] 不能精确表示的值降级，结果为最近的高位或低位可表示值（实现定义） */
    /* 0.1 在二进制浮点数中无法精确表示，降级到 float 后会丢失精度 */
    double d_inexact = 0.1;
    float f_inexact = (float)d_inexact;
    /* 验证精度丢失：降级后再提升回 double，值不再等于原 double 值 */
    assert(d_inexact != (double)f_inexact);
    /* 打印实现定义的舍入结果 */
    printf("d_inexact = %.17f, f_inexact = %.9f\n", d_inexact, f_inexact);

    /* [2] 超出范围的转换是未定义行为，此处不进行测试以避免 UB */

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    /* 本条款（6.3.1.5）不包含 Constraints 段落，因此无负向测试 */
#if 0
/* 6.3.1.5 无约束，此处留空 */
#endif

    return 0;
}