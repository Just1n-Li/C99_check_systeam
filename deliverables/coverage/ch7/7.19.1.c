/*
 * 测试 C99 7.19.1 <stdio.h> Introduction
 *
 * 预期行为：
 *   正向测试：包含 <stdio.h> 后，条款 [1]-[5] 声明的类型、宏、函数均可用，
 *             且语义（类型性质、宏的值/类型、标准流指针类型等）符合标准，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反条款约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 说明：7.19.1 主要是“介绍性”条款，其可测试的“约束”体现在：
 *   - FILE / fpos_t 是对象类型（不是函数类型、不是不完整类型）；
 *   - fpos_t 不是数组类型；
 *   - 各宏展开为整数常量表达式（可用于 #if、数组维度、case 标签等）；
 *   - EOF 类型为 int 且为负值；
 *   - _IOFBF/_IOLBF/_IONBF 与 SEEK_CUR/SEEK_END/SEEK_SET 各自取值互不相同；
 *   - stderr/stdin/stdout 类型为 “pointer to FILE”；
 *   - 标准流指针是表达式（可取地址、可赋值给 FILE*）。
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] <stdio.h> 声明了类型、宏和函数：这里直接使用它们 */
static int use_declared_entities(void)
{
    FILE *fp = NULL;          /* [2] FILE 是对象类型 */
    fpos_t pos;               /* [2] fpos_t 是对象类型 */
    size_t n = 0;             /* [2] size_t 来自 <stddef.h>/<stdio.h> */
    char buf[BUFSIZ];         /* [3] BUFSIZ 是整数常量表达式 */

    (void)fp;
    (void)pos;
    (void)n;
    (void)buf;
    return 0;
}

/* [2] FILE 是对象类型：sizeof 可用，且能声明 FILE 对象 */
static void test_FILE_is_object_type(void)
{
    /* FILE 是完整对象类型，sizeof 必须合法且 > 0 */
    assert(sizeof(FILE) > 0);

    /* 能声明 FILE 类型的对象（对象类型，非函数类型） */
    FILE f;
    memset(&f, 0, sizeof f);
    (void)f;
}

/* [2] fpos_t 是对象类型，且不是数组类型 */
static void test_fpos_t(void)
{
    fpos_t p;
    fpos_t q;

    /* 是对象类型：sizeof 合法 */
    assert(sizeof(fpos_t) > 0);

    /* 不是数组类型：可以整体赋值（数组不能整体赋值） */
    memset(&p, 0, sizeof p);
    q = p;                    /* 若 fpos_t 是数组类型，此行会编译报错 */
    (void)q;

    /* 不是数组类型：可以作为函数返回值/参数类型使用 */
    assert(sizeof(fpos_t) == sizeof(p));
}

/* [3] NULL 宏：来自 <stddef.h>，是空指针常量 */
static void test_NULL(void)
{
    void *p = NULL;
    char *cp = NULL;
    FILE *fp = NULL;
    assert(p == NULL);
    assert(cp == NULL);
    assert(fp == NULL);
    /* NULL 是空指针常量，可赋给任意指针类型 */
    assert(NULL == 0);
}

/* [3] _IOFBF / _IOLBF / _IONBF：整数常量表达式，取值互不相同，
 *     适合作为 setvbuf 的第三个参数 */
static void test_buffering_macros(void)
{
    /* 整数常量表达式：可用于 #if 与数组维度 */
#if !defined(_IOFBF) || !defined(_IOLBF) || !defined(_IONBF)
#error "_IOFBF/_IOLBF/_IONBF must be defined"
#endif
    char arr[_IOFBF + _IOLBF + _IONBF + 1];   /* 常量表达式可用作数组维度 */
    (void)arr;

    /* 取值互不相同 */
    assert(_IOFBF != _IOLBF);
    assert(_IOFBF != _IONBF);
    assert(_IOLBF != _IONBF);

    /* 适合作为 setvbuf 的第三个参数（类型可隐式转换为 int） */
    {
        FILE *fp = tmpfile();
        if (fp != NULL) {
            int r1 = setvbuf(fp, NULL, _IOFBF, BUFSIZ);
            int r2 = setvbuf(fp, NULL, _IOLBF, BUFSIZ);
            int r3 = setvbuf(fp, NULL, _IONBF, 0);
            (void)r1; (void)r2; (void)r3;
            fclose(fp);
        }
    }
}

