/*
 * 测试目标：C99 7.24.4.5.1 —— wcschr 函数
 *
 * 预期行为：
 *   正向测试：包含 <wchar.h>，调用 wcschr 定位宽字符串中首次出现的宽字符，
 *             验证 [2] 语义（含终止空宽字符视为字符串一部分）与 [3] 返回值语义，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），期望编译器报错。
 *
 * 说明：wcschr 的约束主要来自 <wchar.h> 的声明与参数类型（const wchar_t*、
 *       wchar_t），以及标准库函数的一般约束（参数指针有效性属 UB 而非约束，
 *       故不作为负向测试）。负向测试针对可静态检查的类型约束。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] Synopsis：头文件 <wchar.h> 提供 wcschr 声明，原型为
     *     wchar_t *wcschr(const wchar_t *s, wchar_t c);
     *     通过取函数指针并赋给匹配的原型来静态验证签名。 */
    {
        wchar_t *(*fp)(const wchar_t *, wchar_t) = wcschr;
        assert(fp != NULL);
    }

    /* [2] 定位首次出现的宽字符 */
    {
        const wchar_t s[] = L"hello world";
        wchar_t *p = wcschr(s, L'o');
        assert(p != NULL);
        assert(*p == L'o');
        /* 首次出现：索引 4（"hell[o]..."） */
        assert(p == s + 4);
    }

    /* [2] 重复字符时返回第一次出现的位置 */
    {
        const wchar_t s[] = L"aXbXcX";
        wchar_t *p = wcschr(s, L'X');
        assert(p == s + 1);
    }

    /* [2] 终止空宽字符被视为宽字符串的一部分：
     *     查找 L'\0' 应返回指向终止符的指针，而非 NULL。 */
    {
        const wchar_t s[] = L"abc";
        wchar_t *p = wcschr(s, L'\0');
        assert(p != NULL);
        assert(p == s + 3);   /* 指向终止空宽字符 */
        assert(*p == L'\0');
    }

    /* [2] 空宽字符串：仅含终止空宽字符，查找 L'\0' 返回首地址 */
    {
        const wchar_t s[] = L"";
        wchar_t *p = wcschr(s, L'\0');
        assert(p == s);
    }

    /* [3] 字符不存在时返回空指针 */
    {
        const wchar_t s[] = L"hello";
        wchar_t *p = wcschr(s, L'z');
        assert(p == NULL);
    }

    /* [3] 空宽字符串中查找非空字符返回 NULL */
    {
        const wchar_t s[] = L"";
        wchar_t *p = wcschr(s, L'a');
        assert(p == NULL);
    }

    /* [3] 返回值可用于修改（非常量字符串），验证返回类型为 wchar_t* */
    {
        wchar_t s[] = L"hello";
        wchar_t *p = wcschr(s, L'e');
        assert(p != NULL);
        *p = L'E';
        assert(wcscmp(s, L"hEllo") == 0);
    }

    /* [2][3] 与 wcslen 配合：定位终止符位置应等于字符串长度 */
    {
        const wchar_t s[] = L"wide string";
        wchar_t *p = wcschr(s, L'\0');
        assert(p == s + wcslen(s));
    }

    printf("All positive tests for C99 7.24.4.5.1 (wcschr) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcschr 的第一个参数类型为 const wchar_t *」：
 * 传入 char*（窄字符串指针）而非 wchar_t*，类型不兼容，
 * gcc -std=c99 应报错（如 "incompatible pointer type" / "passing argument 1"）。 */
void neg_wrong_first_arg_type(void)
{
    char narrow[] = "abc";
    wchar_t *p = wcschr(narrow, L'a');   /* 错误：第一个实参应为 const wchar_t* */
    (void)p;
}

/* 违反约束「wcschr 的第二个参数类型为 wchar_t」：
 * 传入 int（非 wchar_t 且无法隐式转换为 wchar_t 的指针类型），
 * 这里传入 char* 指针，类型不兼容，应报错。 */
void neg_wrong_second_arg_type(void)
{
    const wchar_t s[] = L"abc";
    char *c = "a";
    wchar_t *p = wcschr(s, c);           /* 错误：第二个实参应为 wchar_t */
    (void)p;
}

/* 违反约束「wcschr 返回 wchar_t *，不能赋给不兼容的指针类型」：
 * 将返回值赋给 int*，类型不兼容，应报错。 */
void neg_wrong_return_assignment(void)
{
    const wchar_t s[] = L"abc";
    int *p = wcschr(s, L'a');            /* 错误：wchar_t* 不能初始化 int* */
    (void)p;
}

/* 违反约束「wcschr 需要两个实参」：
 * 实参个数不足，应报错（"too few arguments to function 'wcschr'"）。 */
void neg_too_few_args(void)
{
    const wchar_t s[] = L"abc";
    wchar_t *p = wcschr(s);              /* 错误：缺少第二个实参 */
    (void)p;
}

/* 违反约束「wcschr 需要两个实参」：
 * 实参个数过多，应报错（"too many arguments to function 'wcschr'"）。 */
void neg_too_many_args(void)
{
    const wchar_t s[] = L"abc";
    wchar_t *p = wcschr(s, L'a', L'b');  /* 错误：多余第三个实参 */
    (void)p;
}

/* 违反约束「wcschr 的返回类型为 wchar_t *，不可作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应报错（"lvalue required as left operand of assignment"）。 */
void neg_assign_to_call_result(void)
{
    const wchar_t s[] = L"abc";
    wcschr(s, L'a') = (wchar_t *)0;      /* 错误：函数返回值不是左值 */
}

#endif /* 负向测试结束 */