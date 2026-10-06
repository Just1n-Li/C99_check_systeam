/*
 * 测试 C99 7.8 <inttypes.h>
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束的片段应被编译器拒绝（编译报错）。
 *
 * 覆盖：
 *   [1] <inttypes.h> 包含 <stdint.h> 并扩展它（宿主实现）。
 *   [2] 声明操作最大宽度整数的函数（imaxabs, imaxdiv）、
 *       字符串转最大宽度整数的函数（strtoimax, strtoumax）、
 *       类型 imaxdiv_t（imaxdiv 返回值类型），
 *       以及为 <stdint.h> 中每个类型定义对应的格式转换宏（PRI*/SCN*）。
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <wchar.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <inttypes.h> 包含 <stdint.h>：以下类型/宏应可用（来自 stdint.h） */
static void test_header_includes_stdint(void)
{
    int8_t   i8  = INT8_C(-5);
    uint8_t  u8  = UINT8_C(200);
    int16_t  i16 = INT16_C(-1000);
    uint16_t u16 = UINT16_C(60000);
    int32_t  i32 = INT32_C(-123456);
    uint32_t u32 = UINT32_C(4000000000u);
    int64_t  i64 = INT64_C(-1234567890123);
    uint64_t u64 = UINT64_C(12345678901234567890u);

    assert(i8 == -5);
    assert(u8 == 200);
    assert(i16 == -1000);
    assert(u16 == 60000);
    assert(i32 == -123456);
    assert(u32 == 4000000000u);
    assert(i64 == -1234567890123LL);
    assert(u64 == 12345678901234567890ULL);

    /* 最大宽度类型 */
    intmax_t  im = INTMAX_C(-42);
    uintmax_t um = UINTMAX_C(42);
    assert(im == -42);
    assert(um == 42);
}

/* [2] imaxabs：操作最大宽度整数 */
static void test_imaxabs(void)
{
    intmax_t v = INTMAX_C(-123456789);
    intmax_t a = imaxabs(v);
    assert(a == INTMAX_C(123456789));

    assert(imaxabs(INTMAX_C(0)) == 0);
    assert(imaxabs(INTMAX_C(7)) == 7);
}

/* [2] imaxdiv_t 是 imaxdiv 返回值的结构类型；imaxdiv 操作最大宽度整数 */
static void test_imaxdiv(void)
{
    imaxdiv_t r = imaxdiv(INTMAX_C(17), INTMAX_C(5));
    /* imaxdiv_t 是结构类型，含 quot 与 rem 成员 */
    assert(r.quot == 3);
    assert(r.rem  == 2);

    imaxdiv_t r2 = imaxdiv(INTMAX_C(-17), INTMAX_C(5));
    assert(r2.quot == -3);
    assert(r2.rem  == -2);

    /* 结构类型：可赋值、可取地址 */
    imaxdiv_t copy = r;
    assert(copy.quot == 3 && copy.rem == 2);
    imaxdiv_t *p = &copy;
    assert(p->quot == 3 && p->rem == 2);
}

/* [2] strtoimax / strtoumax：字符串转最大宽度整数 */
static void test_strtoimax_strtoumax(void)
{
    char *end = NULL;

    intmax_t si = strtoimax("  -12345xyz", &end, 10);
    assert(si == INTMAX_C(-12345));
    assert(end != NULL && *end == 'x');

    uintmax_t su = strtoumax("67890abc", &end, 10);
    assert(su == UINTMAX_C(67890));
    assert(end != NULL && *end == 'a');

    /* 十六进制 */
    intmax_t hx = strtoimax("0x1F", &end, 16);
    assert(hx == INTMAX_C(31));

    uintmax_t hx2 = strtoumax("0x10", &end, 0);
    assert(hx2 == UINTMAX_C(16));

    /* 无有效数字：返回 0，end 指向原字符串 */
    const char *s = "zzz";
    intmax_t none = strtoimax(s, &end, 10);
    assert(none == 0);
    assert(end == s);
}

/* [2] 为 <stdint.h> 中每个类型定义对应的格式转换宏（PRI*/SCN*）。
 * 这里用 snprintf/sscanf 验证若干代表性宏可用且语义正确。 */