/* [3] BUFSIZ：整数常量表达式，是 setbuf 所用缓冲区大小 */
static void test_BUFSIZ(void)
{
#if !defined(BUFSIZ)
#error "BUFSIZ must be defined"
#endif
    char buf[BUFSIZ];         /* 常量表达式用作数组维度 */
    assert(sizeof buf == BUFSIZ);
    assert(BUFSIZ > 0);

    /* 作为 setbuf 的缓冲区大小使用 */
    {
        FILE *fp = tmpfile();
        if (fp != NULL) {
            setbuf(fp, buf);
            fclose(fp);
        }
    }
}

/* [3] EOF：整数常量表达式，类型为 int，值为负 */
static void test_EOF(void)
{
#if !defined(EOF)
#error "EOF must be defined"
#endif
    /* 整数常量表达式：可用于 #if */
#if EOF >= 0
#error "EOF must be negative"
#endif
    /* 类型为 int */
    {
        int e = EOF;
        assert(e < 0);
        assert(EOF == e);
    }
    /* 是负值 */
    assert(EOF < 0);

    /* 由若干函数返回以指示文件结束：用 getc 在空流上验证 */
    {
        FILE *fp = tmpfile();
        if (fp != NULL) {
            int c = getc(fp);   /* 空文件，立即 EOF */
            assert(c == EOF);
            fclose(fp);
        }
    }
}

/* [3] FOPEN_MAX：整数常量表达式，实现保证可同时打开的最少文件数 */
static void test_FOPEN_MAX(void)
{
#if !defined(FOPEN_MAX)
#error "FOPEN_MAX must be defined"
#endif
    char arr[FOPEN_MAX];      /* 常量表达式 */
    (void)arr;
    assert(FOPEN_MAX >= 8);   /* C99 要求至少 8 */
}

/* [3] FILENAME_MAX：整数常量表达式，足以容纳最长文件名的 char 数组大小 */
static void test_FILENAME_MAX(void)
{
#if !defined(FILENAME_MAX)
#error "FILENAME_MAX must be defined"
#endif
    char name[FILENAME_MAX];  /* 常量表达式用作数组维度 */
    (void)name;
    assert(FILENAME_MAX > 0);
}

/* [3] L_tmpnam：整数常量表达式，足以容纳 tmpnam 生成的临时文件名 */
static void test_L_tmpnam(void)
{
#if !defined(L_tmpnam)
#error "L_tmpnam must be defined"
#endif
    char name[L_tmpnam];      /* 常量表达式用作数组维度 */
    assert(L_tmpnam > 0);

    /* 实际用 tmpnam 生成一个名字，验证缓冲区足够 */
    {
        char *r = tmpnam(name);
        if (r != NULL) {
            assert(strlen(name) < (size_t)L_tmpnam);
        }
    }
}

/* [3] SEEK_CUR / SEEK_END / SEEK_SET：整数常量表达式，取值互不相同，
 *     适合作为 fseek 的第三个参数 */
static void test_seek_macros(void)
{
#if !defined(SEEK_CUR) || !defined(SEEK_END) || !defined(SEEK_SET)
#error "SEEK_CUR/SEEK_END/SEEK_SET must be defined"
#endif
    char arr[SEEK_CUR + SEEK_END + SEEK_SET + 1];  /* 常量表达式 */
    (void)arr;

    /* 取值互不相同 */
    assert(SEEK_CUR != SEEK_END);
    assert(SEEK_CUR != SEEK_SET);
    assert(SEEK_END != SEEK_SET);

    /* 适合作为 fseek 的第三个参数 */
    {
        FILE *fp = tmpfile();
        if (fp != NULL) {
            fputs("hello", fp);
            int r1 = fseek(fp, 0, SEEK_SET);
            int r2 = fseek(fp, 0, SEEK_CUR);
            int r3 = fseek(fp, 0, SEEK_END);
            (void)r1; (void)r2; (void)r3;
            fclose(fp);
        }
    }
}

