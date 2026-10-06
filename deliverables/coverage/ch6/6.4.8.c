/*
 * 验证 C99 条款 6.4.8 (Preprocessing numbers)
 * 预期行为：
 * - 正向测试：应能编译并运行通过，验证预处理数字的语法规则、词法包含范围及阶段7转换后的类型与值。
 * - 负向测试：应编译报错，验证无法转换为有效整数或浮点常量的预处理数字将触发约束违规。
 */

#include <stdio.h>
#include <assert.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 语法：pp-number 以 digit 或 . digit 开头 */
    int a = 123;          /* pp-number: digit */
    double b = .456;      /* pp-number: . digit */
    assert(a == 123);
    assert(b == 0.456);

    /* [1] 语法：pp-number 可后接 . 和 identifier-nondigit */
    double c = 123.456;   /* pp-number: digit pp-number . */
    double d = 1.5f;      /* pp-number 后接浮点后缀 f */
    assert(c == 123.456);
    assert(d == 1.5);

    /* [2] 描述：pp-number 可包含 e+, e-, E+, E-, p+, p-, P-, P- 序列 */
    double e1 = 1e+1;     double e2 = 1e-1;
    double e3 = 1E+1;     double e4 = 1E-1;
    double e5 = 0x1p+1;   double e6 = 0x1p-1;
    double e7 = 0x1P+1;   double e8 = 0x1P-1;
    assert(e1 == 10.0 && e2 == 0.1);
    assert(e3 == 10.0 && e4 == 0.1);
    assert(e5 == 2.0 && e6 == 0.5);
    assert(e7 == 2.0 && e8 == 0.5);

    /* [3] 词法包含：预处理数字词法上包含所有整数和浮点常量 */
    int dec = 10;         /* 十进制整型常量 */
    int oct = 010;        /* 八进制整型常量 */
    int hex = 0x10;       /* 十六进制整型常量 */
    double fl1 = 10.0;    /* 十进制浮点常量 */
    double fl2 = 1e1;     /* 指数浮点常量 */
    double fl3 = 0x1p1;   /* 十六进制浮点常量 */
    assert(dec == 10 && oct == 8 && hex == 16);
    assert(fl1 == 10.0 && fl2 == 10.0 && fl3 == 2.0);

    /* [4] 语义：预处理数字在阶段7转换为常量后才获得类型和值 */
    /* 验证转换后获得的类型 */
    assert(sizeof(123) == sizeof(int));      /* 转换为 int 类型 */
    assert(sizeof(123.0) == sizeof(double)); /* 转换为 double 类型 */
    assert(sizeof(0x1p1) == sizeof(double));/* 转换为 double 类型 */
    
    /* 验证转换后获得的值 */
    assert(123 + 456 == 579);
    assert(0x10 == 16);

    printf("正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* [4] 语义约束：预处理数字必须在阶段7成功转换为浮点或整型常量。
       以下预处理数字在词法上是合法的 pp-number，但无法转换为有效的常量，违反约束。 */

    /* 违反约束：pp-number '1abc' 是合法的 pp-number (digit pp-number identifier-nondigit)，
       但无法转换为有效的整型或浮点常量，gcc -std=c99 应报错 */
    int x = 1abc;

    /* 违反约束：pp-number '1.2.3' 是合法的 pp-number (pp-number .)，
       但无法转换为有效的浮点常量，gcc -std=c99 应报错 */
    double y = 1.2.3;

    /* 违反约束：pp-number '1e+' 是合法的 pp-number (pp-number e sign)，
       但缺少指数数字，无法转换为有效的浮点常量，gcc -std=c99 应报错 */
    double z = 1e+;

    /* 违反约束：pp-number '0x1p+' 是合法的 pp-number (pp-number p sign)，
       但缺少指数数字，无法转换为有效的浮点常量，gcc -std=c99 应报错 */
    double w = 0x1p+;
    #endif

    return 0;
}