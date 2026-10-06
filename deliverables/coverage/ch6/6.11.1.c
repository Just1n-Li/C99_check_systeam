/*
 * 验证 C99 条款：6.11.1 Future language directions - Floating types
 * 预期行为：
 * 正向测试：验证当前标准下的浮点类型（float, double, long double）的基本属性，运行通过。
 * 负向测试：本条款为未来方向声明，无约束条件，故无负向约束测试。
 */

#include <stdio.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 验证当前标准支持的浮点类型及其范围/精度关系 */
    /* 条款说明未来标准可能包含比 long double 更大范围或精度的浮点类型，
       这意味着在当前 C99 标准中，long double 是具有最大范围和精度的标准浮点类型。 */
    
    /* 验证 float, double, long double 类型的存在及大小关系 */
    assert(sizeof(float) <= sizeof(double));
    assert(sizeof(double) <= sizeof(long double));

    /* 验证精度关系：long double 的精度至少不低于 double */
    assert(LDBL_MANT_DIG >= DBL_MANT_DIG);
    assert(DBL_MANT_DIG >= FLT_MANT_DIG);

    /* 验证范围关系：long double 的范围至少不小于 double */
    assert(LDBL_MAX_EXP >= DBL_MAX_EXP);
    assert(DBL_MAX_EXP >= FLT_MAX_EXP);

    printf("6.11.1 正向测试通过：当前标准支持 float, double, long double，且 long double 具有最大或等于的范围与精度。\n");
    
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 6.11.1 条款为未来方向声明，不包含任何约束，因此无负向约束测试。 */
/* 未来标准可能增加比 long double 更大范围或精度的浮点类型，但 C99 标准未定义此类类型，也无相关约束。 */
#endif