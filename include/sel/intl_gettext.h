// The SEL extension library
// Free software published under the MIT license.

#pragma once
#if !defined(SEL_INTL_GETTEXT_H_INCLUDED)
#define SEL_INTL_GETTEXT_H_INCLUDED

#include "intl.h"

#define gettext sel_gettext
#define dgettext sel_dgettext
#define dcgettext sel_dcgettext
#define ngettext sel_ngettext
#define dngettext sel_dngettext
#define dcngettext sel_dcngettext
#define bindtextdomain sel_bindtextdomain
#define textdomain sel_textdomain

#endif // !defined(SEL_INTL_GETTEXT_H_INCLUDED)
