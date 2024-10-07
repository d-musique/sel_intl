// The SEL extension library
// Free software published under the MIT license.

#pragma once
#if !defined(SEL_INTL_H_INCLUDED)
#define SEL_INTL_H_INCLUDED

// NOTE: this gettext runtime only supports UTF-8 encoding

#if defined(__cplusplus)
extern "C" {
#endif

const char *sel_gettext(const char *text);
const char *sel_dgettext(const char *domain, const char *text);
const char *sel_dcgettext(const char *domain, const char *text, int category);
const char *sel_ngettext(const char *text, const char *plural, unsigned long n);
const char *sel_dngettext(const char *domain, const char *text, const char *plural, unsigned long n);
const char *sel_dcngettext(const char *domain, const char *text, const char *plural, unsigned long n, int category);
const char *sel_bindtextdomain(const char *domain, const char *dirname);
const char *sel_textdomain(const char *domain);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // !defined(SEL_INTL_H_INCLUDED)
