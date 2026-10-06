/*
 * 验证 C99 6.3.1 Arithmetic operands（算术操作数）
 *
 * 6.3.1 是一个章节标题，统辖 6.3.1.1~6.3.1.8 各子条款，
 * 主题为：哪些类型可作为算术操作数、算术操作数之间的转换。
 *
 * 预期行为：
 *   - 正向测试：能编译并运行通过，所有 assert 成立。
 *   - 负向测试：违反「操作数必须为算术类型」等约束，编译器应报错。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 整数类型作为算术操作数（6.3.1.1） */
    {
        int a = 7, b = 3;
        assert(a + b == 10);
        assert(a - b == 4);
        assert(a * b == 21);
        assert(a / b == 2);
        assert(a % b == 1);
        assert(-a == -7);
        assert(+a == 7);
        assert(~a == ~7);
    }

    /* [1] 实浮点类型作为算术操作数（6.3.1.5/6.3.1.8） */
    {
        double x = 2.5, y = 0.5;
        assert(x + y == 3.0);
        assert(x - y == 2.0);
        assert(x * y == 1.25);
        assert(x / y == 5.0);
        assert(-x == -2.5);
        assert(+x == 2.5);

        float f = 1.5f;
        assert(f + 1.0f == 2.5f);
    }

    /* [1] char/short 经整数提升后参与算术（6.3.1.1） */
    {
        char c = 'A';
        short s = 10;
        /* 提升为 int 后运算，结果类型为 int */
        assert(sizeof(c + c) == sizeof(int));
        assert(sizeof(s + s) == sizeof(int));
        assert(c + 1 == 'B');
        assert(s + 5 == 15);
        assert(c - 'A' == 0);
    }

    /* [1] _Bool 作为算术操作数（6.3.1.1/6.3.1.2） */
    {
        _Bool t = 1, f = 0;
        assert(t + t == 2);
        assert(t * f == 0);
        assert(t - f == 1);
        assert(t && f == 0);
        assert(t || f == 1);
    }

    /* [1] 枚举类型是整数类型，可作为算术操作数（6.3.1.1） */
    {
        enum E { EA, EB, EC } e = EB;
        assert(e + 1 == EC);
        assert(e - 1 == EA);
        assert(sizeof(e + 0) == sizeof(int)); /* 枚举提升为 int */
    }

    /* [1] 整数与浮点混合：整数转换为浮点（6.3.1.4/6.3.1.8） */
    {
        int i = 3;
        double d = 0.5;
        assert(i + d == 3.5);
        assert(i * d == 1.5);
        assert(sizeof(i + d) == sizeof(double));

        long lg = 5L;
        float fl = 2.0f;
        assert(lg + fl == 7.0f || lg + fl == 7.0); /* 提升到 float/double */
    }

    /* [1] 通常算术转换：不同整数类型（6.3.1.8） */
    {
        int i = 1;
        unsigned u = 2u;
        /* int 与 unsigned 运算，int 转为 unsigned */
        assert(i + u == 3u);
        assert(sizeof(i + u) == sizeof(unsigned));

        long l = 1L;
        unsigned long ul = 2UL;
        assert(l + ul == 3UL);
    }

    /* [1] 复数类型作为算术操作数（6.3.1.6/6.3.1.7，若实现支持） */
#ifdef __STDC_IEC_559_COMPLEX__
    {
        double complex z1 = 1.0 + 2.0 * I;
        double complex z2 = 3.0 + 4.0 * I;
        double complex sum = z1 + z2;
        double complex prod = z1 * z2;
        assert(creal(sum) == 4.0);
        assert(cimag(sum) == 6.0);
        /* (1+2i)(3+4i) = 3+4i+6i+8i^2 = 3-8 + 10i = -5+10i */
        assert(creal(prod) == -5.0);
        assert(cimag(prod) == 10.0);

        /* 复数与实数混合：实数转为复数（6.3.1.8） */
        double complex z3 = z1 + 3.0;
        assert(creal(z3) == 4.0);
        assert(cimag(z3) == 2.0);
    }
#endif

    /* [1] 位运算操作数也要求算术类型（整数类型） */
    {
        unsigned a = 0xF0, b = 0x0F;
        assert((a & b) == 0x00);
        assert((a | b) == 0xFF);
        assert((a ^ b) == 0xFF);
        assert((a << 1) == 0x1E0u);
        assert((a >> 4) == 0x0Fu);
    }

    /* [1] 移位操作数：左右操作数均为算术类型 */
    {
        int v = 8;
        assert((v << 2) == 32);
        assert((v >> 1) == 4);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* ---- 违反约束：结构体不是算术类型，不能作为算术操作数 ---- */
struct S { int x; } s1, s2;
s1 + s2;   /* 期望 gcc -std=c99 报错：invalid operands to binary + */
s1 * s2;   /* 期望报错：invalid operands to binary * */
-s1;       /* 期望报错：wrong type argument to unary minus */
~s1;       /* 期望报错：wrong type argument to bit-complement */

/* ---- 违反约束：联合不是算术类型 ---- */
union U { int x; double y; } u1, u2;
u1 - u2;   /* 期望报错：invalid operands to binary - */
u1 / u2;   /* 期望报错：invalid operands to binary / */

/* ---- 违反约束：数组不是算术类型（数组名在算术运算中不作为算术操作数） ---- */
int arr[3] = {1,2,3};
arr * 2;   /* 期望报错：invalid operands to binary *（arr 衰退为指针，指针*int 非法） */
arr % 3;   /* 期望报错：invalid operands to binary % */

/* ---- 违反约束：void 不是算术类型，不能作为算术操作数 ---- */
void vd_func(void);
/* void 表达式不能参与算术 */
(void) + 1;  /* 期望报错：void 表达式不能作为算术操作数 */

/* ---- 违反约束：函数类型不是算术类型 ---- */
vd_func + vd_func;  /* 期望报错：函数指针相加非法（操作数非算术类型） */

/* ---- 违反约束：对非算术类型取逻辑反以外的单目算术 ---- */
struct S2 { int x; } sx;
+sx;       /* 期望报错：wrong type argument to unary plus */

#endif