static void test_pri_macros(void)
{
    char buf[128];

    /* 有符号十进制 */
    snprintf(buf, sizeof buf, "%" PRId8,  (int8_t)-12);
    assert(strcmp(buf, "-12") == 0);

    snprintf(buf, sizeof buf, "%" PRId16, (int16_t)-1234);
    assert(strcmp(buf, "-1234") == 0);

    snprintf(buf, sizeof buf, "%" PRId32, (int32_t)-123456);
    assert(strcmp(buf, "-123456") == 0);

    snprintf(buf, sizeof buf, "%" PRId64, (int64_t)-1234567890123LL);
    assert(strcmp(buf, "-1234567890123") == 0);

    snprintf(buf, sizeof buf, "%" PRIdMAX, (intmax_t)-99);
    assert(strcmp(buf, "-99") == 0);

    /* 无符号十进制 */
    snprintf(buf, sizeof buf, "%" PRIu8,  (uint8_t)200);
    assert(strcmp(buf, "200") == 0);

    snprintf(buf, sizeof buf, "%" PRIu16, (uint16_t)60000);
    assert(strcmp(buf, "60000") == 0);

    snprintf(buf, sizeof buf, "%" PRIu32, (uint32_t)4000000000u);
    assert(strcmp(buf, "4000000000") == 0);

    snprintf(buf, sizeof buf, "%" PRIu64, (uint64_t)12345678901234567890ULL);
    assert(strcmp(buf, "12345678901234567890") == 0);

    snprintf(buf, sizeof buf, "%" PRIuMAX, (uintmax_t)42);
    assert(strcmp(buf, "42") == 0);

    /* 十六进制 */
    snprintf(buf, sizeof buf, "%" PRIx8,  (uint8_t)0xAB);
    assert(strcmp(buf, "ab") == 0);

    snprintf(buf, sizeof buf, "%" PRIX16, (uint16_t)0xBEEF);
    assert(strcmp(buf, "BEEF") == 0);

    snprintf(buf, sizeof buf, "%" PRIx32, (uint32_t)0xDEADBEEFu);
    assert(strcmp(buf, "deadbeef") == 0);

    snprintf(buf, sizeof buf, "%" PRIx64, (uint64_t)0x123456789ABCDEF0ULL);
    assert(strcmp(buf, "123456789abcdef0") == 0);

    snprintf(buf, sizeof buf, "%" PRIxMAX, (uintmax_t)0xFF);
    assert(strcmp(buf, "ff") == 0);

    /* 八进制 */
    snprintf(buf, sizeof buf, "%" PRIo8,  (uint8_t)8);
    assert(strcmp(buf, "10") == 0);

    snprintf(buf, sizeof buf, "%" PRIoMAX, (uintmax_t)8);
    assert(strcmp(buf, "10") == 0);

    /* 指针类型：intptr_t / uintptr_t 的格式宏 */
    int x = 0;
    intptr_t ip = (intptr_t)&x;
    snprintf(buf, sizeof buf, "%" PRIdPTR, ip);
    assert(strlen(buf) > 0);

    uintptr_t up = (uintptr_t)&x;
    snprintf(buf, sizeof buf, "%" PRIuPTR, up);
    assert(strlen(buf) > 0);

    snprintf(buf, sizeof buf, "%" PRIxPTR, up);
    assert(strlen(buf) > 0);
}

/* [2] SCN* 宏用于格式化输入 */
static void test_scn_macros(void)
{
    int8_t   i8  = 0;
    uint8_t  u8  = 0;
    int16_t  i16 = 0;
    uint16_t u16 = 0;
    int32_t  i32 = 0;
    uint32_t u32 = 0;
    int64_t  i64 = 0;
    uint64_t u64 = 0;
    intmax_t im  = 0;
    uintmax_t um = 0;

    assert(sscanf("-12", "%" SCNd8, &i8) == 1 && i8 == -12);
    assert(sscanf("200", "%" SCNu8, &u8) == 1 && u8 == 200);
    assert(sscanf("-1234", "%" SCNd16, &i16) == 1 && i16 == -1234);
    assert(sscanf("60000", "%" SCNu16, &u16) == 1 && u16 == 60000);
    assert(sscanf("-123456", "%" SCNd32, &i32) == 1 && i32 == -123456);
    assert(sscanf("4000000000", "%" SCNu32, &u32) == 1 && u32 == 4000000000u);
    assert(sscanf("-1234567890123", "%" SCNd64, &i64) == 1 && i64 == -1234567890123LL);
    assert(sscanf("12345678901234567890", "%" SCNu64, &u64) == 1 && u64 == 12345678901234567890ULL);
    assert(sscanf("-99", "%" SCNdMAX, &im) == 1 && im == -99);
    assert(sscanf("42", "%" SCNuMAX, &um) == 1 && um == 42);

    /* 十六进制输入 */
    uint32_t hx = 0;
    assert(sscanf("deadbeef", "%" SCNx32, &hx) == 1 && hx == 0xDEADBEEFu);

    /* 指针类型输入 */
    intptr_t ip = 0;
    assert(sscanf("123", "%" SCNdPTR, &ip) == 1 && ip == 123);
}

