/*
 * 验证 C99 条款 6.3.1.7 Real and complex
 * 预期行为：
 * - 正向测试：程序编译并运行通过，所有断言成立。
 * - 负向测试：编译器应对违反约束的代码片段报错。
 */

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 实数类型转换为复数类型：实部按对应实数类型规则转换，虚部为正零或无符号零 */
    {
        /* 隐式转换：double -> double _Complex */
        double d = 3.14;
        double _Complex z = d;
        /* 验证实部 */
        assert(creal(z) == 3.14);
        /* 验证虚部为 0.0 */
        assert(cimag(z) == 0.0);
        /* 验证虚部为正零或无符号零：1.0 / 0.0 == INFINITY，若为负零则为 -INFINITY */
        assert(1.0 / cimag(z) == INFINITY);
    }

    {
        /* 强制转换：int -> float _Complex */
        int i = 42;
        float _Complex fc = (float _Complex)i;
        /* 验证实部按整数转浮点数的规则转换 */
        assert(crealf(fc) == 42.0f);
        /* 验证虚部为正零或无符号零 */
        assert(cimagf(fc) == 0.0f);
        assert(1.0f / cimagf(fc) == INFINITY);
    }

    /* [2] 复数类型转换为实数类型：虚部被丢弃，实部按对应实数类型规则转换 */
    {
        /* 隐式转换：double _Complex -> double */
        double _Complex z = 3.0 + 4.0 * I;
        double d = z;
        /* 验证虚部被丢弃，只保留实部 */
        assert(d == 3.0);
    }

    {
        /* 强制转换：double _Complex -> int */
        double _Complex z = 9.5 + 100.0 * I;
        int i = (int)z;
        /* 验证虚部被丢弃，实部按浮点数转整数的规则截断 */
        assert(i == 9);
    }

    printf("All positive tests passed.\n");

    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束：复数类型与实数类型之间的转换仅适用于算术类型。
   将非算术类型（如指针）转换为复数类型，或反向转换，违反了类型转换的约束。
   gcc -std=c99 应报错，例如 "conversion to non-scalar type" 或 "pointer value used where a complex was expected" */

/* 违反约束：不能将指针类型隐式转换为复数类型 */
void test_pointer_to_complex(void) {
    int *p = 0;
    double _Complex c = p; /* 非法：指针不能隐式转换为复数类型 */
}

/* 违反约束：不能将复数类型隐式转换为指针类型 */
void test_complex_to_pointer(void) {
    double _Complex c = 1.0;
    int *p = c; /* 非法：复数类型不能隐式转换为指针类型 */
}

/* 违反约束：不能将结构体强制转换为复数类型 */
struct S { int x; };
void test_struct_to_complex(void) {
    struct S s = {1};
    double _Complex c = (double _Complex)s; /* 非法：结构体不能强制转换为复数类型 */
}

/* 违反约束：不能将复数类型强制转换为结构体 */
void test_complex_to_struct(void) {
    double _Complex c = 1.0;
    struct S s = (struct S)c; /* 非法：复数类型不能强制转换为结构体 */
}
#endif