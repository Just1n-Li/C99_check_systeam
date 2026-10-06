/*
 * 测试 C99 7.19.6.8 —— vfprintf 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型 / 头文件 <stdarg.h> <stdio.h>
 *   [2] 等价于 fprintf，变参列表由 va_start 初始化的 va_list 替换；
 *       vfprintf 不调用 va_end。
 *   [3] 返回写入的字符数；出错返回负值。
 *   [4] EXAMPLE 通用错误报告例程。
 *   脚注 254：调用后 arg 的值不确定（UB，不作为负向测试）。
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [4] EXAMPLE：通用错误报告例程（原样取自条款） */
static void error(char *function_name, char *format, ...)
{
    va_list args;
    va_start(args, format);
    /* print out name of function causing error */
    fprintf(stderr, "ERROR in %s: ", function_name);
    /* print out remainder of message */
    vfprintf(stderr, format, args);
    va_end(args);
}

/* [2] 用 vfprintf 把 va_list 转发到另一个函数，验证等价于 fprintf */
static int forward_vfprintf(FILE *stream, const char *format, ...)
{
    va_list ap;
    int n;
    va_start(ap, format);
    n = vfprintf(stream, format, ap);   /* [2] 变参列表由 arg 替换 */
    va_end(ap);                          /* [2] vfprintf 自身不调用 va_end，由调用者负责 */
    return n;
}

/* [2] 在 va_start 之后、vfprintf 之前使用 va_arg，验证 arg 可被后续 va_arg 调用推进 */
static int mixed_va_arg_vfprintf(FILE *stream, const char *fmt1, const char *fmt2, ...)
{
    va_list ap;
    int first;
    int n1, n2;
    va_start(ap, fmt2);
    first = va_arg(ap, int);             /* 先取一个 int */
    n1 = vfprintf(stream, fmt1, ap);     /* 再用剩余参数 */
    n2 = vfprintf(stream, fmt2, ap);     /* 再次使用（脚注 254：之后 arg 值不确定，此处不再使用） */
    va_end(ap);
    (void)first;
    return n1 + n2;
}

int main(void)
{
    /* ---------- [1] 原型与头文件可用性 ---------- */
    /* 通过取函数指针验证原型签名：FILE *restrict, const char *restrict, va_list */
    {
        int (*fp)(FILE * restrict, const char * restrict, va_list) = vfprintf;
        assert(fp != NULL);
    }

    /* ---------- [2] 等价于 fprintf：输出内容与返回值一致 ---------- */
    {
        char buf1[128];
        char buf2[128];
        FILE *f1 = tmpfile();
        FILE *f2 = tmpfile();
        int r1, r2;
        assert(f1 != NULL && f2 != NULL);

        /* 用 fprintf 直接写 */
        r1 = fprintf(f1, "value=%d str=%s\n", 42, "hello");
        /* 用 vfprintf 经 va_list 写同样内容 */
        r2 = forward_vfprintf(f2, "value=%d str=%s\n", 42, "hello");

        assert(r1 == r2);                /* [3] 返回字符数相同 */
        assert(r1 > 0);

        rewind(f1);
        rewind(f2);
        assert(fgets(buf1, sizeof buf1, f1) != NULL);
        assert(fgets(buf2, sizeof buf2, f2) != NULL);
        assert(strcmp(buf1, buf2) == 0); /* [2] 输出等价于 fprintf */

        fclose(f1);
        fclose(f2);
    }

    /* ---------- [3] 返回值 = 写入的字符数 ---------- */
    {
        FILE *f = tmpfile();
        int n;
        assert(f != NULL);
        n = forward_vfprintf(f, "%s", "abcdef");   /* 6 个字符 */
        assert(n == 6);
        n = forward_vfprintf(f, "%d", 12345);      /* 5 个字符 */
        assert(n == 5);
        n = forward_vfprintf(f, "%c%c%c", 'x', 'y', 'z'); /* 3 个字符 */
        assert(n == 3);
        fclose(f);
    }

    /* ---------- [2] va_start 之后可先 va_arg 再 vfprintf ---------- */
    {
        FILE *f = tmpfile();
        int total;
        assert(f != NULL);
        /* 第一个 int 被 va_arg 取走，剩余 "B" 与 7 交给 vfprintf */
        total = mixed_va_arg_vfprintf(f, "%s", "%d", 99, "B", 7);
        assert(total == 1 + 1);   /* "B" 1 字符 + "7" 1 字符 */
        fclose(f);
    }

    /* ---------- [4] EXAMPLE 例程可正常调用（输出到 stderr） ---------- */
    error("my_func", "bad argument %d\n", 5);

    /* ---------- [3] 出错时返回负值：向只读流写入 ---------- */
    {
        FILE *f = fopen("/dev/null", "r");   /* 只读打开 */
        if (f != NULL) {
            int n = forward_vfprintf(f, "%s", "should fail");
            /* 向只读流写入应失败，返回负值 */
            assert(n < 0);
            fclose(f);
        }
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束 [1]「vfprintf 的第一个参数类型为 FILE *」：
 * 传入 int 而非 FILE *，gcc -std=c99 应报 incompatible type 错误。 */
#include <stdio.h>
#include <stdarg.h>
void bad1(va_list ap) {
    vfprintf(123, "%d", ap);   /* error: 第一个实参不是 FILE * */
}

/* 违反约束 [1]「第二个参数类型为 const char *」：
 * 传入 int 而非字符串指针，应报 incompatible type 错误。 */
void bad2(FILE *f, va_list ap) {
    vfprintf(f, 42, ap);       /* error: 第二个实参不是 const char * */
}

/* 违反约束 [1]「第三个参数类型为 va_list」：
 * 传入 int 而非 va_list，应报 incompatible type 错误。 */
void bad3(FILE *f) {
    vfprintf(f, "%d", 7);      /* error: 第三个实参不是 va_list */
}

/* 违反约束 [1]「参数个数必须为 3」：
 * 少传参数，应报 too few arguments 错误。 */
void bad4(FILE *f, va_list ap) {
    vfprintf(f, "%d");         /* error: 缺少第三个实参 */
}

/* 违反约束 [1]「参数个数必须为 3」：
 * 多传参数，应报 too many arguments 错误。 */
void bad5(FILE *f, va_list ap) {
    vfprintf(f, "%d", ap, 1);  /* error: 实参过多 */
}

/* 违反约束 [1]「返回类型为 int」：
 * 把返回值赋给不兼容的指针类型（在严格诊断下应报错）。 */
void bad6(FILE *f, va_list ap) {
    int *p = vfprintf(f, "%d", ap);  /* error: int 赋给 int * */
    (void)p;
}

#endif