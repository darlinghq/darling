#include "../include/Calculate/Calculate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <locale.h>
#include <pthread.h>

#ifdef __APPLE__
#include <xlocale.h>
#endif

typedef struct {
    const char *p;
    bool has_error;
    int depth;
} CalcParser;

static locale_t g_c_locale = NULL;
static pthread_once_t g_c_locale_once = PTHREAD_ONCE_INIT;

static void init_c_locale(void) {
    g_c_locale = newlocale(LC_ALL_MASK, "C", NULL);
}

static double parse_number_c(const char *nptr, char **endptr) {
    pthread_once(&g_c_locale_once, init_c_locale);
    if (g_c_locale) {
        return strtod_l(nptr, endptr, g_c_locale);
    }
    return strtod(nptr, endptr);
}

static void skip_spaces(CalcParser *cp) {
    while (*cp->p && isspace((unsigned char)*cp->p)) {
        cp->p++;
    }
}

static double parse_expr(CalcParser *cp);

static double factorial(double n) {
    if (n < 0 || n != floor(n) || n > 170) return NAN;
    double r = 1.0;
    int k = (int)n;
    for (int i = 2; i <= k; i++) r *= i;
    return r;
}

static double parse_factor(CalcParser *cp) {
    skip_spaces(cp);
    if (!*cp->p) {
        cp->has_error = true;
        return 0;
    }

    if (*cp->p == '+') {
        cp->p++;
        return parse_factor(cp);
    }
    if (*cp->p == '-') {
        cp->p++;
        return -parse_factor(cp);
    }

    if (*cp->p == '(') {
        cp->p++;
        double val = parse_expr(cp);
        skip_spaces(cp);
        if (*cp->p == ')') {
            cp->p++;
        } else {
            cp->has_error = true;
        }
        skip_spaces(cp);
        if (*cp->p == '!') {
            cp->p++;
            val = factorial(val);
        }
        return val;
    }

    if (isalpha((unsigned char)*cp->p)) {
        char fn[32];
        int len = 0;
        while (isalpha((unsigned char)*cp->p) && len < 31) {
            fn[len++] = *cp->p++;
        }
        fn[len] = '\0';
        skip_spaces(cp);

        double arg = 0;
        if (*cp->p == '(') {
            cp->p++;
            arg = parse_expr(cp);
            skip_spaces(cp);
            if (*cp->p == ')') cp->p++;
            else cp->has_error = true;
        } else {
            arg = parse_factor(cp);
        }

        if (strcasecmp(fn, "sqrt") == 0) return sqrt(arg);
        if (strcasecmp(fn, "cbrt") == 0) return cbrt(arg);
        if (strcasecmp(fn, "sin") == 0) return sin(arg);
        if (strcasecmp(fn, "cos") == 0) return cos(arg);
        if (strcasecmp(fn, "tan") == 0) return tan(arg);
        if (strcasecmp(fn, "asin") == 0) return asin(arg);
        if (strcasecmp(fn, "acos") == 0) return acos(arg);
        if (strcasecmp(fn, "atan") == 0) return atan(arg);
        if (strcasecmp(fn, "ln") == 0) return log(arg);
        if (strcasecmp(fn, "log") == 0) return log10(arg);
        if (strcasecmp(fn, "log2") == 0) return log2(arg);
        if (strcasecmp(fn, "exp") == 0) return exp(arg);
        if (strcasecmp(fn, "abs") == 0) return fabs(arg);
        if (strcasecmp(fn, "fact") == 0) return factorial(arg);

        cp->has_error = true;
        return 0;
    }

    char *end = NULL;
    double val = parse_number_c(cp->p, &end);
    if (end == cp->p) {
        cp->has_error = true;
        return 0;
    }
    cp->p = end;

    skip_spaces(cp);
    if (*cp->p == '!') {
        cp->p++;
        val = factorial(val);
    }
    return val;
}

static double parse_power(CalcParser *cp) {
    double base = parse_factor(cp);
    skip_spaces(cp);
    if (*cp->p == '^') {
        cp->p++;
        double exp_val = parse_power(cp);
        base = pow(base, exp_val);
    }
    return base;
}

static double parse_term(CalcParser *cp) {
    double left = parse_power(cp);
    while (!cp->has_error) {
        skip_spaces(cp);
        char op = *cp->p;
        if (op != '*' && op != '/' && op != '%') break;
        cp->p++;
        double right = parse_power(cp);
        if (op == '*') {
            left *= right;
        } else if (op == '/') {
            if (right == 0.0) {
                cp->has_error = true;
                return 0;
            }
            left /= right;
        } else if (op == '%') {
            if (right == 0.0) {
                cp->has_error = true;
                return 0;
            }
            left = fmod(left, right);
        }
    }
    return left;
}

static double parse_expr(CalcParser *cp) {
    if (++cp->depth > 256) {
        cp->has_error = true;
        return 0;
    }
    double left = parse_term(cp);
    while (!cp->has_error) {
        skip_spaces(cp);
        char op = *cp->p;
        if (op != '+' && op != '-') break;
        cp->p++;
        double right = parse_term(cp);
        if (op == '+') left += right;
        else left -= right;
    }
    cp->depth--;
    return left;
}

bool CalculateExpression(const char *expr, int base, int flags, char *outBuffer, int bufferSize) {
    if (!expr || !outBuffer || bufferSize <= 0) {
        return false;
    }

    CalcParser cp = { .p = expr, .has_error = false };
    double result = parse_expr(&cp);
    skip_spaces(&cp);
    if (cp.has_error || *cp.p != '\0' || isnan(result) || isinf(result)) {
        return false;
    }

    if (base == 16) {
        if (fabs(result) > 9e18) return false;
        snprintf(outBuffer, bufferSize, "%llX", (unsigned long long)(long long)result);
    } else if (base == 8) {
        if (fabs(result) > 9e18) return false;
        snprintf(outBuffer, bufferSize, "%llo", (unsigned long long)(long long)result);
    } else {
        if (result == floor(result) && fabs(result) < 1e15) {
            snprintf(outBuffer, bufferSize, "%.0f", result);
        } else {
            snprintf(outBuffer, bufferSize, "%.10g", result);
        }
    }

    return true;
}
