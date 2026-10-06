/*
 * 测试条款：C99 7.24.4.4.2  wcscoll 函数
 *
 * 预期行为：
 *   正向测试：包含 <wchar.h>，调用 wcscoll 比较宽字符串，返回值符号
 *             正确反映按当前 LC_COLLATE 区域设置解释下的字典序关系；
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（参数类型错误、参数个数错误、未包含头文件
 *             而隐式声明等）应导致编译报错；这些片段放在 #if 0 中，
 *             保证本文件整体仍可编译运行。
 *
 * 覆盖段落：
 *   [1] 概要：声明 int wcscoll(const wchar_t *, const wchar_t *);
 *   [2] 描述：按 LC_COLLATE 解释比较两个宽字符串
 *   [3] 返回值：>0 / ==0 / <0 分别对应 s1 大于 / 等于 / 小于 s2
 */

#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：验证函数原型存在且签名正确。
 *     通过取函数指针并赋给匹配的原型类型来静态检查签名。 */
static int (*wcscoll_proto_check)(const wchar_t *, const wchar_t *) = wcscoll;

int main(void)
{
    /* [1] 概要：函数可被正常调用，返回 int。 */
    int r;

    /* 使用 "C" 区域设置，保证比较结果可预期（按码点/宽字符序）。 */
    setlocale(LC_ALL, "C");

    /* [3] 返回值：相等时返回 0。 */
    r = wcscoll(L"abc", L"abc");
    assert(r == 0);

    /* [3] 返回值：s1 大于 s2 时返回 > 0。 */
    r = wcscoll(L"b", L"a");
    assert(r > 0);

    /* [3] 返回值：s1 小于 s2 时返回 < 0。 */
    r = wcscoll(L"a", L"b");
    assert(r < 0);

    /* [2] 描述：比较的是整个宽字符串（前缀关系）。
     *     "abc" 与 "abcd"：前者是后者的前缀，故前者小于后者。 */
    r = wcscoll(L"abc", L"abcd");
    assert(r < 0);
    r = wcscoll(L"abcd", L"abc");
    assert(r > 0);

    /* [2] 描述：空宽字符串与任意非空宽字符串比较，空串更小。 */
    r = wcscoll(L"", L"a");
    assert(r < 0);
    r = wcscoll(L"a", L"");
    assert(r > 0);
    r = wcscoll(L"", L"");
    assert(r == 0);

    /* [2] 描述：比较结果与 strcmp 在 "C" 区域下对等宽字符一致，
     *     这里用一组多字符样本交叉验证符号一致性。 */
    {
        const wchar_t *a = L"hello";
        const wchar_t *b = L"world";
        int rc = wcscoll(a, b);
        int rs = wcscmp(a, b);
        /* 在 "C" 区域下，wcscoll 与 wcscmp 的符号应一致。 */
        assert((rc > 0) == (rs > 0));
        assert((rc < 0) == (rs < 0));
        assert((rc == 0) == (rs == 0));
    }

    /* [2] 描述：LC_COLLATE 影响解释方式；在 "C" 区域下结果稳定。
     *     这里再次确认区域设置生效后调用不崩溃且符号正确。 */
    {
        const wchar_t *s1 = L"apple";
        const wchar_t *s2 = L"banana";
        assert(wcscoll(s1, s2) < 0);
        assert(wcscoll(s2, s1) > 0);
        assert(wcscoll(s1, s1) == 0);
    }

    /* [1] 概要：函数指针签名检查变量被使用，避免未使用告警。 */
    assert(wcscoll_proto_check == wcscoll);

    printf("wcscoll: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与形参兼容」：
 * wcscoll 的形参为 const wchar_t *，传入 char * 不兼容，
 * gcc -std=c99 应报错（incompatible pointer type / 参数类型不匹配）。 */
#include <wchar.h>
void bad_arg_type(void)
{
    char *s1 = "abc";
    char *s2 = "def";
    wcscoll(s1, s2);   /* 期望报错：char* 不能转换为 const wchar_t* */
}

/* 违反约束「实参个数必须与形参个数一致」：
 * wcscoll 需要 2 个实参，只传 1 个应报错。 */
#include <wchar.h>
void bad_arg_count_few(void)
{
    wcscoll(L"abc");   /* 期望报错：实参太少 */
}

/* 违反约束「实参个数必须与形参个数一致」：
 * 传 3 个实参应报错。 */
#include <wchar.h>
void bad_arg_count_many(void)
{
    wcscoll(L"a", L"b", L"c");   /* 期望报错：实参太多 */
}

/* 违反约束「实参类型必须为指针」：
 * 传入整数常量而非指针，应报错。 */
#include <wchar.h>
void bad_arg_int(void)
{
    wcscoll(1, 2);   /* 期望报错：int 不能转换为 const wchar_t* */
}

/* 违反约束「函数返回值不可作为左值被赋值」：
 * wcscoll 返回 int（非左值），对其赋值应报错。 */
#include <wchar.h>
void bad_assign_return(void)
{
    wcscoll(L"a", L"b") = 5;   /* 期望报错：赋值目标不是左值 */
}

/* 违反约束「未声明标识符不得使用」：
 * 不包含 <wchar.h> 且无原型时，C99 不允许隐式函数声明，
 * 调用 wcscoll 应报错（implicit declaration）。 */
void bad_no_prototype(void)
{
    wcscoll(L"a", L"b");   /* 期望报错：隐式声明（C99 已删除隐式声明） */
}

#endif /* 负向测试结束 */