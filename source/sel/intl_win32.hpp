// The SEL extension library
// Free software published under the MIT license.

#pragma once
#if !defined(SEL_INTL_WIN32_HPP_INCLUDED)
#define SEL_INTL_WIN32_HPP_INCLUDED

#if defined(_WIN32)

#include <vector>
#include <string>
#include <string_view>
#include <locale.h>

#if defined(LC_MESSAGES)
#error LC_MESSAGES should not be defined on Windows
#endif
#if defined(_WIN32)
#define LC_MESSAGES 31
#endif

namespace sel
{
namespace intl
{

std::string string_from_wstring(std::wstring_view in);
std::wstring wstring_from_string(std::string_view in);

std::vector<std::string> get_preferred_ui_languages();

}
// namespace intl
}
// namespace sel

#endif // defined(_WIN32)

#endif // !defined(SEL_INTL_WIN32_HPP_INCLUDED)
