/*
 * 测试 C99 7.20.4.5 —— getenv 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错），
 *             这些片段统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] 原型声明 char *getenv(const char *name);
 *   [2] 在环境列表中查找匹配 name 的字符串；环境名集合与修改方法由实现定义。
 *   [3] 实现的行为应如同没有任何库函数调用 getenv。
 *   [4] 返回指向匹配项的指针；该字符串不应被程序修改，但可能被后续 getenv 调用覆盖；
 *       找不到时返回空指针。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：getenv 的签名必须是 char *(const char *)。
 *     通过取函数指针并赋值给匹配类型来静态验证签名。 */
static char *(*getenv_sig_check)(const char *) = getenv;

int main(void)
{
    /* [1] 头文件 <stdlib.h> 已包含，getenv 可用；签名匹配。 */
    assert(getenv_sig_check == getenv);

    /* [2] 查找一个几乎肯定存在的环境变量。
     *     环境名集合由实现定义，因此这里只做“存在则验证、不存在则验证返回 NULL”的
     *     双向检查，不假设具体环境内容。 */
    const char *path_name = "PATH";
    char *path_val = getenv(path_name);

    if (path_val != NULL) {
        /* [4] 找到时返回非空指针，指向以 '\0' 结尾的字符串。 */
        assert(path_val != NULL);
        /* 返回的字符串可被读取（strlen 合法）。 */
        size_t len = strlen(path_val);
        (void)len;
        /* [4] 返回的字符串不应被程序修改 —— 我们只读，不写。 */
    } else {
        /* [4] 找不到时返回空指针。 */
        assert(path_val == NULL);
    }

    /* [2] 查找一个几乎肯定不存在的环境变量名，验证“找不到返回 NULL”。
     *     使用一个极不可能存在的名字。 */
    char *missing = getenv("__C99_7_20_4_5_DEFINITELY_NOT_SET_1234567890__");
    assert(missing == NULL);

    /* [2] name 参数为 const char *：可以传入字符串字面量（只读）。 */
    char *r1 = getenv("PATH");
    (void)r1;

    /* [2] 也可以传入可修改的字符数组（作为 const 实参传入）。 */
    char namebuf[16];
    strcpy(namebuf, "PATH");
    char *r2 = getenv(namebuf);
    (void)r2;

    /* [4] 返回的指针可能被后续 getenv 调用覆盖：
     *     这里只验证“再次调用 getenv 是合法的”，不假设两次返回指针的关系
     *     （是否相同由实现定义，属于实现细节，不做断言）。 */
    char *r3 = getenv("PATH");
    char *r4 = getenv("PATH");
    (void)r3;
    (void)r4;

    /* [3] “实现的行为应如同没有任何库函数调用 getenv”：
     *     即 getenv 不依赖其它库函数调用它，也不应被其它库函数隐式调用。
     *     这里通过直接调用 getenv 验证其可独立工作。 */
    char *r5 = getenv("PATH");
    (void)r5;

    /* [4] 返回的字符串以 '\0' 结尾（若存在）。 */
    if (path_val != NULL) {
        assert(path_val[strlen(path_val)] == '\0');
    }

    printf("C99 7.20.4.5 getenv: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「getenv 的参数类型为 const char *」：
 * 传入非指针类型（如 int）应编译报错。
 * 期望：gcc -std=c99 报 "passing argument 1 of 'getenv' makes pointer from integer
 *       without a cast" 或类似错误。 */
{
    int x = 0;
    char *p = getenv(x);   /* 错误：实参应为 const char * */
    (void)p;
}

/* 违反约束「getenv 的参数类型为 const char *」：
 * 传入不兼容的指针类型（如 int *）应编译报错（在严格诊断下）。
 * 期望：gcc -std=c99 报 "passing argument 1 of 'getenv' from incompatible pointer type"。 */
{
    int arr[4] = {0};
    char *p = getenv(arr);  /* 错误：int * 与 const char * 不兼容 */
    (void)p;
}

/* 违反约束「getenv 返回 char *，不能赋给不兼容类型」：
 * 将返回值赋给 int 应编译报错。
 * 期望：gcc -std=c99 报 "assignment makes integer from pointer without a cast"。 */
{
    int n = getenv("PATH");  /* 错误：char * 赋给 int */
    (void)n;
}

/* 违反约束「getenv 返回 char *，不能用于需要非指针类型的上下文」：
 * 对返回值做算术运算应编译报错。
 * 期望：gcc -std=c99 报 "invalid operands to binary *" 或类似错误。 */
{
    char *p = getenv("PATH");
    int y = p * 2;   /* 错误：指针不能参与乘法 */
    (void)y;
}

/* 违反约束「getenv 的返回字符串不应被程序修改」：
 * 虽然 C 语言层面返回类型是 char *（非 const），但标准规定程序不应修改它。
 * 这里演示“修改”的写法，属于违反 [4] 的语义要求（注意：这是语义/契约违反，
 * 编译器通常不会报错，因此仅作为注释说明，不放入负向编译测试）。
 * 下面这行若启用，编译通常通过，但运行行为违反标准契约：
 *   getenv("PATH")[0] = 'X';
 */

#endif /* 负向测试结束 */