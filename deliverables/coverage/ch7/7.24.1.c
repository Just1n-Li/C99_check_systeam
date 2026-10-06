/*
 * 测试条款：C99 7.24.1 <wchar.h> Introduction
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（验证 <wchar.h> 声明的类型、宏、
 *             函数以及相关语义）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错），统一放在 #if 0 中。
 *
 * 覆盖段落：
 *   [1] 头文件声明 4 个类型、1 个 tag、4 个宏、许多函数
 *   [2] wchar_t, size_t, mbstate_t, wint_t, struct tm（不完整类型）
 *   [3] NULL, WCHAR_MIN, WCHAR_MAX, WEOF
 *   [4] 函数分组（I/O、数值转换、通用操作、日期时间、扩展转换）
 *   [5] 重叠对象拷贝为 UB（不作为负向测试，仅注释说明）
 */

#include <wchar.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <wchar.h> 声明了 4 个类型、1 个 tag、4 个宏、许多函数。
 *     这里通过包含头文件并实际使用它们来验证。 */

/* [2] wchar_t 与 size_t 由 7.17 描述（<stddef.h> 中定义）。
 *     验证它们可用且为对象类型。 */
static void test_types_wchar_size_t(void)
{
    wchar_t wc = L'A';
    size_t  sz = sizeof(wchar_t);
    assert(wc == L'A');
    assert(sz >= 1);
}

/* [2] mbstate_t 是「非数组类型的对象类型」，可保存多字节/宽字符转换状态。
 *     验证：它是对象类型（可声明对象、可取地址、可 sizeof），且不是数组类型。 */
static void test_type_mbstate_t(void)
{
    mbstate_t st;                 /* 可声明对象 => 对象类型 */
    mbstate_t *p = &st;           /* 可取地址 */
    size_t s = sizeof(mbstate_t); /* 可 sizeof */
    assert(p == &st);
    assert(s > 0);

    /* 非数组类型：对 mbstate_t 对象做赋值是合法的（数组不可赋值）。 */
    mbstate_t st2;
    st2 = st;                     /* 若为数组类型则此处编译报错 */
    (void)st2;

    /* 用零初始化并用于转换函数。 */
    memset(&st, 0, sizeof st);
    {
        const char *mb = "A";
        wchar_t out[4];
        size_t r = mbrtowc(out, mb, strlen(mb), &st);
        assert(r == 1);
        assert(out[0] == L'A');
    }
}

/* [2] wint_t 是「不受默认实参提升影响的整数类型」，
 *     可保存扩展字符集成员对应的任意值，以及至少一个不对应任何成员的值。
 *     验证：wint_t 是整数类型；默认实参提升后类型不变（即不是 char/short/float）。 */
static void test_type_wint_t(void)
{
    wint_t w = WEOF;
    /* 整数类型：可做整数运算 */
    wint_t w2 = (wint_t)(w + 1);
    (void)w2;

    /* 不受默认实参提升影响：wint_t 的秩不低于 int。
     * 通过比较 sizeof 与 int 来间接验证（wint_t 至少与 int 同宽）。 */
    assert(sizeof(wint_t) >= sizeof(int));

    /* 可保存扩展字符集成员对应的值 */
    wint_t wa = (wint_t)L'A';
    assert(wa == (wint_t)L'A');
}

/* [2] struct tm 被声明为不完整结构类型（内容在 7.23.1 描述）。
 *     验证：可以声明指向它的指针，可以声明它的对象（因为 <wchar.h> 只声明
 *     不完整类型，但 <time.h> 会补全；这里仅验证 tag 存在且可用作指针目标）。 */
static void test_struct_tm_incomplete(void)
{
    struct tm *ptm = NULL;   /* 不完整类型可声明指针 */
    assert(ptm == NULL);
    /* 注意：<wchar.h> 本身只声明不完整类型；完整定义由 <time.h> 提供。
     * 这里不包含 <time.h>，因此不能对 struct tm 取 sizeof 或定义对象。 */
}

