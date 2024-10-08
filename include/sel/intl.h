// The SEL extension library
// Free software published under the MIT license.

#pragma once
#if !defined(SEL_INTL_H_INCLUDED)
#define SEL_INTL_H_INCLUDED

// NOTE: this gettext runtime only supports UTF-8 encoding

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__GNUC__)
#define SEL_INTL_FORMAT_ARG(n) __attribute__((format_arg(n)))
#else
#define SEL_INTL_FORMAT_ARG(n)
#endif

const char *sel_gettext(const char *text) SEL_INTL_FORMAT_ARG(1);
const char *sel_dgettext(const char *domain, const char *text) SEL_INTL_FORMAT_ARG(2);
const char *sel_dcgettext(const char *domain, const char *text, int category) SEL_INTL_FORMAT_ARG(2);
const char *sel_ngettext(const char *text, const char *plural, unsigned long n) SEL_INTL_FORMAT_ARG(1) SEL_INTL_FORMAT_ARG(2);
const char *sel_dngettext(const char *domain, const char *text, const char *plural, unsigned long n) SEL_INTL_FORMAT_ARG(2) SEL_INTL_FORMAT_ARG(3);
const char *sel_dcngettext(const char *domain, const char *text, const char *plural, unsigned long n, int category) SEL_INTL_FORMAT_ARG(2) SEL_INTL_FORMAT_ARG(3);
const char *sel_bindtextdomain(const char *domain, const char *dirname);
const char *sel_textdomain(const char *domain);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // !defined(SEL_INTL_H_INCLUDED)