/* [2] 最大宽度类型与格式宏配合使用 */
static void test_maxwidth_roundtrip(void)
{
    char buf[128];
    intmax_t  im = INTMAX_C(-1234567890123456789);
    uintmax_t um = UINTMAX_C(12345678901234567890);

    snprintf(buf, sizeof buf, "%" PRIdMAX, im);
    intmax_t im2 = 0;
    assert(sscanf(buf, "%" SCNdMAX, &im2) == 1);
    assert(im2 == im);

    snprintf(buf, sizeof buf, "%" PRIuMAX, um);
    uintmax_t um2 = 0;
    assert(sscanf(buf, "%" SCNuMAX, &um2) == 1);
    assert(um2 == um);
}

int main(void)
{
    test_header_includes_stdint();
    test_imaxabs();
    test_imaxdiv();
    test_strtoimax_strtoumax();
    test_pri_macros();
    test_scn_macros();
    test_maxwidth_roundtrip();

    printf("C99 7.8 <inttypes.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「imaxdiv_t 是结构类型，其成员为 quot/rem」：
 * 访问不存在的成员应编译报错（gcc -std=c99 报 "no member named ..."）。 */
void neg_imaxdiv_bad_member(void)
{
    imaxdiv_t r = imaxdiv(1, 2);
    (void)r.nonexistent_member;   /* 错误：imaxdiv_t 无此成员 */
}

/* 违反约束「imaxdiv 的两个实参必须为 intmax_t 类型」：
 * 传入结构体类型实参应编译报错。 */
struct NotInt { int a; };
void neg_imaxdiv_bad_arg(void)
{
    struct NotInt s;
    (void)imaxdiv(s, s);          /* 错误：实参类型不匹配 */
}

/* 违反约束「imaxabs 的实参必须为 intmax_t 类型」：
 * 传入指针类型应编译报错。 */
void neg_imaxabs_bad_arg(void)
{
    int *p = 0;
    (void)imaxabs(p);             /* 错误：指针不能隐式转换为 intmax_t */
}

/* 违反约束「strtoimax 的第二个实参必须为 char ** 类型」：
 * 传入 int * 应编译报错。 */
void neg_strtoimax_bad_endptr(void)
{
    int n = 0;
    (void)strtoimax("123", &n, 10);  /* 错误：&n 类型为 int*，非 char** */
}

/* 违反约束「strtoumax 的第三个实参必须为 int 类型」：
 * 传入指针应编译报错。 */
void neg_strtoumax_bad_base(void)
{
    char *end = 0;
    int *base = 0;
    (void)strtoumax("123", &end, base);  /* 错误：base 为 int*，非 int */
}

/* 违反约束「PRI* 宏必须用于格式字符串中，且与对应类型匹配」：
 * 将 PRI 宏当作普通标识符使用（未在字符串中拼接）应编译报错。 */
void neg_pri_macro_misuse(void)
{
    int x = PRId32;               /* 错误：PRId32 展开为字符串字面量，不能初始化 int */
    (void)x;
}

/* 违反约束「SCN* 宏必须用于格式字符串中」：
 * 将 SCN 宏当作整数常量使用应编译报错。 */
void neg_scn_macro_misuse(void)
{
    int y = SCNd32;               /* 错误：SCNd32 展开为字符串字面量，不能初始化 int */
    (void)y;
}

/* 违反约束「imaxdiv_t 为结构类型，不能直接与整数比较」：
 * 结构体与整数比较应编译报错。 */
void neg_imaxdiv_compare(void)
{
    imaxdiv_t r = imaxdiv(1, 2);
    if (r == 0) { }               /* 错误：结构体不能与整数比较 */
}

/* 违反约束「imaxdiv_t 为结构类型，不能直接赋值给整数」：
 * 结构体赋值给 intmax_t 应编译报错。 */
void neg_imaxdiv_assign(void)
{
    imaxdiv_t r = imaxdiv(1, 2);
    intmax_t v = r;               /* 错误：结构体不能隐式转换为 intmax_t */
    (void)v;
}

#endif /* 负向测试结束 */