/* [3] NULL 宏（由 7.17 描述）。 */
static void test_macro_NULL(void)
{
    void *p = NULL;
    assert(p == NULL);
    assert(NULL == 0);
}

/* [3] WCHAR_MIN 与 WCHAR_MAX（由 7.18.3 描述）。
 *     验证：它们是常量表达式，且 WCHAR_MIN <= WCHAR_MAX。 */
static void test_macro_WCHAR_MIN_MAX(void)
{
    wchar_t lo = WCHAR_MIN;
    wchar_t hi = WCHAR_MAX;
    assert(lo <= hi);
    /* 常量表达式：可用于数组维度（编译期常量）。 */
    {
        char arr[(WCHAR_MAX >= WCHAR_MIN) ? 1 : -1];
        (void)arr;
    }
}

/* [3] WEOF 展开为 wint_t 类型的常量表达式，其值不对应扩展字符集任何成员。
 *     验证：类型为 wint_t；值不等于任何普通宽字符。 */
static void test_macro_WEOF(void)
{
    wint_t eof = WEOF;
    /* 类型为 wint_t：赋值给 wint_t 不产生截断警告（此处仅语义验证）。 */
    assert(eof == WEOF);

    /* 值不对应扩展字符集任何成员：与常见宽字符不同。 */
    assert(WEOF != (wint_t)L'A');
    assert(WEOF != (wint_t)L'\0');

    /* 被若干函数接受并返回以表示文件结束。 */
    {
        wint_t r = fgetwc(stdin);   /* 可能返回 WEOF 或字符 */
        (void)r;
    }
}

/* [4] 函数分组验证：仅验证各组代表性函数可被声明/调用（不深入语义）。 */
static void test_function_groups(void)
{
    /* 组 1：宽字符/多字节字符 I/O */
    {
        wchar_t buf[8];
        /* swprintf 属于宽字符 I/O 组 */
        int n = swprintf(buf, 8, L"%ls", L"hi");
        assert(n == 2);
        assert(buf[0] == L'h' && buf[1] == L'i');
    }

    /* 组 2：宽字符串数值转换 */
    {
        wchar_t *end = NULL;
        long v = wcstol(L"123", &end, 10);
        assert(v == 123);
        assert(end != NULL && *end == L'\0');
    }

    /* 组 3：通用宽字符串操作 */
    {
        wchar_t dst[16];
        wcscpy(dst, L"abc");
        assert(wcscmp(dst, L"abc") == 0);
        assert(wcslen(dst) == 3);
    }

    /* 组 4：宽字符串日期时间转换（wcsftime 需要 struct tm 完整定义，
     * 这里只验证函数可被取地址，不调用）。 */
    {
        size_t (*fp)(wchar_t *, size_t, const wchar_t *, const struct tm *) = wcsftime;
        assert(fp != NULL);
    }

    /* 组 5：多字节/宽字符序列扩展转换 */
    {
        mbstate_t st;
        memset(&st, 0, sizeof st);
        wchar_t wc;
        size_t r = mbrtowc(&wc, "Z", 1, &st);
        assert(r == 1);
        assert(wc == L'Z');
    }
}

/* [5] 重叠对象拷贝为 UB —— 这是未定义行为，不是约束违反，
 *     因此不作为负向测试。此处仅注释说明，不写实际 UB 代码。 */

