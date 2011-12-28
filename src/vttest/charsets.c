/* $Id: charsets.c,v 1.34 2010/05/28 09:20:59 tom Exp $ */

/*
 * Test character-sets (e.g., SCS control, DECNRCM mode)
 */
#include <vttest.h>
#include <esc.h>

/* the values, where specified, correspond to the keyboard-language codes */
typedef enum {
  ASCII = 1,
  British = 2,
  Flemish = 3,
  French_Canadian = 4,
  Danish = 5,
  Finnish = 6,
  German = 7,
  Dutch = 8,
  Italian = 9,
  Swiss_French = 10,
  Swiss_German = 11,
  Swiss,
  Swedish = 12,
  Norwegian_Danish = 13,
  French = 14,
  Spanish = 15,
  Portugese = 16,
  Hebrew = 17,
  British_Latin_1,
  Cyrillic,
  DEC_Spec_Graphic,
  DEC_Supp,
  DEC_Supp_Graphic,
  DEC_Tech,
  Greek,
  Greek_Supp,
  Hebrew_Supp,
  Latin_5_Supp,
  Latin_Cyrillic,
  Russian,
  Turkish,
  SCS_NRCS,
  Unknown
} National;
/* *INDENT-OFF* */
static const struct {
  National code;  /* internal name (chosen to sort 'name' member) */
  int allow96;    /* flag for 96-character sets (e.g., GR mapping) */
  int order;      /* check-column so we can mechanically-sort this table */
  int model;      /* 0=base, 2=vt220, 3=vt320, etc. */
  const char *final;    /* end of SCS string */
  const char *name;     /* the string we'll show the user */
} KnownCharsets[] = {
  { ASCII,             0, 0, 0, "B",    "US ASCII" },
  { British,           0, 0, 0, "A",    "British" },
  { British_Latin_1,   1, 0, 3, "A",    "ISO Latin-1" },
  { Cyrillic,          0, 0, 5, "&4",   "Cyrillic (DEC)" },
  { DEC_Spec_Graphic,  0, 0, 0, "0",    "DEC Special Graphics" },
  { DEC_Supp,          0, 0, 2, "<",    "DEC Supplemental" },
  { DEC_Supp_Graphic,  0, 0, 3, "%5",   "DEC Supplemental Graphic" },
  { DEC_Tech,          0, 0, 3, ">",    "DEC Technical" },
  { Dutch,             0, 0, 2, "4",    "Dutch" },
  { Finnish,           0, 0, 2, "5",    "Finnish" },
  { Finnish,           0, 1, 2, "C",    "Finnish" },
  { French,            0, 0, 2, "R",    "French" },
  { French,            0, 1, 2, "f",    "French" }, /* Kermit (vt340 model?) */
  { French_Canadian,   0, 0, 2, "Q",    "French Canadian" },
  { French_Canadian,   0, 1, 3, "9",    "French Canadian" },
  { German,            0, 0, 2, "K",    "German" },
  { Greek,             0, 0, 5, "\"?",  "Greek (DEC)" },
  { Greek_Supp,        1, 0, 5, "F",    "ISO Greek Supplemental" },
  { Hebrew,            0, 0, 5, "\"4",  "Hebrew (DEC)" },
  { Hebrew,            0, 1, 5, "%=",   "Hebrew NRCS" },
  { Hebrew_Supp,       1, 0, 5, "H",    "ISO Hebrew Supplemental" },
  { Italian,           0, 0, 2, "Y",    "Italian" },
  { Latin_5_Supp,      1, 0, 5, "M",    "ISO Latin-5 Supplemental" },
  { Latin_Cyrillic,    1, 0, 5, "L",    "ISO Latin-Cyrillic" },
  { Norwegian_Danish,  0, 0, 3, "`",    "Norwegian/Danish" },
  { Norwegian_Danish,  0, 1, 2, "E",    "Norwegian/Danish" },
  { Norwegian_Danish,  0, 2, 2, "6",    "Norwegian/Danish" },
  { Portugese,         0, 0, 3, "%6",   "Portugese" },
  { Russian,           0, 0, 5, "&5",   "Russian" },
  { SCS_NRCS,          0, 0, 5, "%3",   "SCS NRCS" },
  { Spanish,           0, 0, 2, "Z",    "Spanish" },
  { Swedish,           0, 0, 2, "7",    "Swedish" },
  { Swedish,           0, 1, 2, "H",    "Swedish" },
  { Swiss,             0, 0, 2, "=",    "Swiss" },
  { Turkish,           0, 0, 5, "%0",   "Turkish (DEC)" },
  { Turkish,           0, 1, 5, "%2",   "Turkish NRCS" },
  { Unknown,           0, 0, 0, "?",    "Unknown" }
};
/* *INDENT-ON* */

