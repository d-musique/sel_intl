// The SEL extension library
// Free software published under the MIT license.

#if defined(_WIN32)

#include "intl_win32.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winnls.h>
#include <stringapiset.h>
#include <memory>

namespace sel
{
namespace intl
{

std::string string_from_wstring(std::wstring_view in)
{
    std::string str;
    int cb = WideCharToMultiByte(
        CP_UTF8, 0, in.data(), (int)in.size(),
        nullptr, 0, nullptr, nullptr);
    str.resize((unsigned int)cb);
    WideCharToMultiByte(
        CP_UTF8, 0, in.data(), (int)in.size(),
        str.data(), cb, nullptr, nullptr);
    return str;
}

std::wstring wstring_from_string(std::string_view in)
{
    std::wstring str;
    int cch = MultiByteToWideChar(
        CP_UTF8, 0, in.data(), (int)in.size(),
        nullptr, 0);
    str.resize((unsigned int)cch);
    MultiByteToWideChar(
        CP_UTF8, 0, in.data(), (int)in.size(),
        str.data(), cch);
    return str;
}

std::vector<std::string> get_preferred_ui_languages()
{
    std::vector<std::string> langs;
    DWORD count = 0;
    ULONG bufsize = 0;

    if (!GetUserPreferredUILanguages(
            MUI_LANGUAGE_NAME, &count, nullptr, &bufsize))
        return {};

    std::unique_ptr<WCHAR[]> buf(new WCHAR[bufsize]);

    if (!GetUserPreferredUILanguages(
            MUI_LANGUAGE_NAME, &count, buf.get(), &bufsize))
        return {};

    langs.resize(count);

    WCHAR *bufp = buf.get();
    for (DWORD i = 0; i < count; ++i)
    {
        std::wstring_view wstr(bufp, wcslen(bufp));
        std::string str = string_from_wstring(wstr);

        for (size_t i = 0; i < str.size(); ++i)
        {
            char c = str[i];
            if (c == '-')
                str[i] = '_';
            else if (c == '.' || c == '@')
                break;
        }

        langs[i] = std::move(str);
        bufp += wstr.size() + 1;
    }

    return langs;
};

}
// namespace intl
}
// namespace sel

#endif // defined(_WIN32)
