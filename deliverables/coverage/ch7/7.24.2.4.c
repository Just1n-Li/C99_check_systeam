/*
 * 测试 C99 7.24.2.4 —— swscanf 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错（此处被屏蔽以保证文件可编译）。
 *
 * 覆盖段落：
 *   [1] 原型/头文件 <wchar.h>，restrict 限定，可变参数 ...
 *   [2] 等价于 fwscanf，但输入来自宽字符串；到达宽字符串末尾等价于 fwscanf 遇到 EOF
 *   [3] 返回值：任何转换前发生输入失败返回 EOF；否则返回成功赋值的输入项数（可少于提供数，甚至为 0）
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证签名可用（restrict 与 ... 不参与类型匹配） */
static int (*swscanf_ptr)(const wchar_t *restrict, const wchar_t *restrict, ...) = swscanf;

int main(void)
{
    /* 使用 C locale 以便宽字符与基本转换稳定工作 */
    setlocale(LC_ALL, "C");

    /* [1] 函数指针非空，说明 <wchar.h> 中声明了 swscanf */
    assert(swscanf_ptr != NULL);

    /* [2] 基本用法：从宽字符串读取整数，等价于 fwscanf 但源为宽字符串 */
    {
        const wchar_t *s = L"42 100";
        int a = 0, b = 0;
        int n = swscanf(s, L"%d %d", &a, &b);
        assert(n == 2);          /* [3] 成功赋值 2 项 */
        assert(a == 42);
        assert(b == 100);
    }

    /* [2] 到达宽字符串末尾等价于 fwscanf 遇到 EOF：
     *     在已耗尽输入后再读取，应返回 EOF（输入失败，未做任何转换） */
    {
        const wchar_t *s = L"";
        int a = 0;
        int n = swscanf(s, L"%d", &a);
        assert(n == EOF);        /* [3] 任何转换前输入失败 -> EOF */
    }

    /* [2][3] 部分匹配：先成功一项，随后到达末尾，返回已赋值项数 */
    {
        const wchar_t *s = L"7";
        int a = 0, b = 0;
        int n = swscanf(s, L"%d %d", &a, &b);
        assert(n == 1);          /* [3] 少于提供项数 */
        assert(a == 7);
    }

    /* [3] 早期匹配失败：第一个转换就失败，返回 0（不是 EOF，因为未发生输入失败） */
    {
        const wchar_t *s = L"abc";
        int a = 0;
        int n = swscanf(s, L"%d", &a);
        assert(n == 0);          /* [3] 早期匹配失败 -> 0 */
    }

    /* [3] 早期匹配失败后，后续转换不再进行 */
    {
        const wchar_t *s = L"xyz 5";
        int a = 0, b = 0;
        int n = swscanf(s, L"%d %d", &a, &b);
        assert(n == 0);
    }

    /* [2] 混合转换：整数、浮点、字符串、字符 */
    {
        const wchar_t *s = L"12 3.5 hello Z";
        int i = 0;
        double d = 0.0;
        wchar_t word[16] = {0};
        wchar_t c = 0;
        int n = swscanf(s, L"%d %lf %ls %lc", &i, &d, word, &c);
        assert(n == 4);
        assert(i == 12);
        assert(d > 3.49 && d < 3.51);
        assert(wcscmp(word, L"hello") == 0);
        assert(c == L'Z');
    }

    /* [2] 宽度限制与字符集扫描 */
    {
        const wchar_t *s = L"12345";
        wchar_t buf[4] = {0};
        int n = swscanf(s, L"%3ls", buf);
        assert(n == 1);
        assert(wcscmp(buf, L"123") == 0);
    }

    /* [2] 字面量匹配失败：格式中的字面字符不匹配 -> 匹配失败 */
    {
        const wchar_t *s = L"a=1";
        int v = 0;
        int n = swscanf(s, L"a=%d", &v);
        assert(n == 1);
        assert(v == 1);

        const wchar_t *s2 = L"b=1";
        int v2 = 0;
        int n2 = swscanf(s2, L"a=%d", &v2);
        assert(n2 == 0);         /* 字面量 'a' 不匹配 -> 早期匹配失败 */
    }

    /* [2] 抑制赋值（*）：不计入返回值 */
    {
        const wchar_t *s = L"10 20";
        int b = 0;
        int n = swscanf(s, L"%*d %d", &b);
        assert(n == 1);          /* 抑制的项不计入 */
        assert(b == 20);
    }

    /* [2] 到达末尾（EOF 等价）在部分转换后：返回已赋值项数 */
    {
        const wchar_t *s = L"5 ";
        int a = 0, b = 0;
        int n = swscanf(s, L"%d %d", &a, &b);
        assert(n == 1);
        assert(a == 5);
    }

    /* [3] 空格式串：不进行任何转换，返回 0 */
    {
        const wchar_t *s = L"anything";
        int n = swscanf(s, L"");
        assert(n == 0);
    }

    printf("All positive tests for C99 7.24.2.4 (swscanf) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「swscanf 的第一个参数类型为 const wchar_t *」：
 * 传入窄字符串（char *）应报错（类型不兼容）。
 * 期望：gcc -std=c99 报 "incompatible pointer type" 或类似错误。 */
#include <wchar.h>
void neg1(void) {
    char narrow[] = "42";
    int a;
    swscanf(narrow, L"%d", &a);   /* 错误：第一个实参应为 const wchar_t * */
}

/* 违反约束「swscanf 的第二个参数类型为 const wchar_t *」：
 * 格式串传入窄字符串应报错。
 * 期望：gcc -std=c99 报 "incompatible pointer type"。 */
void neg2(void) {
    const wchar_t *s = L"42";
    int a;
    swscanf(s, "%d", &a);         /* 错误：格式串应为 const wchar_t * */
}

/* 违反约束「swscanf 需要 <wchar.h> 中的声明」：
 * 若未包含 <wchar.h>，在 C99 中隐式函数声明被禁止（约束违反）。
 * 期望：gcc -std=c99 报 "implicit declaration of function 'swscanf'"。 */
void neg3(void) {
    /* 故意不包含 <wchar.h> */
    const wchar_t *s = L"1";
    int a;
    swscanf(s, L"%d", &a);        /* 错误：无声明 */
}

/* 违反约束「格式串必须为宽字符字面量/宽字符串」：
 * 使用普通字符字面量作为格式串，类型不匹配。
 * 期望：gcc -std=c99 报 "incompatible pointer type"。 */
void neg4(void) {
    const wchar_t *s = L"1";
    int a;
    swscanf(s, L"%d" + 0, &a);    /* 合法；下面才是错误示例 */
    /* swscanf(s, "%d", &a); */   /* 错误：窄格式串 */
}

#endif /* 负向测试结束 */