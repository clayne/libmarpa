/*
 * Copyright 2015 Jeffrey Kegler
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* Tests of Libmarpa methods on trivial grammar that is not merely nulling */

#include <stdio.h>
#include "marpa.h"

#include "marpa_m_test.h"

static int
warn (const char *s, Marpa_Grammar g)
{
  printf ("%s returned %d\n", s, marpa_g_error (g, NULL));
}

static int
fail (const char *s, Marpa_Grammar g)
{
  warn (s, g);
  exit (1);
}

Marpa_Symbol_ID S_top;
Marpa_Symbol_ID S_A1;
Marpa_Symbol_ID S_A2;
Marpa_Symbol_ID S_B1;
Marpa_Symbol_ID S_B2;
Marpa_Symbol_ID S_C1;
Marpa_Symbol_ID S_C2;

/* Longest rule is <= 4 symbols */
Marpa_Symbol_ID rhs[4];

Marpa_Rule_ID R_top_1;
Marpa_Rule_ID R_top_2;
Marpa_Rule_ID R_C2_3; // highest rule id

/* For (error) messages */
char msgbuf[80];

char *
symbol_name (Marpa_Symbol_ID id)
{
  if (id == S_top) return "top";
  if (id == S_A1) return "A1";
  if (id == S_A2) return "A2";
  if (id == S_B1) return "B1";
  if (id == S_B2) return "B2";
  if (id == S_C1) return "C1";
  if (id == S_C2) return "C2";
  sprintf (msgbuf, "no such symbol: %d", id);
  return msgbuf;
}

static Marpa_Grammar
marpa_g_simple_new(Marpa_Config *config)
{
  Marpa_Grammar g;
  g = marpa_g_new (config);
  if (!g)
    {
      Marpa_Error_Code errcode = marpa_c_error (config, NULL);
      printf ("marpa_g_new returned %d", errcode);
      exit (1);
    }

  ((S_top = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_A1 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_A2 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_B1 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_B2 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_C1 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);
  ((S_C2 = marpa_g_symbol_new (g)) >= 0)
    || fail ("marpa_g_symbol_new", g);

  /* 
   * top ::= A1
   * top ::= A2
   * A1  ::= B1
   * A2  ::= B2
   * B1  ::= C1
   * B2  ::= C2
   */
  rhs[0] = S_A1;
  ((R_top_1 = marpa_g_rule_new (g, S_top, rhs, 1)) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_A2;
  ((R_top_2 = marpa_g_rule_new (g, S_top, rhs, 1)) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_B1;
  (marpa_g_rule_new (g, S_A1, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_B2;
  (marpa_g_rule_new (g, S_A2, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_C1;
  (marpa_g_rule_new (g, S_B1, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);
  rhs[0] = S_C2;
  (marpa_g_rule_new (g, S_B2, rhs, 1) >= 0)
    || fail ("marpa_g_rule_new", g);

  return g;
}

static Marpa_Error_Code
marpa_g_simple_precompute(Marpa_Grammar g, Marpa_Symbol_ID S_start)
{
  Marpa_Error_Code rc;

  (marpa_g_start_symbol_set (g, S_start) >= 0)
    || fail ("marpa_g_start_symbol_set", g);

  rc = marpa_g_precompute (g);
  if (rc < 0)
    fail("marpa_g_precompute", g);

  return rc;
}

int
main (int argc, char *argv[])
{
  int rc;

  Marpa_Config marpa_configuration;

  Marpa_Grammar g;
  Marpa_Recognizer r;

  plan(20);

  marpa_c_init (&marpa_configuration);
  g = marpa_g_simple_new(&marpa_configuration);

  /* for marpa_g_error() in marpa_m_test() */
  marpa_m_grammar_set(g);

  marpa_g_simple_precompute(g, S_top);
  ok(1, "precomputation succeeded");

  r = marpa_r_new (g);
  if (!r)
    fail("marpa_r_new", g);

  Marpa_Symbol_ID S_token = S_A2;
  marpa_m_test("marpa_r_alternative", r, S_token, 0, 0,
    MARPA_ERR_RECCE_NOT_ACCEPTING_INPUT, "before marpa_r_start_input()");

  rc = marpa_r_start_input (r);
  if (!rc)
    fail("marpa_r_start_input", g);

  Marpa_Symbol_ID S_expected = S_A2;
  int value = 1;
  marpa_m_test("marpa_r_expected_symbol_event_set", r, S_expected, value, value);

  /* recognizer reading methods on invalid and missing symbols */
  marpa_m_test("marpa_r_alternative", r, S_invalid, 0, 0, MARPA_ERR_INVALID_SYMBOL_ID,
    "invalid token symbol is checked before no-such");
  marpa_m_test("marpa_r_alternative", r, S_no_such, 0, 0, MARPA_ERR_NO_SUCH_SYMBOL_ID,
    "no such token symbol");
  marpa_m_test("marpa_r_alternative", r, S_token, 0, 0,
    MARPA_ERR_TOKEN_LENGTH_LE_ZERO, marpa_m_error_message(MARPA_ERR_TOKEN_LENGTH_LE_ZERO));
  marpa_m_test("marpa_r_earleme_complete", r, -2, MARPA_ERR_PARSE_EXHAUSTED);

  /* re-create the recce and try some input */
  r = marpa_r_new (g);
  if (!r)
    fail("marpa_r_new", g);

  rc = marpa_r_start_input (r);
  if (!rc)
    fail("marpa_r_start_input", g);

  marpa_m_test("marpa_r_alternative", r, S_C1, 1, 1, MARPA_ERR_NONE);
  marpa_m_test("marpa_r_earleme_complete", r, 1);

  /* marpa_o_high_rank_only_* */
  Marpa_Bocage b = marpa_b_new(r, marpa_r_current_earleme(r));
  if(!b)
    fail("marpa_b_new", g);

  marpa_m_test("marpa_b_ambiguity_metric", b, 1);
  marpa_m_test("marpa_b_is_null", b, 0);

  Marpa_Order o = marpa_o_new (b);
  ok(o != NULL, "marpa_o_new(): ordering at earleme 0");

  int flag = 1;
  marpa_m_test("marpa_o_high_rank_only_set", o, flag, flag);
  marpa_m_test("marpa_o_high_rank_only", o, flag);

  marpa_m_test("marpa_o_ambiguity_metric", o, 1);
  marpa_m_test("marpa_o_is_null", o, 0);

  marpa_m_test("marpa_o_high_rank_only_set", o, flag, -2, MARPA_ERR_ORDER_FROZEN);
  marpa_m_test("marpa_o_high_rank_only", o, flag);

  return 0;
}
