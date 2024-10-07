// The SEL extension library
// Free software published under the MIT license.

#pragma once
#if !defined(SEL_INTL_CATALOG_HPP_INCLUDED)
#define SEL_INTL_CATALOG_HPP_INCLUDED

#include "intl_plural_expr.hpp"
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <stdint.h>
#include <stddef.h>

namespace sel
{
namespace intl
{

struct catalog_entry
{
    char *m_source = nullptr;
    char *m_translated = nullptr;
    uint32_t m_extra_plurals = 0;
    char *get_plural(uint64_t nth) const noexcept;
};

struct catalog_key
{
    int m_category = 0;
    std::string_view m_message;
};

struct catalog_key_hash
{
    size_t operator()(const catalog_key &key) const noexcept;
};

struct catalog_key_equal
{
    bool operator()(const catalog_key &a, const catalog_key &b) const noexcept;
};

struct plural_forms
{
    unsigned int m_num_plurals{};
    plural_expr m_expr_plural;
};

struct catalog
{
    std::string m_domain;
    std::string m_dir;
    volatile uint32_t m_loaded = 0;
    std::unique_ptr<char[]> m_blob;
    std::unique_ptr<plural_forms> m_plural;
    std::unordered_map<catalog_key, catalog_entry, catalog_key_hash, catalog_key_equal> m_strings;
    const char *lookup(const char *text, int category);
    const char *plural_lookup(const char *text, const char *plural, unsigned long n, int category);
    bool load(int category, std::string_view lang, std::shared_lock<std::shared_mutex> &shared_lock);
    bool load_file_strings(const std::string &path, int category);
    static std::string_view string_of_category(int category);
};

}
// namespace intl
}
// namespace sel

#endif // !defined(SEL_INTL_CATALOG_HPP_INCLUDED)