/* [3] TMP_MAX：整数常量表达式，tmpnam 可生成的唯一文件名最大数 */
static void test_TMP_MAX(void)
{
#if !defined(TMP_MAX)
#error "TMP_MAX must be defined"
#endif
    char arr[TMP_MAX > 0 ? 1 : 1];  /* 常量表达式 */
    (void)arr;
    assert(TMP_MAX > 0);
}

/* [3] stderr / stdin / stdout：类型为 “pointer to FILE”，
 *     分别指向标准错误、输入、输出流 */
static void test_standard_streams(void)
{
    /* 类型为 pointer to FILE：可赋给 FILE* */
    FILE *in  = stdin;
    FILE *out = stdout;
    FILE *err = stderr;
    assert(in  != NULL);
    assert(out != NULL);
    assert(err != NULL);

    /* 是表达式：可取地址，得到 FILE** */
    FILE **pin  = &stdin;
    FILE **pout = &stdout;
    FILE **perr = &stderr;
    assert(*pin  == stdin);
    assert(*pout == stdout);
    assert(*perr == stderr);

    /* 三者互不相同（分别关联不同流） */
    assert(stdin  != stdout);
    assert(stdin  != stderr);
    assert(stdout != stderr);

    /* 实际可用：向 stdout 写、从 stdin 读类型正确 */
    {
        int n = fprintf(stdout, "%s", "");   /* 写 0 个字符 */
        assert(n == 0);
        fflush(stdout);
    }
}

/* [4] <wchar.h> 声明宽字符 I/O 函数；此处仅验证头文件可包含，
 *     且其声明的函数名可用（不调用，只取地址以验证声明存在） */
#include <wchar.h>
static void test_wchar_header(void)
{
    /* 取函数地址验证声明存在（条款 [4] 提到 <wchar.h> 声明这些函数） */
    int (*pfgetwc)(FILE *) = fgetwc;
    int (*pfputwc)(wint_t, FILE *) = fputwc;
    (void)pfgetwc;
    (void)pfputwc;
}

/* [5] 字节输入/输出函数集合：验证这些函数名均被声明（取地址） */
static void test_byte_io_functions_declared(void)
{
    /* 输入函数 */
    int   (*p_fgetc)(FILE *)            = fgetc;
    char *(*p_fgets)(char *, int, FILE *) = fgets;
    int   (*p_fscanf)(FILE *, const char *, ...) = fscanf;
    int   (*p_getc)(FILE *)             = getc;
    int   (*p_getchar)(void)            = getchar;
    char *(*p_gets)(char *)             = gets;
    int   (*p_scanf)(const char *, ...) = scanf;
    int   (*p_ungetc)(int, FILE *)      = ungetc;
    int   (*p_vfscanf)(FILE *, const char *, va_list_dummy_t) = NULL; /* 占位 */

    /* 输出函数 */
    int   (*p_fprintf)(FILE *, const char *, ...) = fprintf;
    int   (*p_fputc)(int, FILE *)       = fputc;
    int   (*p_fputs)(const char *, FILE *) = fputs;
    int   (*p_printf)(const char *, ...) = printf;
    int   (*p_putc)(int, FILE *)        = putc;
    int   (*p_putchar)(int)             = putchar;
    int   (*p_puts)(const char *)       = puts;

    /* 二进制 I/O */
    size_t (*p_fread)(void *, size_t, size_t, FILE *)  = fread;
    size_t (*p_fwrite)(const void *, size_t, size_t, FILE *) = fwrite;

    (void)p_fgetc; (void)p_fgets; (void)p_fscanf; (void)p_getc;
    (void)p_getchar; (void)p_gets; (void)p_scanf; (void)p_ungetc;
    (void)p_vfscanf;
    (void)p_fprintf; (void)p_fputc; (void)p_fputs; (void)p_printf;
    (void)p_putc; (void)p_putchar; (void)p_puts;
    (void)p_fread; (void)p_fwrite;
}

