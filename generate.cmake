#!/usr/bin/cmake -P
# The SEL extension library
# Free software published under the MIT license.

include("cmake/generators/re2c.cmake")
include("cmake/generators/lemon.cmake")
generate_re2c("source/sel/intl_plural_expr.re" ".ipp")
generate_lemon("source/sel/intl_plural_expr.yy" ".ipp")
