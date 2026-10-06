/*
 * 测试目标：C99 7.24.4.2.2  wcsncpy 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>
 *   [2] 最多复制 n 个宽字符；遇到空宽字符后不再复制
 *   [3] 若 s2 指向的宽字符串短于 n，则用空宽字符填充至共 n 个
 *   [4] 返回 s1 的值
 *   脚注 297：若 s2 前 n 个宽字符内无空宽字符，则结果不以空宽字符结尾
 */

#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：wchar_t *wcsncpy(wchar_t * restrict, const wchar_t * restrict, size_t) */
    {
        wchar_t dst[16];
        const wchar_t *src = L"abc";
        wchar_t *ret = wcsncpy(dst, src, 3);
        assert(ret == dst);                 /* [4] 返回 s1 */
        assert(dst[0] == L'a' && dst[1] == L'b' && dst[2] == L'c');
    }

    /* [2] 最多复制 n 个宽字符：n 小于源串长度时只复制前 n 个 */
    {
        wchar_t dst[16];
        const wchar_t *src = L"abcdef";
        wchar_t *ret = wcsncpy(dst, src, 3);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'a');
        assert(dst[1] == L'b');
        assert(dst[2] == L'c');
        /* 脚注 297：前 n 个字符内无空宽字符，结果不以空宽字符结尾 */
        assert(dst[3] != L'\0');            /* 未被写入，保持原值（此处未初始化，仅演示语义） */
    }

    /* [2] 遇到空宽字符后不再复制：n 大于源串长度时，空宽字符之后的内容不被复制 */
    {
        wchar_t dst[16];
        const wchar_t *src = L"ab";
        wchar_t *ret = wcsncpy(dst, src, 5);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'a');
        assert(dst[1] == L'b');
        /* [3] 源串短于 n，用空宽字符填充至共 n 个 */
        assert(dst[2] == L'\0');
        assert(dst[3] == L'\0');
        assert(dst[4] == L'\0');
    }

    /* [3] 源串恰好等于 n 个宽字符（含结尾空宽字符） */
    {
        wchar_t dst[16];
        const wchar_t *src = L"abc";        /* 4 个宽字符含结尾 L'\0' */
        wchar_t *ret = wcsncpy(dst, src, 4);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'a');
        assert(dst[1] == L'b');
        assert(dst[2] == L'c');
        assert(dst[3] == L'\0');
    }

    /* [3] 源串短于 n，填充空宽字符，验证填充数量精确为 n */
    {
        wchar_t dst[16];
        const wchar_t *src = L"x";
        wchar_t *ret = wcsncpy(dst, src, 6);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'x');
        assert(dst[1] == L'\0');
        assert(dst[2] == L'\0');
        assert(dst[3] == L'\0');
        assert(dst[4] == L'\0');
        assert(dst[5] == L'\0');
    }

    /* [2] n == 0：不复制任何字符，返回 s1 */
    {
        wchar_t dst[4] = { L'Z', L'Z', L'Z', L'Z' };
        const wchar_t *src = L"abc";
        wchar_t *ret = wcsncpy(dst, src, 0);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'Z');             /* 未被修改 */
        assert(dst[1] == L'Z');
        assert(dst[2] == L'Z');
        assert(dst[3] == L'Z');
    }

    /* [2] 源串为空宽字符串：全部填充空宽字符 */
    {
        wchar_t dst[8];
        const wchar_t *src = L"";
        wchar_t *ret = wcsncpy(dst, src, 4);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'\0');
        assert(dst[1] == L'\0');
        assert(dst[2] == L'\0');
        assert(dst[3] == L'\0');
    }

    /* [4] 返回值就是 s1 本身（指针相等） */
    {
        wchar_t dst[8];
        const wchar_t *src = L"hello";
        assert(wcsncpy(dst, src, 5) == dst);
    }

    /* [2] 源串中间含空宽字符：空宽字符之后的内容不被复制 */
    {
        wchar_t dst[8];
        const wchar_t src[6] = { L'a', L'b', L'\0', L'c', L'd', L'\0' };
        wchar_t *ret = wcsncpy(dst, src, 5);
        assert(ret == dst);                 /* [4] */
        assert(dst[0] == L'a');
        assert(dst[1] == L'b');
        assert(dst[2] == L'\0');
        /* [3] 源串（作为宽字符串）短于 n，剩余位置填充空宽字符 */
        assert(dst[3] == L'\0');
        assert(dst[4] == L'\0');
    }

    printf("All positive tests for C99 7.24.4.2.2 wcsncpy passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「实参类型必须与原型匹配」：
     * wcsncpy 的第一个参数类型为 wchar_t *，传入 int * 应报错。
     * 期望：gcc -std=c99 报 incompatible pointer type 类错误。
     */
    {
        int idst[8];
        const wchar_t *src = L"abc";
        wcsncpy(idst, src, 3);              /* 错误：int* 不能赋给 wchar_t* 形参 */
    }

    /*
     * 违反约束「实参类型必须与原型匹配」：
     * 第二个参数类型为 const wchar_t *，传入 char * 应报错。
     * 期望：gcc -std=c99 报 incompatible pointer type 类错误。
     */
    {
        wchar_t dst[8];
        const char *src = "abc";
        wcsncpy(dst, src, 3);               /* 错误：char* 不能赋给 const wchar_t* 形参 */
    }

    /*
     * 违反约束「实参个数必须与原型一致」：
     * wcsncpy 需要 3 个实参，只传 2 个应报错。
     * 期望：gcc -std=c99 报 too few arguments 类错误。
     */
    {
        wchar_t dst[8];
        const wchar_t *src = L"abc";
        wcsncpy(dst, src);                  /* 错误：缺少第三个实参 n */
    }

    /*
     * 违反约束「实参个数必须与原型一致」：
     * 传入 4 个实参应报错。
     * 期望：gcc -std=c99 报 too many arguments 类错误。
     */
    {
        wchar_t dst[8];
        const wchar_t *src = L"abc";
        wcsncpy(dst, src, 3, 0);            /* 错误：实参过多 */
    }

    /*
     * 违反约束「第三个实参必须为整数类型（size_t）」：
     * 传入浮点常量应报错。
     * 期望：gcc -std=c99 报 incompatible type for argument 3 类错误。
     */
    {
        wchar_t dst[8];
        const wchar_t *src = L"abc";
        wcsncpy(dst, src, 3.0);             /* 错误：double 不能作为 size_t 实参 */
    }

    /*
     * 违反约束「第一个实参必须为可修改左值（wchar_t *）」：
     * 传入 const wchar_t * 应报错（丢弃 const 限定）。
     * 期望：gcc -std=c99 报 discards qualifiers 类错误。
     */
    {
        const wchar_t cdst[8] = { 0 };
        const wchar_t *src = L"abc";
        wcsncpy(cdst, src, 3);              /* 错误：const wchar_t* 不能赋给 wchar_t* 形参 */
    }

    /*
     * 违反约束「第一个实参必须为可修改左值」：
     * 传入字符串字面量（类型为 wchar_t[N]，但不可修改）应报错。
     * 期望：gcc -std=c99 报 discards qualifiers 类错误。
     */
    {
        const wchar_t *src = L"abc";
        wcsncpy(L"literal", src, 3);        /* 错误：字面量不可修改 */
    }
#endif

    return 0;
}