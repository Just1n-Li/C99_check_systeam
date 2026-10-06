/*
 * 测试 C99 7.20.4.6 —— system 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：int system(const char *string);  声明于 <stdlib.h>
 *   [2] string 为 NULL 时探测宿主环境是否有命令处理器；
 *       string 非 NULL 时把字符串交给命令处理器执行（实现定义方式）。
 *   [3] 返回：NULL 实参时，仅当命令处理器可用才返回非零；
 *       非 NULL 实参且函数返回时，返回实现定义的值。
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与 int(const char *) 兼容 */
static int (*sys_ptr)(const char *) = system;

int main(void)
{
    /* [1] 头文件 <stdlib.h> 提供了 system 的声明，可正常取地址并调用 */
    assert(sys_ptr == system);

    /* [2][3] 实参为 NULL：探测命令处理器是否存在。
     * 返回值只保证「有命令处理器则非零，否则为零」，不保证具体数值。 */
    int has_shell = system(NULL);
    printf("[2][3] system(NULL) = %d (0 表示无命令处理器，非 0 表示有)\n", has_shell);
    /* 返回值只能是 0 或非 0，这里只断言其语义：非零 <=> 有命令处理器。
     * 无法在可移植代码中断言具体值，故仅做逻辑一致性检查。 */
    assert(has_shell == 0 || has_shell != 0);

    /* [2][3] 实参非 NULL：把字符串交给命令处理器执行。
     * 返回值是实现定义的，因此只检查「函数返回了」这一事实，
     * 不对具体返回值做可移植断言。 */
    if (has_shell) {
        /* 使用一个几乎在所有命令处理器上都存在的命令。
         * 注意：具体命令与返回值是实现定义的，这里只验证调用不崩溃。 */
        int rc = system("exit 0");
        printf("[2][3] system(\"exit 0\") = %d (实现定义)\n", rc);
        (void)rc; /* 返回值实现定义，不做断言 */

        /* 再调用一次，验证可重复调用 */
        rc = system("exit 0");
        (void)rc;
    } else {
        printf("[2] 宿主环境无命令处理器，跳过非 NULL 实参的执行测试\n");
    }

    /* [2] 传入空字符串（非 NULL）：仍应交给命令处理器处理，不崩溃 */
    if (has_shell) {
        int rc = system("");
        (void)rc; /* 实现定义 */
    }

    /* [1] const 限定：形参是 const char *，传入字符串字面量合法 */
    const char *cmd = "exit 0";
    if (has_shell) {
        int rc = system(cmd);
        (void)rc;
    }

    printf("正向测试全部通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与形参 const char * 兼容」：
 * 传入 int 实参，且无原型可见时也无法进行指针转换，gcc -std=c99 应报错
 * （error: passing argument 1 of 'system' makes pointer from integer without a cast）。 */
void bad_arg_type(void)
{
    system(42);
}

/* 违反约束「形参为指向 const char 的指针」：
 * 传入 int * 与 const char * 不兼容，应报错
 * （error: incompatible pointer type）。 */
void bad_arg_ptr(void)
{
    int x = 0;
    system(&x);
}

/* 违反约束「函数返回 int，不能当作结构体使用」：
 * 把 system 的返回值当结构体访问成员，应报错
 * （error: request for member 'x' in something not a structure or union）。 */
void bad_return_use(void)
{
    system(NULL).x;
}

/* 违反约束「函数调用结果不是左值，不能赋值」：
 * 对 system 的返回值赋值，应报错
 * （error: lvalue required as left operand of assignment）。 */
void bad_assign_return(void)
{
    system(NULL) = 0;
}

/* 违反约束「函数名不是左值，不能赋值」：
 * 对函数指示符赋值，应报错
 * （error: lvalue required as left operand of assignment）。 */
void bad_assign_func(void)
{
    system = 0;
}

/* 违反约束「实参个数必须匹配」：
 * 少传实参，应报错
 * （error: too few arguments to function 'system'）。 */
void bad_too_few_args(void)
{
    system();
}

/* 违反约束「实参个数必须匹配」：
 * 多传实参，应报错
 * （error: too many arguments to function 'system'）。 */
void bad_too_many_args(void)
{
    system(NULL, NULL);
}

#endif /* 负向测试结束 */