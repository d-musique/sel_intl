%name plural_expr_syntax
%include {#define yyParser plural_expr_syntax_parser}
%start_symbol program
%token_type {expr *}
%extra_argument {plural_expr_yyextra *extra}
%extra_context {plural_expr_yycontext *context}
%default_destructor {delete $$; (void)extra; (void)context;}
%token_destructor {delete $$; (void)extra; (void)context;}
%syntax_error {(void)yymajor; (void)yyminor; (void)extra; (void)context;}
%parse_failure {extra->m_error = true;}

%right QUESTION COLON.
%left OR.
%left AND.
%left EQ NE.
%left GE LE GT LT.
%left PLUS MINUS.
%left TIMES DIVIDE MOD.
%nonassoc NOT.

program ::= expr(A). { extra->m_result.reset(A); (void)context; }
expr(R) ::= LPAREN expr(A) RPAREN. { R = A; }
expr(R) ::= INTEGER(A). { R = A; }
expr(R) ::= VARN. { R = new expr(et_var_n); }
expr(R) ::= expr(A) EQ expr(B). { R = new expr(et_eq, 0, A, B); }
expr(R) ::= expr(A) NE expr(B). { R = new expr(et_ne, 0, A, B); }
expr(R) ::= expr(A) GE expr(B). { R = new expr(et_ge, 0, A, B); }
expr(R) ::= expr(A) LE expr(B). { R = new expr(et_le, 0, A, B); }
expr(R) ::= expr(A) GT expr(B). { R = new expr(et_gt, 0, A, B); }
expr(R) ::= expr(A) LT expr(B). { R = new expr(et_lt, 0, A, B); }
expr(R) ::= expr(A) PLUS expr(B). { R = new expr(et_plus, 0, A, B); }
expr(R) ::= expr(A) MINUS expr(B). { R = new expr(et_minus, 0, A, B); }
expr(R) ::= expr(A) TIMES expr(B). { R = new expr(et_times, 0, A, B); }
expr(R) ::= expr(A) DIVIDE expr(B). { R = new expr(et_divide, 0, A, B); }
expr(R) ::= expr(A) MOD expr(B). { R = new expr(et_mod, 0, A, B); }
expr(R) ::= expr(A) OR expr(B). { R = new expr(et_or, 0, A, B); }
expr(R) ::= expr(A) AND expr(B). { R = new expr(et_and, 0, A, B); }
expr(R) ::= NOT expr(A). { R = new expr(et_not, 0, A); }
expr(R) ::= expr(A) QUESTION expr(B) COLON expr(C). { R = new expr(et_ternary, 0, A, B, C); }
