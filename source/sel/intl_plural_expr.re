// -*- c++ -*-

// The SEL extension library
// Free software published under the MIT license.

/*!rules:re2c:Common
    re2c:flags:tags = 1;
    re2c:yyfill:enable = 0;
    re2c:eof = 0;
    re2c:api = custom;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE = int;
    re2c:define:YYPEEK       = "cursor < limit ? (unsigned char)*cursor : 0"; // "*cursor";
    re2c:define:YYSKIP       = "++cursor;";
    re2c:define:YYBACKUP     = "marker = cursor;";
    re2c:define:YYRESTORE    = "cursor = marker;";
    re2c:define:YYBACKUPCTX  = "ctxmarker = cursor;";
    re2c:define:YYRESTORECTX = "cursor = ctxmarker;";
    re2c:define:YYRESTORETAG = "cursor = ${tag};";
    re2c:define:YYLESSTHAN   = "limit - cursor < @@{len}";
    re2c:define:YYSTAGP      = "@@{tag} = cursor;";
    re2c:define:YYSTAGN      = "@@{tag} = NULL;";
    re2c:define:YYSHIFT      = "cursor += @@{shift};";
    re2c:define:YYSHIFTSTAG  = "@@{tag} += @@{shift};";
    any = .|"\n";
    ws = " "|"\f"|"\n"|"\r"|"\t"|"\v";
    uint = [0-9]+;
*/

static const char *get_next_token(
    const char *cursor, const char *limit, int *tok, std::unique_ptr<expr> &minor)
{
    const char *p1, *p2;
    /*!stags:re2c:Token format = 'const char *@@;\n'; */

    minor.reset();

    begin:

/*!local:re2c:Token
  !use:Common;

ws+ { goto begin; }

@p1 uint @p2
{
    uint64_t value = 0;
    for (const char *p = p1; p != p2; ++p)
    {
        unsigned int digit = *p - '0';
        if (value > UINT64_MAX / 10 || UINT64_MAX - 10 * value < digit)
            return nullptr;
        value = 10 * value + digit;
    }

    *tok = INTEGER;
    minor.reset(new expr(et_value, value));
    return cursor;
}

"(" { *tok = LPAREN; return cursor; }
")" { *tok = RPAREN; return cursor; }
"n" { *tok = VARN; return cursor; }
"==" { *tok = EQ; return cursor; }
"!=" { *tok = NE; return cursor; }
">=" { *tok = GE; return cursor; }
"<=" { *tok = LE; return cursor; }
">" { *tok = GT; return cursor; }
"<" { *tok = LT; return cursor; }
"+" { *tok = PLUS; return cursor; }
"-" { *tok = MINUS; return cursor; }
"*" { *tok = TIMES; return cursor; }
"/" { *tok = DIVIDE; return cursor; }
"%" { *tok = MOD; return cursor; }
"&&" { *tok = AND; return cursor; }
"||" { *tok = OR; return cursor; }
"!" { *tok = NOT; return cursor; }
"?" { *tok = QUESTION; return cursor; }
":" { *tok = COLON; return cursor; }

$ { *tok = 0; return cursor; }
any { return nullptr; }

*/
}
