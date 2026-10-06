/*
 * 测试 C99 7.24.3.4 —— fputws 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：int fputws(const wchar_t * restrict s, FILE * restrict stream);
 *   [2] 把 s 指向的宽字符串写入 stream，不写入结尾的空宽字符。
 *   [3] 写错误或编码错误返回 EOF；否则返回非负值。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 检查函数原型存在且返回类型为 int，参数为 (const wchar_t * restrict, FILE * restrict) */
static int check_prototype(void)
{
    /* 通过函数指针类型匹配来验证原型签名 */
    int (*fp)(const wchar_t * restrict, FILE * restrict) = fputws;
    return fp != NULL;
}

int main(void)
{
    /* 设置一个支持宽字符的本地环境，便于编码测试 */
    setlocale(LC_ALL, "C");

    /* [1] 原型检查 */
    assert(check_prototype());

    /* [2] 基本写入：把宽字符串写入文件流，不写入结尾空宽字符 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        const wchar_t *ws = L"Hello";
        int r = fputws(ws, fp);
        /* [3] 成功时返回非负值 */
        assert(r >= 0);

        /* 回读验证内容：应为 "Hello"，不含结尾空宽字符 */
        assert(fflush(fp) == 0);
        assert(fseek(fp, 0, SEEK_SET) == 0);

        char buf[64];
        size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
        buf[n] = '\0';
        /* 写入的字节数应等于 "Hello" 的编码长度（在 C locale 下为 5） */
        assert(strcmp(buf, "Hello") == 0);
        /* 确认没有写入结尾空宽字符（即没有多余的 '\0' 字节） */
        assert(n == 5);

        fclose(fp);
    }

    /* [2] 空宽字符串：写入 0 个字符，返回非负值 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        int r = fputws(L"", fp);
        assert(r >= 0);

        assert(fflush(fp) == 0);
        assert(fseek(fp, 0, SEEK_SET) == 0);
        char buf[8];
        size_t n = fread(buf, 1, sizeof(buf), fp);
        assert(n == 0); /* 空串不写入任何字节 */

        fclose(fp);
    }

    /* [2] 多次调用：连续写入两个宽字符串，内容应拼接 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        assert(fputws(L"AB", fp) >= 0);
        assert(fputws(L"CD", fp) >= 0);

        assert(fflush(fp) == 0);
        assert(fseek(fp, 0, SEEK_SET) == 0);
        char buf[16];
        size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
        buf[n] = '\0';
        assert(strcmp(buf, "ABCD") == 0);

        fclose(fp);
    }

    /* [3] 写错误：对只读流写入应返回 EOF */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        assert(fputws(L"x", fp) >= 0);
        assert(fflush(fp) == 0);
        assert(fseek(fp, 0, SEEK_SET) == 0);

        /* 以只读方式重新打开同一文件不可行（tmpfile 无名字），
           改用 fopen 一个只读文件来测试写错误 */
        fclose(fp);

        const char *path = "fputws_test_tmp.txt";
        FILE *w = fopen(path, "w");
        assert(w != NULL);
        assert(fputws(L"data", w) >= 0);
        fclose(w);

        FILE *r = fopen(path, "r");
        assert(r != NULL);
        /* 对只读流调用 fputws 应失败并返回 EOF */
        int rr = fputws(L"more", r);
        assert(rr == EOF);

        fclose(r);
        remove(path);
    }

    /* [3] 返回值为非负整数（成功路径） */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);
        int r = fputws(L"test", fp);
        assert(r >= 0);
        fclose(fp);
    }

    printf("All positive tests for C99 7.24.3.4 fputws passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须匹配原型」：
   fputws 的第一个参数类型为 const wchar_t *，传入 char * 应报错。
   期望：gcc -std=c99 报 incompatible pointer type / 参数类型不匹配。 */
void neg_wrong_first_arg(void)
{
    FILE *fp = tmpfile();
    char *s = "narrow";
    fputws(s, fp);   /* 错误：char * 不能转换为 const wchar_t * */
}

/* 违反约束「实参类型必须匹配原型」：
   第二个参数类型为 FILE *，传入 int 应报错。
   期望：gcc -std=c99 报 incompatible type for argument 2。 */
void neg_wrong_second_arg(void)
{
    const wchar_t *s = L"x";
    fputws(s, 0);    /* 错误：int 不能转换为 FILE *（0 作为空指针常量可接受，
                        但此处用非零整数以明确违反约束） */
    fputws(s, 42);   /* 错误：int 不能转换为 FILE * */
}

/* 违反约束「参数个数必须匹配原型」：
   fputws 需要 2 个参数，只传 1 个应报错。
   期望：gcc -std=c99 报 too few arguments to function 'fputws'。 */
void neg_too_few_args(void)
{
    const wchar_t *s = L"x";
    fputws(s);       /* 错误：参数个数不足 */
}

/* 违反约束「参数个数必须匹配原型」：
   传入 3 个参数应报错。
   期望：gcc -std=c99 报 too many arguments to function 'fputws'。 */
void neg_too_many_args(void)
{
    const wchar_t *s = L"x";
    FILE *fp = tmpfile();
    fputws(s, fp, fp);  /* 错误：参数个数过多 */
}

/* 违反约束「返回值不可作为左值」：
   fputws 返回 int，是右值，对其赋值应报错。
   期望：gcc -std=c99 报 lvalue required as left operand of assignment。 */
void neg_assign_to_return(void)
{
    FILE *fp = tmpfile();
    fputws(L"x", fp) = 0;  /* 错误：函数返回值不是左值 */
}

/* 违反约束「const 限定：不能修改 const 对象」：
   虽然 fputws 接受 const wchar_t *，但若把非 const 指针
   指向的字符串通过其他方式修改，不属于本条款约束。
   这里测试对 const 宽字符串指针解引用赋值应报错。 */
void neg_modify_const_wstring(void)
{
    const wchar_t *s = L"abc";
    s[0] = L'x';   /* 错误：s 指向 const wchar_t，不可修改 */
}

#endif /* 负向测试结束 */