/*
 * 验证 C99 条款 6.3.1.6：Complex types
 * [1] 当一个复数类型的值转换为另一个复数类型时，其实部和虚部都遵循相应实数类型的转换规则。
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <complex.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 复数类型转换为另一个复数类型，实部和虚部遵循对应实数类型的转换规则 */
    
    /* double _Complex 转 float _Complex：实部和虚部应遵循 double 转 float 的规则 */
    double dr = 1.123456789012345;
    double di = 2.987654321098765;
    double _Complex dc = dr + di * I;
    float _Complex fc = (float _Complex)dc;
    
    /* 验证复数转换后的实部和虚部，与单独进行实数类型转换的结果完全一致 */
    assert(crealf(fc) == (float)dr);
    assert(cimagf(fc) == (float)di);
    
    /* long double _Complex 转 double _Complex：实部和虚部应遵循 long double 转 double 的规则 */
    long double lr = 1.1234567890123456789L;
    long double li = 2.9876543210987654321L;
    long double _Complex lc = lr + li * I;
    double _Complex dc2 = (double _Complex)lc;
    
    assert(creal(dc2) == (double)lr);
    assert(cimag(dc2) == (double)li);
    
    /* long double _Complex 转 float _Complex：实部和虚部应遵循 long double 转 float 的规则 */
    float _Complex fc2 = (float _Complex)lc;
    assert(crealf(fc2) == (float)lr);
    assert(cimagf(fc2) == (float)li);
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 6.3.1.6 条款本身无 Constraints 段落，但复数类型作为算术类型，不能隐式转换为非算术类型。
   以下测试复数类型不能隐式转换为结构体或指针，违反 6.5.16.1 赋值约束。 */

/* 违反约束：复数类型不能隐式转换为结构体类型 */
struct S { int x; };
double _Complex c = 1.0 + 2.0 * I;
struct S s = c; /* gcc -std=c99 应报错：incompatible types when initializing type ‘struct S’ using type ‘double _Complex’ */

/* 违反约束：复数类型不能隐式转换为指针类型 */
int *p = c; /* gcc -std=c99 应报错：incompatible types when initializing type ‘int *’ using type ‘double _Complex’ */

#endif