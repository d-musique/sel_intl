// The SEL extension library
// Free software published under the MIT license.

#pragma once
#if !defined(SEL_INTL_PLURAL_EXPR_HPP_INCLUDED)
#define SEL_INTL_PLURAL_EXPR_HPP_INCLUDED

#include <string_view>
#include <memory>
#include <stdint.h>

namespace sel
{
namespace intl
{

class plural_expr
{
public:
    plural_expr() noexcept = default;
    explicit plural_expr(std::string_view text);
    bool valid() const noexcept;
    bool eval(uint64_t n, uint64_t *r, unsigned int max_level = 64);
    explicit operator bool() const noexcept { return valid(); }

private:
    struct internal;
    struct internal_delete { void operator()(internal *x) const noexcept; };
    std::unique_ptr<internal, internal_delete> m_priv;
};

}
// namespace intl
}
// namespace sel

#endif // !defined(SEL_INTL_PLURAL_EXPR_HPP_INCLUDED)