static int national;
static int cleanup;

static int current_Gx[4];

static void
send32(int row, int upper)
{
  int col;
  char buffer[33];

  for (col = 0; col <= 31; col++) {
    buffer[col] = (char) (row * 32 + upper + col);
  }
  buffer[32] = 0;
  tprintf("%s", buffer);
}

static char *
scs_params(char *dst, int g)
{
  int n = current_Gx[g];

  sprintf(dst, "%c%s",
          ((KnownCharsets[n].allow96 && get_level() > 2)
           ? "?-./"[g]
           : "()*+"[g]),
          KnownCharsets[n].final);
  return dst;
}

static void
do_scs(int g)
{
  char buffer[80];

  esc(scs_params(buffer, g));
}

static int
lookupCode(National code)
{
  int n;
  for (n = 0; n < TABLESIZE(KnownCharsets); n++) {
    if (KnownCharsets[n].code == code)
      return n;
  }
  return lookupCode(ASCII);
}

/* reset given Gg back to sane setting */
static int
sane_cs(int g)
{
  return lookupCode(((g == 0) || (get_level() <= 1))
                    ? ASCII
                    : (get_level() < 3
                       ? British
                       : British_Latin_1));   /* ...to get 8-bit codes 128-255 */
}

/* reset given Gg back to sane setting */
static int
reset_scs(int g)
{
  int n = sane_cs(g);
  do_scs(n);
  return n;
}

/* reset all of the Gn to sane settings */
static int
reset_charset(MENU_ARGS)
{
  int n, m;

  decnrcm(national = FALSE);
  for (n = 0; n < 4; n++) {
    m = sane_cs(cleanup ? 0 : n);
    if (m != current_Gx[n] || (m == 0)) {
      current_Gx[n] = m;
      do_scs(n);
    }
  }
  return MENU_NOHOLD;
}

static int the_code;
static int the_list[TABLESIZE(KnownCharsets) + 2];

static int
lookup_Gx(MENU_ARGS)
{
  int n;
  the_code = -1;
  for (n = 0; n < TABLESIZE(KnownCharsets); n++) {
    if (the_list[n]
        && !strcmp(the_title, KnownCharsets[n].name)) {
      the_code = n;
      break;
    }
  }
  return MENU_NOHOLD;
}

static void
specify_any_Gx(int g)
{
  MENU my_menu[TABLESIZE(KnownCharsets) + 2];
  int n, m;

  /*
   * Build up a menu of the character sets we will allow the user to specify.
   * There are a couple of tentative table entries (the "?" ones), which we
   * won't show in any event.  Beyond that, we limit some of the character sets
   * based on the emulation level (vt220 implements national replacement
   * character sets, for example, but not the 96-character ISO Latin-1).
   */
  for (n = m = 0; n < TABLESIZE(KnownCharsets); n++) {
    the_list[n] = 0;
    if (!strcmp(KnownCharsets[n].final, "?"))
      continue;
    if (get_level() < KnownCharsets[n].model)
      continue;
    if ((g == 0) && KnownCharsets[n].allow96)
      continue;
    if (m && !strcmp(my_menu[m - 1].description, KnownCharsets[n].name))
      continue;
    my_menu[m].description = KnownCharsets[n].name;
    my_menu[m].dispatch = lookup_Gx;
    the_list[n] = 1;
    m++;
  }
  my_menu[m].description = "";
  my_menu[m].dispatch = 0;

  do {
    vt_clear(2);
    __(title(0), println("Choose character-set:"));
  } while (menu(my_menu) && the_code < 0);

  current_Gx[g] = the_code;
}

static int
toggle_nrc(MENU_ARGS)
{
  national = !national;
  decnrcm(national);
  return MENU_NOHOLD;
}

static int
specify_G0(MENU_ARGS)
{
  specify_any_Gx(0);
  return MENU_NOHOLD;
}

static int
specify_G1(MENU_ARGS)
{
  specify_any_Gx(1);
  return MENU_NOHOLD;
}

static int
specify_G2(MENU_ARGS)
{
  specify_any_Gx(2);
  return MENU_NOHOLD;
}

