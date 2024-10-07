// The SEL extension library
// Free software published under the MIT license.

#include "intl_plural_expr.hpp"
#include <memory>
#include <assert.h>

namespace
{

struct expr
{
    explicit expr(
        int type, uint64_t value = 0,
        expr *a = nullptr, expr *b = nullptr, expr *c = nullptr) noexcept
        : m_type(type), m_value(value), m_a(a), m_b(b), m_c(c) {}
    int m_type = 0;
    uint64_t m_value = 0;
    std::unique_ptr<expr> m_a, m_b, m_c;
};

enum expr_type : int
{
    et_value = 1,
    et_var_n,
    et_eq,
    et_ne,
    et_ge,
    et_le,
    et_gt,
    et_lt,
    et_plus,
    et_minus,
    et_times,
    et_divide,
    et_mod,
    et_and,
    et_or,
    et_not,
    et_ternary,
};

enum expr_flag
{
    ef_eval_b_if_a = 1 << 0,
    ef_eval_b_if_not_a = 1 << 1,
    ef_eval_c_if_not_a = 1 << 2,
};

struct expr_properties
{
    unsigned int m_num_op;
    unsigned int m_flags;
    bool (*m_calc)(uint64_t a, uint64_t b, uint64_t c, uint64_t v, uint64_t n, uint64_t *r);
};

#define expr_debug(f, ...)
//#define expr_debug(f, ...) fprintf(stderr, "%s\n", std::format((f), __VA_ARGS__).c_str())

expr_properties get_expr_properties(int type)
{
    expr_properties prop;

    switch (type)
    {
    default:
        assert(false);
        prop.m_num_op = 0;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t *) -> bool
        {
            return false;
        };
        break;

    case et_value:
        prop.m_num_op = 0;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t, uint64_t, uint64_t, uint64_t v, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("val {}", v);
            *r = v;
            return true;
        };
        break;

    case et_var_n:
        prop.m_num_op = 0;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t, uint64_t, uint64_t, uint64_t, uint64_t n, uint64_t *r) -> bool
        {
            expr_debug("var {}", n);
            *r = n;
            return true;
        };
        break;

    case et_eq:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} == {}", a, b);
            *r = a == b;
            return true;
        };
        break;

    case et_ne:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} != {}", a, b);
            *r = a != b;
            return true;
        };
        break;

    case et_ge:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} >= {}", a, b);
            *r = a >= b;
            return true;
        };
        break;

    case et_le:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} <= {}", a, b);
            *r = a <= b;
            return true;
        };
        break;

    case et_gt:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} > {}", a, b);
            *r = a > b;
            return true;
        };
        break;

    case et_lt:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} < {}", a, b);
            *r = a < b;
            return true;
        };
        break;

    case et_plus:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} + {}", a, b);
            *r = a + b;
            return true;
        };
        break;

    case et_minus:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} - {}", a, b);
            *r = a - b;
            return true;
        };
        break;

    case et_times:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} * {}", a, b);
            *r = a * b;
            return true;
        };
        break;

    case et_divide:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} / {}", a, b);
            if (b == 0) return false;
            *r = a / b;
            return true;
        };
        break;

    case et_mod:
        prop.m_num_op = 2;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} % {}", a, b);
            if (b == 0) return false;
            *r = a % b;
            return true;
        };
        break;

    case et_and:
        prop.m_num_op = 2;
        prop.m_flags = ef_eval_b_if_a;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} && {}", a, b);
            *r = a && b;
            return true;
        };
        break;

    case et_or:
        prop.m_num_op = 2;
        prop.m_flags = ef_eval_b_if_not_a;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} || {}", a, b);
            *r = a || b;
            return true;
        };
        break;

    case et_not:
        prop.m_num_op = 1;
        prop.m_flags = 0;
        prop.m_calc = [](uint64_t a, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("! {}", a);
            *r = !a;
            return true;
        };
        break;

    case et_ternary:
        prop.m_num_op = 3;
        prop.m_flags = ef_eval_b_if_a|ef_eval_c_if_not_a;
        prop.m_calc = [](uint64_t a, uint64_t b, uint64_t c, uint64_t, uint64_t, uint64_t *r) -> bool
        {
            expr_debug("{} ? {} : {}", a, b, c);
            *r = a ? b : c;
            return true;
        };
        break;
    }

    return prop;
}

