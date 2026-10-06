/*
 * 测试条款：C99 7.9 Alternative spellings <iso646.h>
 *
 * 预期行为：
 *   正向测试：包含 <iso646.h> 后，11 个宏 and, and_eq, bitand, bitor, compl,
 *             not, not_eq, or, or_eq, xor, xor_eq 应分别展开为
 *             &&, &=, &, |, ~, !, !=, ||, |=, ^, ^=，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段（例如把宏当作标识符声明、对宏展开结果赋值等）
 *             应导致编译报错，统一放在 #if 0 ... #endif 中。
 */

#include <stdio.h>
#include <assert.h>
#include <iso646.h>   /* [1] 头文件定义 11 个宏 */

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] and -> && */
    {
        int a = 1, b = 0;
        int r = (a and b);          /* 等价于 a && b */
        assert(r == (a && b));
        assert(r == 0);
        r = (a and 1);
        assert(r == 1);
    }

    /* [1] and_eq -> &= */
    {
        int x = 0xF0;
        x and_eq 0x3C;              /* 等价于 x &= 0x3C */
        assert(x == (0xF0 & 0x3C));
        assert(x == 0x30);
    }

    /* [1] bitand -> & */
    {
        int x = 0xF0, y = 0x3C;
        int r = (x bitand y);       /* 等价于 x & y */
        assert(r == (x & y));
        assert(r == 0x30);
    }

    /* [1] bitor -> | */
    {
        int x = 0xF0, y = 0x0F;
        int r = (x bitor y);        /* 等价于 x | y */
        assert(r == (x | y));
        assert(r == 0xFF);
    }

    /* [1] compl -> ~ */
    {
        unsigned int x = 0u;
        unsigned int r = compl x;   /* 等价于 ~x */
        assert(r == ~x);
        assert(r == 0xFFFFFFFFu);
    }

    /* [1] not -> ! */
    {
        int a = 0;
        int r = (not a);            /* 等价于 !a */
        assert(r == !a);
        assert(r == 1);
        r = (not 5);
        assert(r == 0);
    }

    /* [1] not_eq -> != */
    {
        int a = 3, b = 4;
        int r = (a not_eq b);       /* 等价于 a != b */
        assert(r == (a != b));
        assert(r == 1);
        r = (a not_eq 3);
        assert(r == 0);
    }

    /* [1] or -> || */
    {
        int a = 0, b = 1;
        int r = (a or b);           /* 等价于 a || b */
        assert(r == (a || b));
        assert(r == 1);
        r = (0 or 0);
        assert(r == 0);
    }

    /* [1] or_eq -> |= */
    {
        int x = 0xF0;
        x or_eq 0x0F;               /* 等价于 x |= 0x0F */
        assert(x == (0xF0 | 0x0F));
        assert(x == 0xFF);
    }

    /* [1] xor -> ^ */
    {
        int x = 0xF0, y = 0xFF;
        int r = (x xor y);          /* 等价于 x ^ y */
        assert(r == (x ^ y));
        assert(r == 0x0F);
    }

    /* [1] xor_eq -> ^= */
    {
        int x = 0xF0;
        x xor_eq 0xFF;              /* 等价于 x ^= 0xFF */
        assert(x == (0xF0 ^ 0xFF));
        assert(x == 0x0F);
    }

    /* [1] 组合使用：验证宏可像对应记号一样参与复杂表达式 */
    {
        int a = 1, b = 0, c = 1;
        int r = (a and (b or c));   /* a && (b || c) */
        assert(r == (a && (b || c)));
        assert(r == 1);

        int x = 0x0F;
        x or_eq (0x10 bitand 0x30); /* x |= (0x10 & 0x30) */
        assert(x == (0x0F | (0x10 & 0x30)));
        assert(x == 0x1F);

        int y = 0xAA;
        y and_eq (compl 0x00);      /* y &= ~0x00 */
        assert(y == (0xAA & ~0x00));
        assert(y == 0xAA);
    }

    /* [1] 宏展开后与原生记号语义完全一致（逐位比较） */
    {
        unsigned int v = 0x12345678u;
        assert((v bitand 0xFFu) == (v & 0xFFu));
        assert((v bitor  0xFFu) == (v | 0xFFu));
        assert((v xor   0xFFu) == (v ^ 0xFFu));
        assert((compl v)        == (~v));
        assert((not v)          == (!v));
        assert((v not_eq 0u)    == (v != 0u));
    }

    printf("C99 7.9 <iso646.h> positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束：<iso646.h> 中的 and 等是「宏名」，不是关键字。
     * 宏名在预处理阶段被替换为记号，因此不能作为普通标识符来声明变量。
     * 期望：gcc -std=c99 报错（and 被展开为 &&，导致语法错误）。
     */
    int and = 1;            /* 错误：and 展开为 &&，非法声明 */

    /*
     * 违反约束：not 是宏，展开为 !，不能用作函数名或变量名。
     * 期望：编译报错。
     */
    int not = 0;            /* 错误：not 展开为 ! */

    /*
     * 违反约束：宏展开结果 && 是逻辑运算符，其操作数必须是标量类型；
     * 结构体不是标量类型，不能作为 && 的操作数。
     * 期望：编译报错（invalid operands to binary &&）。
     */
    struct S { int x; } s1, s2;
    int bad1 = (s1 and s2); /* 错误：结构体不能用于 && */

    /*
     * 违反约束：compl 展开为 ~，按位取反的操作数必须是整型；
     * 浮点类型不能作为 ~ 的操作数。
     * 期望：编译报错（wrong type argument to bit-complement）。
     */
    double d = 1.0;
    double bad2 = compl d;  /* 错误：~ 不能作用于 double */

    /*
     * 违反约束：and_eq 展开为 &=，左操作数必须是可修改的左值；
     * 对常量赋值违反约束。
     * 期望：编译报错（lvalue required as left operand of assignment）。
     */
    1 and_eq 2;             /* 错误：1 &= 2，左操作数不是左值 */

    /*
     * 违反约束：xor_eq 展开为 ^=，左操作数必须是可修改的左值；
     * 对强制转换结果赋值违反约束。
     * 期望：编译报错。
     */
    (int)0 xor_eq 1;        /* 错误：(int)0 ^= 1，左操作数不是左值 */

    /*
     * 违反约束：bitor 展开为 |，操作数必须是整型；
     * 指针不能作为 | 的操作数。
     * 期望：编译报错。
     */
    int *p = 0;
    int bad3 = (p bitor 1); /* 错误：指针不能用于 | */
#endif

    return 0;
}