static int
specify_G3(MENU_ARGS)
{
  specify_any_Gx(3);
  return MENU_NOHOLD;
}

static int
tst_layout(MENU_ARGS)
{
  char buffer[80];
  return tst_keyboard_layout(scs_params(buffer, 0));
}

static int
tst_vt100_charsets(MENU_ARGS)
{
  /* Test of:
   * SCS    (Select character Set)
   */
  /* *INDENT-OFF* */
  static const struct { char code; const char *msg; } table[] = {
    { 'A', "UK / national" },
    { 'B', "US ASCII" },
    { '0', "Special graphics and line drawing" },
    { '1', "Alternate character ROM standard characters" },
    { '2', "Alternate character ROM special graphics" },
  };
  /* *INDENT-ON* */

  int i, g, cset;

  __(cup(1, 10), printf("Selected as G0 (with SI)"));
  __(cup(1, 48), printf("Selected as G1 (with SO)"));
  for (cset = 0; cset < TABLESIZE(table); cset++) {
    int row = 3 + (4 * cset);

    scs(1, 'B');
    cup(row, 1);
    sgr("1");
    tprintf("Character set %c (%s)", table[cset].code, table[cset].msg);
    sgr("0");
    for (g = 0; g <= 1; g++) {
      int set_nrc = (get_level() >= 2 && table[cset].code == 'A');
      if (set_nrc)
        decnrcm(TRUE);
      scs(g, (int) table[cset].code);
      for (i = 1; i <= 3; i++) {
        cup(row + i, 10 + 38 * g);
        send32(i, 0);
      }
      if (set_nrc != national)
        decnrcm(national);
    }
  }
  scs_normal();
  __(cup(max_lines, 1), printf("These are the installed character sets. "));
  return MENU_HOLD;
}

static int
tst_shift_in_out(MENU_ARGS)
{
  /* Test of:
     SCS    (Select character Set)
   */
  static const char *label[] =
  {
    "Selected as G0 (with SI)",
    "Selected as G1 (with SO)"
  };
  int i, cset;
  char buffer[80];

  __(cup(1, 10), printf("These are the G0 and G1 character sets."));
  for (cset = 0; cset < 2; cset++) {
    int row = 3 + (4 * cset);

    scs(cset, 'B');
    cup(row, 1);
    sgr("1");
    tprintf("Character set %s (%s)",
            KnownCharsets[current_Gx[cset]].final,
            KnownCharsets[current_Gx[cset]].name);
    sgr("0");

    cup(row, 48);
    tprintf("%s", label[cset]);

    esc(scs_params(buffer, cset));
    for (i = 1; i <= 3; i++) {
      cup(row + i, 10);
      send32(i, 0);
    }
    scs(cset, 'B');
  }
  cup(max_lines, 1);
  return MENU_HOLD;
}

#define map_g1_to_gr() esc("~")   /* LS1R */

static int
tst_vt220_locking(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static const struct {
    int upper;
    int mapped;
    const char *code;
    const char *msg;
  } table[] = {
    { 1, 1, "~", "G1 into GR (LS1R)" },
    { 0, 2, "n", "G2 into GL (LS2)"  }, /* "{" vi */
    { 1, 2, "}", "G2 into GR (LS2R)" },
    { 0, 3, "o", "G3 into GL (LS3)"  },
    { 1, 3, "|", "G3 into GR (LS3R)" },
  };
  /* *INDENT-ON* */

  int i, cset;

  __(cup(1, 10), tprintf("Locking shifts, with NRC %s:",
                         national ? "enabled" : "disabled"));
  for (cset = 0; cset < TABLESIZE(table); cset++) {
    int row = 3 + (4 * cset);
    int map = table[cset].mapped;

    scs_normal();
    cup(row, 1);
    sgr("1");
    tprintf("Character set %s (%s) in G%d",
            KnownCharsets[current_Gx[map]].final,
            KnownCharsets[current_Gx[map]].name,
            map);
    sgr("0");

    cup(row, 48);
    tprintf("Maps %s", table[cset].msg);

    for (i = 1; i <= 3; i++) {
      if (table[cset].upper) {
        scs_normal();
        map_g1_to_gr();
      } else {
        do_scs(map);
        esc(table[cset].code);
      }
      cup(row + i, 5);
      send32(i, 0);

      if (table[cset].upper) {
        do_scs(map);
        esc(table[cset].code);
      } else {
        scs_normal();
        map_g1_to_gr();
      }
      cup(row + i, 40);
      send32(i, 128);
    }
    reset_scs(cset);
  }
  scs_normal();
  cup(max_lines, 1);
  return MENU_HOLD;
}

