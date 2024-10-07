// The SEL extension library
// Free software published under the MIT license.

#include "sel/intl.h"
#include "intl_catalog.hpp"
#include <string>
#include <string_view>
#include <map>
#include <optional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <locale.h>

#if defined(_WIN32)
#include "intl_win32.hpp"
#endif

namespace sel
{
namespace intl
{

struct catalog;

struct intl
{
    static intl &get();

    const char *gettext(const char *domain, const char *text, int category);
    const char *ngettext(const char *domain, const char *text, const char *plural, unsigned long n, int category);
    const char *bindtextdomain(std::string_view domain, const char *dirname);
    const char *textdomain(const char *domain);

    catalog *find_catalog(std::string_view domain);
    std::string_view get_category_language(int category);

    std::shared_mutex m_mutex;
    std::string m_current_domain;
    std::map<std::string_view, std::unique_ptr<catalog>> m_domains;

#if !defined(_WIN32)
    std::optional<std::string> m_category_language[32];
#else
    std::optional<std::string> m_language;
#endif
};

intl &intl::get()
{
    static intl instance;
    return instance;
}

const char *intl::gettext(const char *domain, const char *text, int category)
{
    if (!text)
        text = "";

    if (!text[0])
        return text;
    if (category < 0 || category >= 32)
        return text;

    catalog *cat = nullptr;
    std::shared_lock<std::shared_mutex> shared_lock(m_mutex);
    cat = find_catalog(domain ? std::string_view(domain) : m_current_domain);

    if (!cat)
        return text;

    cat->load(category, get_category_language(category), shared_lock);

    const char *translated = cat->lookup(text, category);
    if (!translated)
        return text;

    return translated;
}

const char *intl::ngettext(const char *domain, const char *text, const char *plural, unsigned long n, int category)
{
    if (!text)
        text = "";
    if (!plural)
        plural = "";

    if (!text[0])
        return (n == 1) ? text : plural;
    if (category < 0 || category >= 32)
        return (n == 1) ? text : plural;

    catalog *cat = nullptr;
    std::shared_lock<std::shared_mutex> shared_lock(m_mutex);
    cat = find_catalog(domain ? std::string_view(domain) : m_current_domain);

    if (!cat)
        return (n == 1) ? text : plural;

    cat->load(category, get_category_language(category), shared_lock);

    const char *translated = cat->plural_lookup(text, plural, n, category);
    if (!translated)
        return (n == 1) ? text : plural;

    return translated;
}

const char *intl::bindtextdomain(std::string_view domain, const char *dirname)
{
    catalog *cat = nullptr;
    std::lock_guard<std::shared_mutex> lock(m_mutex);
    cat = find_catalog(domain);

    if (!cat)
    {
        std::unique_ptr<catalog> key(new catalog);
        key->m_domain.assign(domain);
        cat = m_domains.insert(
            std::make_pair(std::string_view(key->m_domain), std::move(key)))
            .first->second.get();
    }

    cat->m_dir.assign(dirname);
    return cat->m_dir.c_str();
}

const char *intl::textdomain(const char *domain)
{
    std::lock_guard<std::shared_mutex> lock(m_mutex);
    m_current_domain.assign(domain);
    return m_current_domain.c_str();
}

catalog *intl::find_catalog(std::string_view domain)
{
    auto it = m_domains.find(domain);
    return (it != m_domains.end()) ? it->second.get() : nullptr;
}

#if defined(_WIN32)

std::string_view intl::get_category_language(int category)
{
    (void)category;

    if (!m_language)
    {
        const std::vector<std::string> languages = get_preferred_ui_languages();

        //TODO select the best language
        std::string lang;
        if (!languages.empty())
            lang = languages.front();

        m_language = std::move(lang);
    }

    return *m_language;
}

#else

std::string_view intl::get_category_language(int category)
{
    if (category < 0 || category >= 32)
        return {};

    if (!m_category_language[category])
    {
        std::string lang;
        if (const char *value = setlocale(category, nullptr))
            lang.assign(value);

        m_category_language[category] = std::move(lang);
    }

    return *m_category_language[category];
}

#endif

}
// namespace intl
}
// namespace sel

extern "C"
{

const char *sel_gettext(const char *text)
{
    return sel_dcgettext(nullptr, text, LC_MESSAGES);
}

const char *sel_dgettext(const char *domain, const char *text)
{
    return sel_dcgettext(domain, text, LC_MESSAGES);
}

const char *sel_dcgettext(const char *domain, const char *text, int category)
{
    return sel::intl::intl::get().gettext(domain, text, category);
}

const char *sel_ngettext(const char *text, const char *plural, unsigned long n)
{
    return sel_dcngettext({}, text, plural, n, LC_MESSAGES);
}

const char *sel_dngettext(const char *domain, const char *text, const char *plural, unsigned long n)
{
    return sel_dcngettext(domain, text, plural, n, LC_MESSAGES);
}

const char *sel_dcngettext(const char *domain, const char *text, const char *plural, unsigned long n, int category)
{
    return sel::intl::intl::get().ngettext(domain, text, plural, n, category);
}

extern const char *sel_bindtextdomain(const char *domain, const char *dirname)
{
    return sel::intl::intl::get().bindtextdomain(domain, dirname);
}

extern const char *sel_textdomain(const char *domain)
{
    return sel::intl::intl::get().textdomain(domain);
}

}
// extern "C"