struct plural_expr_yyextra
{
    bool m_error = false;
    std::unique_ptr<expr> m_result;
};

struct plural_expr_yycontext
{
};

}
// namespace

[[maybe_unused]] static void *plural_expr_syntaxAlloc(void *(*)(size_t), plural_expr_yycontext *);
[[maybe_unused]] static void plural_expr_syntaxFree(void *, void (*)(void *));
[[maybe_unused]] static void plural_expr_syntaxInit(void *, plural_expr_yycontext *);
[[maybe_unused]] static void plural_expr_syntaxFinalize(void *);
[[maybe_unused]] static void plural_expr_syntax(void *, int, expr *, plural_expr_yyextra *);
[[maybe_unused]] static void plural_expr_syntaxTrace(FILE *, char *);
[[maybe_unused]] static int plural_expr_syntaxFallback(int);

#include "intl_plural_expr.yy.ipp"
#include "intl_plural_expr.re.ipp"

static std::unique_ptr<expr> parse_expr(std::string_view text)
{
    plural_expr_yycontext context;
    plural_expr_syntax_parser *parser = new plural_expr_syntax_parser;
    plural_expr_syntaxInit(parser, &context);

    struct parser_delete
    {
        void operator()(plural_expr_syntax_parser *parser) const noexcept
        {
            plural_expr_syntaxFinalize(parser);
            delete parser;
        }
    };
    std::unique_ptr<plural_expr_syntax_parser, parser_delete> parser_cleanup(parser);

    const char *cursor = text.data();
    const char *limit = cursor + text.size();

    int tok;
    std::unique_ptr<expr> minor;
    plural_expr_yyextra extra;

    for (bool done = false; !done; done = !tok)
    {
        cursor = get_next_token(cursor, limit, &tok, minor);
        if (!cursor)
            return nullptr;

        plural_expr_syntax(parser, tok, minor.release(), &extra);
        if (extra.m_error)
            return nullptr;
    }

    std::unique_ptr<expr> ex = std::move(extra.m_result);
    assert(ex != nullptr);
    return ex;
}

//------------------------------------------------------------------------------

namespace sel
{
namespace intl
{

struct plural_expr::internal
{
    std::unique_ptr<expr> m_ex;
    static bool eval(const expr *e, unsigned int level, unsigned int max_level, uint64_t x, uint64_t *r);
};

plural_expr::plural_expr(std::string_view text)
    : m_priv(new internal)
{
    m_priv->m_ex = parse_expr(text);
}

bool plural_expr::valid() const noexcept
{
    return m_priv && m_priv->m_ex != nullptr;
}

bool plural_expr::eval(uint64_t n, uint64_t *r, unsigned int max_level)
{
    if (!valid())
        return false;

    return m_priv->eval(m_priv->m_ex.get(), 0, max_level, n, r);
}

bool plural_expr::internal::eval(
    const expr *ex, unsigned int level, unsigned int max_level, uint64_t x, uint64_t *r)
{
    assert(ex != nullptr);
    assert(r != nullptr);

    if (level >= max_level)
        return false;

    const expr_properties prop = get_expr_properties(ex->m_type);

    uint64_t a = 0, b = 0, c = 0;

    if (prop.m_num_op >= 1)
    {
        if (!eval(ex->m_a.get(), level + 1, max_level, x, &a))
            return false;
    }
    if (prop.m_num_op >= 2)
    {
        bool e = true;
        if (prop.m_flags & ef_eval_b_if_a) e = a;
        if (prop.m_flags & ef_eval_b_if_not_a) e = !a;
        if (e && !eval(ex->m_b.get(), level + 1, max_level, x, &b))
            return false;
    }
    if (prop.m_num_op >= 3)
    {
        bool e = true;
        if (prop.m_flags & ef_eval_c_if_not_a) e = !a;
        if (e && !eval(ex->m_c.get(), level + 1, max_level, x, &c))
            return false;
    }

    return prop.m_calc(a, b, c, ex->m_value, x, r);
}

void plural_expr::internal_delete::operator()(internal *x) const noexcept
{
    delete x;
}

}
// namespace intl
}
// namespace sel
