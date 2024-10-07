// The SEL extension library
// Free software published under the MIT license.

#include <doctest/doctest.h>
#include "sel/intl_catalog.hpp"
#include "sel/intl_plural_expr.hpp"
#include <string_view>
#include <stdint.h>

#if defined(_WIN32)
#include "sel/intl_win32.hpp"
#endif

using namespace std::literals::string_view_literals;

TEST_CASE("Intl: simple catalog")
{
    sel::intl::catalog cat;
    int category = LC_MESSAGES;

    REQUIRE(cat.load_file_strings(SEL_TEST_DIR "/catalog-simple.mo", category));
    cat.m_loaded = 1u << category;

    const char *msgid;
    const char *msgstr;

    msgid = "A message in english";
    msgstr = cat.lookup(msgid, category);
    REQUIRE(msgstr == "Un message en français"sv);

    msgid = "Another message in english";
    msgstr = cat.lookup(msgid, category);
    REQUIRE(msgstr == "Un autre message en français"sv);

    msgid = "A message not in the catalog";
    msgstr = cat.lookup(msgid, category);
    REQUIRE(msgstr == nullptr);
}

TEST_CASE("Intl: plural catalog")
{
    sel::intl::catalog cat;
    int category = LC_MESSAGES;

    REQUIRE(cat.load_file_strings(SEL_TEST_DIR "/catalog-plural.mo", category));
    cat.m_loaded = 1u << category;

    const char *msgid;
    const char *msgid_plural;
    const char *msgstr;
    unsigned long n;

    msgid = "I have one apple.";
    msgid_plural = "I have {} apples.";
    n = 0;
    msgstr = cat.plural_lookup(msgid, msgid_plural, n, category);
    REQUIRE(msgstr == "J'ai {} pommes."sv);
    n = 1;
    msgstr = cat.plural_lookup(msgid, msgid_plural, n, category);
    REQUIRE(msgstr == "J'ai une pomme."sv);
    n = 2;
    msgstr = cat.plural_lookup(msgid, msgid_plural, n, category);
    REQUIRE(msgstr == "J'ai deux pommes."sv);
    n = 3;
    msgstr = cat.plural_lookup(msgid, msgid_plural, n, category);
    REQUIRE(msgstr == "J'ai {} pommes."sv);

    msgid = "A singular message not in the catalog";
    msgid_plural = "A plural message not in the catalog";
    msgstr = cat.plural_lookup(msgid, msgid_plural, n, category);
    n = 0;
    REQUIRE(msgstr == nullptr);
}

TEST_CASE("Intl: plural expression operations")
{
    {
        sel::intl::plural_expr expr("n+100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(1, &r));
        REQUIRE(r == 101);
    }
    {
        sel::intl::plural_expr expr("n-100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(101, &r));
        REQUIRE(r == 1);
    }
    {
        sel::intl::plural_expr expr("n*2");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(123, &r));
        REQUIRE(r == 246);
    }
    {
        sel::intl::plural_expr expr("n/2");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(123, &r));
        REQUIRE(r == 61);
    }
    {
        sel::intl::plural_expr expr("2/n");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(!expr.eval(0, &r));
    }
    {
        sel::intl::plural_expr expr("n%100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(1234, &r));
        REQUIRE(r == 34);
    }
    {
        sel::intl::plural_expr expr("2%n");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(!expr.eval(0, &r));
    }
    {
        sel::intl::plural_expr expr("n==100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 0);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 1);
    }
    {
        sel::intl::plural_expr expr("n!=100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 1);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 0);
    }
    {
        sel::intl::plural_expr expr("n>=100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(99, &r));
        REQUIRE(r == 0);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 1);
    }
    {
        sel::intl::plural_expr expr("n<=100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 1);
        REQUIRE(expr.eval(101, &r));
        REQUIRE(r == 0);
    }
    {
        sel::intl::plural_expr expr("n>100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 0);
        REQUIRE(expr.eval(101, &r));
        REQUIRE(r == 1);
    }
    {
        sel::intl::plural_expr expr("n<100");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(99, &r));
        REQUIRE(r == 1);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 0);
    }
    {
        sel::intl::plural_expr expr("n&&(n/0)");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 0);
        REQUIRE(!expr.eval(100, &r));
    }
    {
        sel::intl::plural_expr expr("n||(n/0)");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 1);
        REQUIRE(!expr.eval(0, &r));
    }
    {
        sel::intl::plural_expr expr("!n");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 1);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 0);
    }
    {
        sel::intl::plural_expr expr("n?123:(n/0)");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(100, &r));
        REQUIRE(r == 123);
        REQUIRE(!expr.eval(0, &r));
    }
    {
        sel::intl::plural_expr expr("n?(n/0):123");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(!expr.eval(100, &r));
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 123);
    }
}

TEST_CASE("Intl: plural expression precedence")
{
    {
        sel::intl::plural_expr expr("2+6/3");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 4);
    }
    {
        sel::intl::plural_expr expr("(2+6)/3");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 2);
    }
    {
        sel::intl::plural_expr expr("n?2:3?4:5");
        uint64_t r{};
        REQUIRE(expr);
        REQUIRE(expr.eval(1, &r));
        REQUIRE(r == 2);
        REQUIRE(expr.eval(0, &r));
        REQUIRE(r == 4);
    }
}
