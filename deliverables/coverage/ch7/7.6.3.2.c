/*
 * 测试 C99 7.6.3.2 —— fesetround 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（放在 #if 0 中，不参与编译）。
 *
 * 覆盖段落：
 *   [1] 头文件 <fenv.h> 与原型 int fesetround(int round);
 *   [2] 建立由参数 round 表示的舍入方向；若参数不等于任何舍入方向宏的值，
 *       则舍入方向不变。
 *   [3] 当且仅当所请求的舍入方向被建立时返回 0。
 *   [4] EXAMPLE：保存、设置、恢复舍入方向；设置失败则报错并中止。
 */

#include <fenv.h>
#include <assert.h>
#include <stdio.h>

/* 若编译器未定义 FE_TONEAREST 等宏，则无法测试；此处假定 C99 实现提供。 */

/* [4] EXAMPLE 的忠实实现：保存、设置、恢复舍入方向 */
static int example_f(int round_dir)
{
    /* [4] #pragma STDC FENV_ACCESS ON */
    int save_round;
    int setround_ok;

    save_round = fegetround();          /* [4] 保存当前舍入方向 */
    setround_ok = fesetround(round_dir);/* [4] 设置请求的舍入方向 */
    assert(setround_ok == 0);           /* [4] 设置失败则中止 */

    /* ... 使用该舍入方向进行计算 ... */

    fesetround(save_round);             /* [4] 恢复原舍入方向 */
    return setround_ok;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：int fesetround(int round); */
    int (*fp)(int) = fesetround;
    assert(fp != NULL);

    /* [2][3] 对每个标准舍入方向宏，设置应成功并返回 0 */
    {
        int r = fesetround(FE_TONEAREST);
        assert(r == 0);                 /* [3] 成功返回 0 */
        assert(fegetround() == FE_TONEAREST);
    }
    {
        int r = fesetround(FE_DOWNWARD);
        assert(r == 0);
        assert(fegetround() == FE_DOWNWARD);
    }
    {
        int r = fesetround(FE_UPWARD);
        assert(r == 0);
        assert(fegetround() == FE_UPWARD);
    }
    {
        int r = fesetround(FE_TOWARDZERO);
        assert(r == 0);
        assert(fegetround() == FE_TOWARDZERO);
    }

    /* [2] 若参数不等于任何舍入方向宏的值，舍入方向不变。
     * 先设定一个已知方向，再用一个非法值调用，方向应保持不变。
     * 注意：非法值的选择需避免与任何舍入方向宏相等。 */
    {
        int before, after, r;
        r = fesetround(FE_TONEAREST);
        assert(r == 0);
        before = fegetround();
        assert(before == FE_TONEAREST);

        /* 构造一个几乎肯定不是合法舍入方向宏的值。
         * 标准未规定非法参数时返回值，但要求方向不变。 */
        r = fesetround(123456789);
        after = fegetround();
        assert(after == before);        /* [2] 舍入方向未被改变 */
        (void)r;                        /* [3] 非法参数时返回值未规定，不检查 */
    }

    /* [4] EXAMPLE：保存、设置、恢复 */
    {
        int saved = fegetround();
        assert(example_f(FE_DOWNWARD) == 0);
        assert(fegetround() == saved);  /* 恢复成功 */
    }

    /* [3] 返回值语义：成功时为 0（上面已多次验证） */
    assert(fesetround(FE_TONEAREST) == 0);

    printf("C99 7.6.3.2 fesetround: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「fesetround 的参数类型为 int」：
     * 传入结构体类型实参，gcc -std=c99 应报错
     * （incompatible type for argument 1 of 'fesetround'）。
     */
    struct S { int x; } s;
    fesetround(s);

    /*
     * 违反约束「fesetround 的参数个数为 1」：
     * 无实参调用，gcc -std=c99 应报错（too few arguments to function 'fesetround'）。
     */
    fesetround();

    /*
     * 违反约束「fesetround 的参数个数为 1」：
     * 传入两个实参，gcc -std=c99 应报错（too many arguments to function 'fesetround'）。
     */
    fesetround(FE_TONEAREST, FE_DOWNWARD);

    /*
     * 违反约束「fesetround 返回 int，不能作为左值赋值」：
     * 对函数调用结果赋值，gcc -std=c99 应报错（lvalue required as left operand of assignment）。
     */
    fesetround(FE_TONEAREST) = 0;

    /*
     * 违反约束「fesetround 的返回类型为 int，不能取地址后解引用赋值」：
     * 对函数返回值取地址，gcc -std=c99 应报错（lvalue required as unary '&' operand）。
     */
    &fesetround(FE_TONEAREST);
#endif
}