int main(void)
{
    /* 设置 locale 以便多字节/宽字符转换可用（"C" locale 下 ASCII 也可用）。 */
    setlocale(LC_ALL, "C");

    test_types_wchar_size_t();       /* [2] wchar_t, size_t */
    test_type_mbstate_t();           /* [2] mbstate_t */
    test_type_wint_t();              /* [2] wint_t */
    test_struct_tm_incomplete();     /* [2] struct tm 不完整类型 */
    test_macro_NULL();               /* [3] NULL */
    test_macro_WCHAR_MIN_MAX();      /* [3] WCHAR_MIN, WCHAR_MAX */
    test_macro_WEOF();               /* [3] WEOF */
    test_function_groups();          /* [4] 函数分组 */

    printf("C99 7.24.1 <wchar.h> Introduction: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「struct tm 在 <wchar.h> 中仅为不完整类型」：
 * 对不完整类型取 sizeof 或定义对象应编译报错。
 * 期望：error: invalid application of 'sizeof' to incomplete type 'struct tm'
 *       error: storage size of 'x' isn't known
 * 注意：仅包含 <wchar.h>，不包含 <time.h>。 */
#include <wchar.h>
void bad_incomplete_tm(void)
{
    struct tm x;              /* 不完整类型不能定义对象 */
    size_t s = sizeof(struct tm);  /* 不完整类型不能 sizeof */
    (void)x; (void)s;
}

/* 违反约束「mbstate_t 是非数组的对象类型」：
 * 若把 mbstate_t 当作数组类型使用（如 sizeof 数组元素、下标），
 * 在标准实现中会失败。这里用「对 mbstate_t 对象取下标」来触发错误。
 * 期望：error: subscripted value is neither array nor pointer
 * 说明：标准规定 mbstate_t 不是数组类型，因此下标运算非法。 */
#include <wchar.h>
void bad_mbstate_subscript(void)
{
    mbstate_t st;
    st[0];                    /* mbstate_t 非数组，下标非法 */
}

/* 违反约束「WEOF 是 wint_t 类型的常量表达式」：
 * 试图把 WEOF 用作需要整型常量表达式以外、或类型不匹配的上下文。
 * 这里演示：把 WEOF 用作数组维度（非常量表达式场景下会失败）。
 * 期望：error: variably modified ... at file scope
 * 说明：WEOF 是常量表达式，但若实现中它不是整型常量表达式则此处报错；
 *       标准要求它是常量表达式，故此处应能通过——因此改为验证
 *       「WEOF 不能用作非 wint_t 的窄字符常量」的误用。
 * 更明确的约束违反：把 WEOF 赋给 char 并期望无截断（不报错），
 * 故改用「对 WEOF 取地址并解引用为数组」的非法用法。 */
#include <wchar.h>
void bad_weof_use(void)
{
    /* WEOF 是 wint_t 常量，不能作为数组维度以外的非法用法；
     * 这里演示对宏展开结果做非法取模（非整数类型场景）。
     * 期望：若 WEOF 非整数类型则报错。 */
    int arr[WEOF];            /* 若 WEOF 为负或非整型常量则报错 */
    (void)arr;
}

/* 违反约束「NULL 是空指针常量」：
 * 把 NULL 用作非指针上下文（如算术运算）在严格实现中可能报错。
 * 期望：error: invalid operands to binary + (have 'void *' and 'int')
 * 说明：NULL 展开为 ((void*)0) 或 0；若为 (void*)0，则 + 非法。 */
#include <wchar.h>
void bad_null_arith(void)
{
    void *p = NULL + 1;       /* 对 void* 做算术非法 */
    (void)p;
}

/* 违反约束「wchar_t 是整数类型」：
 * 对 wchar_t 对象做非法操作（如取成员）应报错。
 * 期望：error: request for member 'x' in something not a structure or union */
#include <wchar.h>
void bad_wchar_member(void)
{
    wchar_t wc = L'A';
    wc.x;                     /* wchar_t 非结构体，取成员非法 */
}

/* 违反约束「wint_t 是整数类型」：
 * 对 wint_t 对象做非法操作（如解引用）应报错。
 * 期望：error: invalid type argument of unary '*' (have 'wint_t') */
#include <wchar.h>
void bad_wint_deref(void)
{
    wint_t w = 0;
    *w;                       /* wint_t 非指针，解引用非法 */
}

#endif /* 负向测试结束 */