static int
tst_vt220_single(MENU_ARGS)
{
  int pass, x, y;

  for (pass = 0; pass < 2; pass++) {
    int g = pass + 2;

    vt_clear(2);
    cup(1, 1);
    tprintf("Testing single-shift G%d into GL (SS%d) with NRC %s\n",
            g, g, national ? "enabled" : "disabled");
    tprintf("G%d is %s", g, KnownCharsets[current_Gx[g]].name);

    do_scs(g);
    for (y = 0; y < 16; y++) {
      for (x = 0; x < 6; x++) {
        int ch = y + (x * 16) + 32;
        cup(y + 5, (x * 12) + 5);
        tprintf("%3d: (", ch);
        esc(pass ? "O" : "N");  /* SS3 or SS2 */
        tprintf("%c", ch);
        if (ch == 127 && !KnownCharsets[current_Gx[g]].allow96)
          tprintf(" ");   /* DEL should have been eaten - skip past */
        tprintf(")");
      }
    }

    cup(max_lines, 1);
    holdit();
  }

  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * For parsing DECCIR response.  The end of the response consists of so-called
 * intermediate and final bytes as used by the SCS controls.  Most of the
 * strings fit into that description, but note that '<', '=' and '>' do not,
 * since they are used to denote private parameters rather than final bytes.
 * (But ECMA-48 hedges this by stating that the format in those cases is not
 * specified).
 */
const char *
parse_Sdesig(const char *source, int *offset)
{
  int j;
  const char *first = source + (*offset);
  const char *result = 0;
  size_t limit = strlen(first);

  for (j = 0; j < TABLESIZE(KnownCharsets); ++j) {
    if (KnownCharsets[j].code != Unknown) {
      size_t check = strlen(KnownCharsets[j].final);
      if (check <= limit
          && !strncmp(KnownCharsets[j].final, first, check)) {
        result = KnownCharsets[j].name;
        *offset += (int) check;
        break;
      }
    }
  }
  if (result == 0) {
    static char temp[80];
    sprintf(temp, "? %#x\n", *source);
    *offset += 1;
    result = temp;
  }
  return result;
}

/*
 * Reset G0 to ASCII
 * Reset G1 to ASCII
 * Shift-in.
 */
void
scs_normal(void)
{
  scs(0, 'B');
}

/*
 * Set G0 to Line Graphics
 * Reset G1 to ASCII
 * Shift-in.
 */
void
scs_graphics(void)
{
  scs(0, '0');
}

int
tst_characters(MENU_ARGS)
{
  static char whatis_Gx[4][80];
  static char nrc_mesg[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Reset (ASCII for G0, G1, no NRC mode)",             reset_charset },
      { nrc_mesg,                                            toggle_nrc },
      { whatis_Gx[0],                                        specify_G0 },
      { whatis_Gx[1],                                        specify_G1 },
      { whatis_Gx[2],                                        specify_G2 },
      { whatis_Gx[3],                                        specify_G3 },
      { "Test VT100 Character Sets",                         tst_vt100_charsets },
      { "Test Shift In/Shift Out (SI/SO)",                   tst_shift_in_out },
      { "Test VT220 Locking Shifts",                         tst_vt220_locking },
      { "Test VT220 Single Shifts",                          tst_vt220_single },
      { "Test Soft Character Sets",                          not_impl },
      { "Test Keyboard Layout with G0 Selection",            tst_layout },
      { "",                                                  0 }
  };
  /* *INDENT-ON* */

  int n;

  cleanup = 0;
  reset_charset(PASS_ARGS);   /* make the menu consistent */
  if (get_level() > 1 || input_8bits || output_8bits) {
    do {
      vt_clear(2);
      __(title(0), printf("Character-Set Tests"));
      __(title(2), println("Choose test type:"));
      sprintf(nrc_mesg, "%s National Replacement Character (NRC) mode",
              national ? "Disable" : "Enable");
      for (n = 0; n < 4; n++) {
        sprintf(whatis_Gx[n], "Specify G%d (now %s)",
                n, KnownCharsets[current_Gx[n]].name);
      }
    } while (menu(my_menu));
    cleanup = 1;
    return reset_charset(PASS_ARGS);
  } else {
    return tst_vt100_charsets(PASS_ARGS);
  }
}
