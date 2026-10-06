/*
 * 测试 C99 7.21.4.3 —— strcoll 函数
 *
 * 预期行为：
 *   正向测试：包含 <string.h>，调用 strcoll 比较两个字符串，
 *             返回值符号反映在当前 LC_COLLATE 区域设置下的字典序关系；
 *             整个程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数类型错误、缺少原型声明等）
 *             应被编译器拒绝（编译报错），统一放在 #if 0 中。
 *
 * 覆盖段落：
 *   [1] Synopsis：声明 int strcoll(const char *s1, const char *s2);
 *   [2] Description：按当前 locale 的 LC_COLLATE 类别解释两个字符串并比较
 *   [3] Returns：返回 >0 / ==0 / <0 分别表示 s1 大于 / 等于 / 小于 s2
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证头文件 <string.h> 提供了 strcoll 的原型。
 *     通过取函数指针并赋给匹配的原型类型来静态检查签名。 */
static int (*strcoll_proto_check)(const char *, const char *) = strcoll;

/* [3] 返回值符号的辅助判定 */
static int sign_of(int v)
{
    return (v > 0) - (v < 0);
}

int main(void)
{
    /* [1] 原型签名检查：类型必须完全匹配 */
    assert(strcoll_proto_check == strcoll);

    /* 使用 "C" locale，保证行为可预测（LC_COLLATE 为 C 时按字节序） */
    setlocale(LC_COLLATE, "C");

    /* [2][3] 相等字符串：返回值必须等于 0 */
    {
        const char *a = "hello";
        const char *b = "hello";
        int r = strcoll(a, b);
        assert(r == 0);                 /* [3] 相等 -> 0 */
        assert(sign_of(r) == 0);
    }

    /* [2][3] s1 小于 s2：返回值必须小于 0 */
    {
        const char *a = "abc";
        const char *b = "abd";
        int r = strcoll(a, b);
        assert(r < 0);                  /* [3] s1 < s2 -> 负 */
        assert(sign_of(r) == -1);
    }

    /* [2][3] s1 大于 s2：返回值必须大于 0 */
    {
        const char *a = "abd";
        const char *b = "abc";
        int r = strcoll(a, b);
        assert(r > 0);                  /* [3] s1 > s2 -> 正 */
        assert(sign_of(r) == 1);
    }

    /* [2][3] 前缀关系：短串小于长串（C locale 下） */
    {
        const char *a = "abc";
        const char *b = "abcd";
        int r = strcoll(a, b);
        assert(r < 0);
    }

    /* [2][3] 空串与空串相等 */
    {
        int r = strcoll("", "");
        assert(r == 0);
    }

    /* [2][3] 空串小于非空串 */
    {
        int r = strcoll("", "a");
        assert(r < 0);
    }

    /* [2][3] 反对称性：strcoll(a,b) 与 strcoll(b,a) 符号相反 */
    {
        const char *a = "apple";
        const char *b = "banana";
        int rab = strcoll(a, b);
        int rba = strcoll(b, a);
        assert(sign_of(rab) == -sign_of(rba));
        assert(rab < 0 && rba > 0);
    }

    /* [2][3] 传递性（C locale 下按字节序） */
    {
        const char *a = "aaa";
        const char *b = "bbb";
        const char *c = "ccc";
        assert(strcoll(a, b) < 0);
        assert(strcoll(b, c) < 0);
        assert(strcoll(a, c) < 0);
    }

    /* [2] 参数为 const char *：可传入字符串字面量与 const 指针，
     *     且函数不修改字符串内容（此处验证可读性）。 */
    {
        char buf1[] = "xyz";
        char buf2[] = "xyw";
        const char *p1 = buf1;
        const char *p2 = buf2;
        int r = strcoll(p1, p2);
        assert(r > 0);
        /* 内容未被修改 */
        assert(strcmp(buf1, "xyz") == 0);
        assert(strcmp(buf2, "xyw") == 0);
    }

    /* [2] 在非 "C" locale 下也应返回合法符号（不假设具体顺序，
     *     只验证返回值与自身比较为 0，且与反向比较符号相反）。 */
    {
        const char *a = "resume";
        const char *b = "résumé";   /* 含非 ASCII 字节 */
        int rab = strcoll(a, b);
        int rba = strcoll(b, a);
        assert(strcoll(a, a) == 0);
        assert(strcoll(b, b) == 0);
        assert(sign_of(rab) == -sign_of(rba));
    }

    printf("C99 7.21.4.3 strcoll: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 参数类型必须为 const char *」：
 * 传入 int 实参，gcc -std=c99 应报 incompatible type / 参数类型不匹配。 */
#include <string.h>
void bad_arg_type(void)
{
    int x = 1, y = 2;
    strcoll(x, y);              /* 期望报错：int 不能转换为 const char * */
}

/* 违反约束「[1] 参数个数必须为 2」：
 * 少传参数，gcc -std=c99 应报 too few arguments。 */
void bad_too_few_args(void)
{
    strcoll("a");               /* 期望报错：参数太少 */
}

/* 违反约束「[1] 参数个数必须为 2」：
 * 多传参数，gcc -std=c99 应报 too many arguments。 */
void bad_too_many_args(void)
{
    strcoll("a", "b", "c");     /* 期望报错：参数太多 */
}

/* 违反约束「[1] 返回类型为 int，不可作为左值赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报 lvalue required。 */
void bad_assign_to_call(void)
{
    strcoll("a", "b") = 0;      /* 期望报错：非左值不能赋值 */
}

/* 违反约束「[1] 参数为 const char *，不能传入不兼容的指针类型」：
 * 传入 int * 指针，gcc -std=c99 应报 incompatible pointer type。 */
void bad_ptr_type(void)
{
    int arr[4] = {0};
    strcoll(arr, arr);          /* 期望报错：int * 与 const char * 不兼容 */
}

/* 违反约束「[1] 参数为 const char *，不能传入结构体」：
 * 传入结构体实参，gcc -std=c99 应报 incompatible type。 */
struct S { int x; };
void bad_struct_arg(void)
{
    struct S s1, s2;
    strcoll(s1, s2);            /* 期望报错：结构体不能转换为 const char * */
}

#endif /* 负向测试结束 */