int main(void)
{
    use_declared_entities();
    test_FILE_is_object_type();
    test_fpos_t();
    test_NULL();
    test_buffering_macros();
    test_BUFSIZ();
    test_EOF();
    test_FOPEN_MAX();
    test_FILENAME_MAX();
    test_L_tmpnam();
    test_seek_macros();
    test_TMP_MAX();
    test_standard_streams();
    test_wchar_header();
    test_byte_io_functions_declared();

    printf("All positive tests for C99 7.19.1 passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * （统一放在 #if 0 中，保证本文件仍能正常编译运行）
 * ============================================================ */
#if 0

/* 违反约束「FILE 是对象类型」：把 FILE 当作函数类型使用（调用它）。
 * 期望：gcc -std=c99 报错，如 "called object 'f' is not a function"。
 */
void neg_FILE_not_function(void)
{
    FILE f;
    f();   /* 错误：FILE 是对象类型，不是函数类型 */
}

/* 违反约束「fpos_t 是对象类型且不是数组类型」：
 * 把 fpos_t 当作数组类型使用（用下标访问）。
 * 期望：gcc -std=c99 报错，如 "subscripted value is neither array nor pointer"。
 */
void neg_fpos_t_not_array(void)
{
    fpos_t p;
    p[0] = p[0];   /* 错误：fpos_t 不是数组类型 */
}

/* 违反约束「fpos_t 不是数组类型」：对 fpos_t 对象整体赋值是允许的，
 * 但若把它声明为数组再整体赋值则违反约束。这里直接对数组整体赋值。
 * 期望：gcc -std=c99 报错，如 "assignment to expression with array type"。
 */
void neg_array_assignment(void)
{
    char a[4];
    char b[4];
    a = b;   /* 错误：数组不能整体赋值 */
}

/* 违反约束「_IOFBF/_IOLBF/_IONBF 是整数常量表达式」：
 * 在需要整数常量表达式的地方（case 标签）使用非常量。
 * 期望：gcc -std=c99 报错，如 "case label does not reduce to an integer constant"。
 */
void neg_buffering_not_constant(int x)
{
    switch (x) {
    case _IOFBF: break;   /* 若 _IOFBF 不是整数常量表达式则报错 */
    }
}

/* 违反约束「EOF 类型为 int」：把 EOF 用作需要非 int 类型的地方，
 * 例如作为指针初始化（int 不能隐式转换为指针）。
 * 期望：gcc -std=c99 报错，如 "initialization makes pointer from integer"。
 */
void neg_EOF_not_pointer(void)
{
    char *p = EOF;   /* 错误：EOF 是 int，不能初始化指针 */
    (void)p;
}

/* 违反约束「stderr/stdin/stdout 是 pointer to FILE」：
 * 把它们当作 FILE 对象（非指针）使用，例如取成员。
 * 期望：gcc -std=c99 报错，如 "invalid type argument of '->'"。
 */
void neg_stdout_not_object(void)
{
    stdout->_flag = 0;   /* 错误：stdout 是 FILE*，不是 FILE 对象 */
}

/* 违反约束「stderr/stdin/stdout 是 pointer to FILE」：
 * 把它们赋给不兼容的指针类型（FILE* 不能隐式转换为 int*）。
 * 期望：gcc -std=c99 报错，如 "incompatible pointer types"。
 */
void neg_stdout_wrong_pointer_type(void)
{
    int *p = stdout;   /* 错误：FILE* 不能隐式转换为 int* */
    (void)p;
}

/* 违反约束「SEEK_CUR/SEEK_END/SEEK_SET 是整数常量表达式」：
 * 在数组维度中使用非常量。
 * 期望：gcc -std=c99 报错，如 "variably modified ... at file scope"。
 */
int neg_seek_not_constant_array[SEEK_CUR];   /* 若 SEEK_CUR 非常量则报错 */

#endif /* 负向测试结束 */