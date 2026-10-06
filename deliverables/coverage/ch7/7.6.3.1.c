/*
 * 测试条款：C99 7.6.3.1 —— fegetround 函数
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，调用 fegetround()，验证其返回值语义
 *             （返回舍入方向宏的值，或负值表示不可确定）。
 *   负向测试：违反约束的代码应编译报错（本条款无显式约束，故负向部分
 *             针对函数原型/头文件使用上的约束，如未包含头文件、参数个数错误等）。
 *
 * 覆盖段落：
 *   [1] 函数原型：int fegetround(void);
 *   [2] 描述：获取当前舍入方向。
 *   [3] 返回值：返回舍入方向宏的值，或负值。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 函数原型：int fegetround(void);
 *     验证可以取函数地址，且类型为 int (*)(void) */
static int (*fp_fegetround)(void) = fegetround;

int main(void)
{
    /* [1] 原型调用：无参数，返回 int */
    int r = fegetround();
    (void)r;

    /* [1] 函数指针类型匹配检查 */
    assert(fp_fegetround == fegetround);

    /* [2] 描述：获取当前舍入方向。
     *     调用 fegetround 应返回当前舍入方向宏的值。 */
    int dir = fegetround();

    /* [3] 返回值：返回舍入方向宏的值，或负值。
     *     若返回非负值，则必须是标准定义的舍入方向宏之一。 */
    if (dir >= 0) {
        assert(dir == FE_TONEAREST ||
               dir == FE_DOWNWARD  ||
               dir == FE_UPWARD    ||
               dir == FE_TOWARDZERO);
    } else {
        /* [3] 负值：表示没有对应宏或当前舍入方向不可确定。
         *     这是允许的返回值，测试通过。 */
        assert(dir < 0);
    }

    /* [2][3] 多次调用应保持一致（在未改变舍入方向的情况下） */
    {
        int dir2 = fegetround();
        assert(dir2 == dir);
    }

    /* [2][3] 改变舍入方向后，fegetround 应反映新的方向（若支持） */
    {
        int saved = fegetround();
        if (saved >= 0) {
            /* 尝试设置一个不同的舍入方向 */
            int target = (saved == FE_TONEAREST) ? FE_DOWNWARD : FE_TONEAREST;
            if (fesetround(target) == 0) {
                int now = fegetround();
                /* 若 fegetround 可确定，应返回刚设置的方向 */
                if (now >= 0) {
                    assert(now == target);
                }
                /* 恢复原方向 */
                fesetround(saved);
                assert(fegetround() == saved || fegetround() < 0);
            }
        }
    }

    printf("C99 7.6.3.1 fegetround: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数原型为 int fegetround(void)」：
 * 传入参数调用 fegetround，参数个数不匹配，gcc -std=c99 应报错。 */
void test_wrong_arg_count(void)
{
    int x = fegetround(1);   /* 错误：fegetround 不接受参数 */
    (void)x;
}

/* 违反约束「函数原型为 int fegetround(void)」：
 * 将返回值赋给不兼容类型（如指针），类型不匹配，应报错或警告。 */
void test_wrong_return_type(void)
{
    char *p = fegetround();  /* 错误：int 不能隐式转换为 char* */
    (void)p;
}

/* 违反约束「使用前需包含 <fenv.h> 声明」：
 * 未包含头文件时调用 fegetround，C99 下隐式声明返回 int，
 * 但若以 -Werror=implicit-function-declaration 编译应报错。
 * 此处演示在无声明情况下取地址赋给不兼容类型。 */
void test_no_declaration(void)
{
    /* 假设未包含 <fenv.h>，fegetround 未声明 */
    double (*bad)(void) = fegetround;  /* 错误：类型不兼容 */
    (void)bad;
}

#endif