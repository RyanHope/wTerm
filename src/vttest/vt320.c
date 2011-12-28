/* $Id: vt320.c,v 1.21 2011/05/06 20:46:55 tom Exp $ */

/*
 * Reference:  VT330/VT340 Programmer Reference Manual (EK-VT3XX-TP-001)
 */
#include <vttest.h>
#include <ttymodes.h>
#include <esc.h>

static void
show_Locator_Status(char *report)
{
  int pos = 0;
  int code = scanto(report, &pos, 'n');
  const char *show;

  switch (code) {
  case 53:
    show = "No locator";
    break;
  case 50:
    show = "Locator ready";
    break;
  case 58:
    show = "Locator busy";
    break;
  default:
    show = SHOW_FAILURE;
  }
  show_result("%s", show);
}

/*
 * Though some people refer to the locator controls as "vt220", it appears in
 * later terminals (documented in the vt320 manual, but introduced as in UWS).
 */
int
tst_DSR_locator(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?53n", show_Locator_Status);
}

static void
show_ExtendedCursorPosition(char *report)
{
  int pos = 0;
  int Pl = scan_any(report, &pos, 'R');
  int Pc = scan_any(report, &pos, 'R');
  int Pp = scan_any(report, &pos, 'R');

  if (Pl != 0 && Pc != 0) {
    if (Pp != 0)
      show_result("Line %d, Column %d, Page %d", Pl, Pc, Pp);
    else
      show_result("Line %d, Column %d (Page?)", Pl, Pc);
  } else
    show_result(SHOW_FAILURE);
}

/* vt340/vt420 & up */
int
tst_DSR_cursor(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?6n", show_ExtendedCursorPosition);
}

/******************************************************************************/

int
tst_vt320_device_status(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT220 features",                               tst_vt220_device_status },
      { "Test Keyboard Status",                              tst_DSR_keyboard },
      { "Test Printer Status",                               tst_DSR_printer },
      { "Test UDK Status",                                   tst_DSR_userkeys },
      { "Test Locator Status",                               tst_DSR_locator },
      { "Test Extended Cursor-Position (DECXCPR)",           tst_DSR_cursor },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Device Status Reports (DSR)"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * DECCIR returns single characters separated by semicolons.  It's not clear
 * (unless you test on a DEC terminal) from the documentation, which only cites
 * their values.  This function returns an equivalent-value, recovering from
 * the bogus implementations that return a decimal number.
 */
static int
scan_chr(char *str, int *pos, int toc)
{
  int value = 0;
  while (str[*pos] != '\0' && str[*pos] != toc) {
    value = (value * 256) + (unsigned char) str[*pos];
    *pos += 1;
  }
  if (str[*pos] == toc)
    *pos += 1;
  return value;
}

/*
 * From Kermit 3.13 & VT220 pocket guide
 *
 * Request  CSI 1 $ w             cursor information report
 * Response DCS 1 $ u Pr; Pc; Pp; Srend; Satt; Sflag; Pgl; Pgr; Scss; Sdesig ST
 *        where   Pr is cursor row (counted from origin as 1,1)
 *                Pc is cursor column
 *                Pp is 1, video page, a constant for VT320s
 *                Srend = 40h + 8 (rev video on) + 4 (blinking on)
 *                                 + 2 (underline on) + 1 (bold on)
 *                Satt = 40h + 1  (selective erase on)
 *                Sflag = 40h + 8 (autowrap pending) + 4 (SS3 pending)
 *                                + 2 (SS2 pending) + 1 (Origin mode on)
 *                Pgl = char set in GL (0 = G0, 1 = G1, 2 = G2, 3 = G3)
 *                Pgr = char set in GR (same as for Pgl)
 *                Scss = 40h + 8 (G3 is 96 char) + 4 (G2 is 96 char)
 *                                + 2 (G1 is 96 char) + 1 (G0 is 96 char)
 *                Sdesig is string of character idents for sets G0...G3, with
 *                                no separators between set idents.
 *                If NRCs are active the set idents (all 94 byte types) are:
 *                British         A       Italian         Y
 *                Dutch           4       Norwegian/Danish ' (hex 60) or E or 6
 *                Finnish         5 or C  Portuguese      %6 or g or L
 *                French          R or f  Spanish         Z
 *                French Canadian 9 or Q  Swedish         7 or H
 *                German          K       Swiss           =
 *                Hebrew          %=
 *                (MS Kermit uses any choice when there are multiple)
 */

#define show_DECCIR_flag(value, mask, string) \
  if (value & mask) { value &= ~mask; show_result(string); }

static void
show_DECCIR(char *report)
{
  int Pr, Pc, Pp, Srend, Satt, Sflag, Pgl, Pgr, Scss;
  int pos = 3;                  /* skip "1$u" */
  int n;

  Pr = scanto(report, &pos, ';');
  Pc = scanto(report, &pos, ';');
  Pp = scanto(report, &pos, ';');
  vt_move(5, 10);
  show_result("Cursor (%d,%d), page %d", Pr, Pc, Pp);

  Srend = scan_chr(report, &pos, ';');
  vt_move(6, 10);
  if (Srend & 0x40) {
    show_DECCIR_flag(Srend, 0x40, "Rendition:");
    if (Srend == 0)
      show_result(" normal");
    show_DECCIR_flag(Srend, 0x08, " reverse");
    show_DECCIR_flag(Srend, 0x04, " blinking");
    show_DECCIR_flag(Srend, 0x02, " underline");
    show_DECCIR_flag(Srend, 0x01, " bold");
  }
  if (Srend)
    show_result(" -> unknown rendition (0x%x)", Srend);

  Satt = scan_chr(report, &pos, ';');
  vt_move(7, 10);
  switch (Satt) {
  case 0x40:
    show_result("Selective erase: off");
    break;
  case 0x41:
    show_result("Selective erase: ON");
    break;
  default:
    show_result("Selective erase: unknown (0x%x)", Satt);
  }

  Sflag = scan_chr(report, &pos, ';');
  vt_move(8, 10);
  if (Sflag & 0x40) {
    show_DECCIR_flag(Sflag, 0x40, "Flags:");
    show_DECCIR_flag(Sflag, 0x08, " autowrap pending");
    show_DECCIR_flag(Sflag, 0x04, " SS3 pending");
    show_DECCIR_flag(Sflag, 0x02, " SS2 pending");
    show_DECCIR_flag(Sflag, 0x01, " origin-mode on");
  } else {
    show_result(" -> unknown flag (0x%x)", Sflag);
  }

  Pgl = scanto(report, &pos, ';');
  Pgr = scanto(report, &pos, ';');
  vt_move(9, 10);
  show_result("Char set in GL: G%d, Char set in GR: G%d", Pgl, Pgr);

  Scss = scan_chr(report, &pos, ';');
  vt_move(10, 10);
  if (Scss & 0x40) {
    show_DECCIR_flag(Scss, 0x40, "Char set sizes:");
    show_DECCIR_flag(Scss, 0x08, " G3 is 96 char");
    show_DECCIR_flag(Scss, 0x04, " G2 is 96 char");
    show_DECCIR_flag(Scss, 0x02, " G1 is 96 char");
    show_DECCIR_flag(Scss, 0x01, " G0 is 96 char");   /* VT420 manual says this cannot happen */
  } else {
    show_result(" -> unknown char set size (0x%x)", Scss);
  }

  n = 11;
  vt_move(n, 10);
  show_result("Character set idents for G0...G3: ");
  println("");
  while (report[pos] != '\0') {
    const char *result = parse_Sdesig(report, &pos);
    show_result("            %s\n", result);
    println("");
  }
}

/******************************************************************************/

/*
 * Request  CSI 2 $ w             tab stop report
 * Response DCS 2 $ u Pc/Pc/...Pc ST
 *        Pc are column numbers (from 1) where tab stops occur. Note the
 *        separator "/" occurs in a real VT320 but should have been ";".
 */
static void
show_DECTABSR(char *report)
{
  int pos = 3;                  /* skip "2$u" */
  int stop;
  char *buffer = (char *) malloc(strlen(report));

  *buffer = '\0';
  strcat(report, "/");  /* simplify scanning */
  while ((stop = scanto(report, &pos, '/')) != 0) {
    sprintf(buffer + strlen(buffer), " %d", stop);
  }
  println("");
  show_result("Tab stops:%s", buffer);
  free(buffer);
}

/******************************************************************************/

int
any_decrqpsr(MENU_ARGS, int Ps)
{
  char *report;

  vt_move(1, 1);
  printf("Testing DECRQPSR: %s\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("%d$w", Ps);
  report = get_reply();
  vt_move(3, 10);
  chrprint(report);
  if ((report = skip_dcs(report)) != 0) {
    if (strip_terminator(report)
        && *report == Ps + '0'
        && !strncmp(report + 1, "$u", (size_t) 2)) {
      show_result("%s (valid request)", SHOW_SUCCESS);
      switch (Ps) {
      case 1:
        show_DECCIR(report);
        break;
      case 2:
        show_DECTABSR(report);
        break;
      }
    } else {
      show_result(SHOW_FAILURE);
    }
  } else {
    show_result(SHOW_FAILURE);
  }

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
tst_DECCIR(MENU_ARGS)
{
  return any_decrqpsr(PASS_ARGS, 1);
}

static int
tst_DECTABSR(MENU_ARGS)
{
  return any_decrqpsr(PASS_ARGS, 2);
}

/* Test Window Report - VT340, VT420 */
static int
tst_DECRQDE(MENU_ARGS)
{
  char *report;
  char chr;
  int Ph, Pw, Pml, Pmt, Pmp;

  vt_move(1, 1);
  println("Testing DECRQDE/DECRPDE Window Report");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("\"v");
  report = get_reply();
  vt_move(3, 10);
  chrprint(report);

  if ((report = skip_csi(report)) != 0
      && sscanf(report, "%d;%d;%d;%d;%d\"%c",
                &Ph, &Pw, &Pml, &Pmt, &Pmp, &chr) == 6
      && chr == 'w') {
    vt_move(5, 10);
    show_result("lines:%d, cols:%d, left col:%d, top line:%d, page %d",
                Ph, Pw, Pml, Pmt, Pmp);
  } else {
    show_result(SHOW_FAILURE);
  }

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

/* Test User-Preferred Supplemental Set - VT320 */
static int
tst_DECRQUPSS(MENU_ARGS)
{
  char *report;
  const char *show;

  __(vt_move(1, 1), println("Testing DECRQUPSS/DECAUPSS Window Report"));

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("&u");
  report = get_reply();
  vt_move(3, 10);
  chrprint(report);
  if ((report = skip_dcs(report)) != 0
      && strip_terminator(report)) {
    if (!strcmp(report, "0!u%5"))
      show = "DEC Supplemental Graphic";
    else if (!strcmp(report, "1!uA"))
      show = "ISO Latin-1 supplemental";
    else
      show = "unknown";
  } else {
    show = SHOW_FAILURE;
  }
  show_result("%s", show);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

/* Request Terminal State Report */
static int
tst_DECRQTSR(MENU_ARGS)
{
  char *report;
  const char *show;

  vt_move(1, 1);
  println("Testing Terminal State Reports (DECRQTSR/DECTSR)");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("1$u");
  report = get_reply();

  vt_move(3, 10);
  chrprint(report);

  if ((report = skip_dcs(report)) != 0
      && strip_terminator(report)
      && !strncmp(report, "1$s", (size_t) 3)) {
    show = SHOW_SUCCESS;
  } else {
    show = SHOW_FAILURE;
  }
  show_result("%s", show);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

/******************************************************************************/

static int use_DECRPM;

int
set_DECRPM(int level)
{
  int result = use_DECRPM;
  use_DECRPM = level;
  return result;
}

#define DATA(name,level) { name, #name, level }

int
any_RQM(MENU_ARGS, RQM_DATA * table, int tablesize, int private)
{
  int j, row, Pa, Ps;
  char chr;
  char *report;

  vt_move(1, 1);
  printf("Testing %s\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (j = row = 0; j < tablesize; j++) {
    if (use_DECRPM < table[j].level)
      continue;

    if (++row >= max_lines - 3) {
      restore_ttymodes();
      vt_move(max_lines - 1, 1);
      holdit();
      vt_clear(2);
      vt_move(row = 2, 1);
      set_tty_raw(TRUE);
      set_tty_echo(FALSE);
    }

    do_csi((private ? "?%d$p" : "%d$p"), table[j].mode);
    report = instr();
    printf("\n     %4d: %-10s", table[j].mode, table[j].name);
    if (LOG_ENABLED)
      fprintf(log_fp, "Testing %s\n", table[j].name);
    chrprint(report);
    if ((report = skip_csi(report)) != 0
        && sscanf(report, (private
                           ? "?%d;%d$%c"
                           : "%d;%d$%c"),
                  &Pa, &Ps, &chr) == 3
        && Pa == table[j].mode
        && chr == 'y') {
      switch (Ps) {
      case 0:
        show_result(" unknown");
        break;
      case 1:
        show_result(" set");
        break;
      case 2:
        show_result(" reset");
        break;
      case 3:
        show_result(" permanently set");
        break;
      case 4:
        show_result(" permanently reset");
        break;
      default:
        show_result(" ?");
        break;
      }
    } else {
      show_result(SHOW_FAILURE);
    }
  }

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
tst_ISO_DECRPM(MENU_ARGS)
{
  /* *INDENT-OFF* */
  RQM_DATA ansi_modes[] = { /* this list is sorted by code, not name */
    DATA( GATM, 3 /* guarded area transfer (disabled) */),
    DATA( KAM,  3 /* keyboard action */),
    DATA( CRM,  3 /* control representation (setup) */),
    DATA( IRM,  3 /* insert/replace */),
    DATA( SRTM, 3 /* status reporting transfer (disabled) */),
    DATA( ERM,  9 /* erasure mode (non-DEC) */),
    DATA( VEM,  3 /* vertical editing (disabled) */),
    DATA( BDSM, 9 /* bi-directional support mode (non-DEC) */),
    DATA( DCSM, 9 /* device component select mode (non-DEC) */),
    DATA( HEM,  3 /* horizontal editing (disabled) */),
    DATA( PUM,  3 /* positioning unit (disabled) */),
    DATA( SRM,  3 /* send/receive */),
    DATA( FEAM, 3 /* format effector action (disabled) */),
    DATA( FETM, 3 /* format effector transfer (disabled) */),
    DATA( MATM, 3 /* multiple area transfer (disabled) */),
    DATA( TTM,  3 /* transfer termination (disabled) */),
    DATA( SATM, 3 /* selected area transfer (disabled) */),
    DATA( TSM,  3 /* tabulation stop (disabled) */),
    DATA( EBM,  3 /* editing boundary (disabled) */),
    DATA( LNM,  3 /* line feed/new line */) };
  /* *INDENT-ON* */

  return any_RQM(PASS_ARGS, ansi_modes, TABLESIZE(ansi_modes), 0);
}

static int
tst_DEC_DECRPM(MENU_ARGS)
{
  /* *INDENT-OFF* */
  RQM_DATA dec_modes[] = { /* this list is sorted by code, not name */
    DATA( DECCKM,  3 /* cursor keys */),
    DATA( DECANM,  3 /* ANSI */),
    DATA( DECCOLM, 3 /* column */),
    DATA( DECSCLM, 3 /* scrolling */),
    DATA( DECSCNM, 3 /* screen */),
    DATA( DECOM,   3 /* origin */),
    DATA( DECAWM,  3 /* autowrap */),
    DATA( DECARM,  3 /* autorepeat */),
    DATA( DECEDM,  3 /* edit */),
    DATA( DECLTM,  3 /* line transmit */),
    DATA( DECSCFDM,3 /* space compression field delimiter */),
    DATA( DECTEM,  3 /* transmission execution */),
    DATA( DECEKEM, 3 /* edit key execution */),
    DATA( DECPFF,  3 /* print form feed */),
    DATA( DECPEX,  3 /* printer extent */),
    DATA( DECTCEM, 3 /* text cursor enable */),
    DATA( DECRLM,  5 /* left-to-right */),
    DATA( DECTEK,  3 /* 4010/4014 emulation */),
    DATA( DECHEM,  5 /* Hebrew encoding */),
    DATA( DECNRCM, 3 /* national replacement character set */),
    DATA( DECGEPM, 3 /* graphics expanded print */),
    DATA( DECGPCM, 3 /* graphics print color */),
    DATA( DECGPCS, 3 /* graphics print color syntax */),
    DATA( DECGPBM, 3 /* graphics print background */),
    DATA( DECGRPM, 3 /* graphics rotated print */),
    DATA( DEC131TM,3 /* VT131 transmit */),
    DATA( DECNAKB, 5 /* Greek/N-A Keyboard Mapping */),
    DATA( DECHCCM, 3 /* horizontal cursor coupling (disabled) */),
    DATA( DECVCCM, 3 /* vertical cursor coupling */),
    DATA( DECPCCM, 3 /* page cursor coupling */),
    DATA( DECNKM,  3 /* numeric keypad */),
    DATA( DECBKM,  3 /* backarrow key */),
    DATA( DECKBUM, 3 /* keyboard usage */),
    DATA( DECVSSM, 4 /* vertical split */),
    DATA( DECXRLM, 3 /* transmit rate linking */),
    DATA( DECKPM,  4 /* keyboard positioning */),
    DATA( DECNCSM, 5 /* no clearing screen on column change */),
    DATA( DECRLCM, 5 /* right-to-left copy */),
    DATA( DECCRTSM,5 /* CRT save */),
    DATA( DECARSM, 5 /* auto resize */),
    DATA( DECMCM,  5 /* modem control */),
    DATA( DECAAM,  5 /* auto answerback */),
    DATA( DECCANSM,5 /* conceal answerback */),
    DATA( DECNULM, 5 /* null */),
    DATA( DECHDPXM,5 /* half duplex */),
    DATA( DECESKM, 5 /* enable secondary keyboard language */),
    DATA( DECOSCNM,5 /* overscan */),
    DATA( DECFWM,  5 /* framed windows */),
    DATA( DECRPL,  5 /* review previous lines */),
    DATA( DECHWUM, 5 /* host wake-up mode (CRT and energy saver) */),
    DATA( DECATCUM,5 /* alternate text color underline */),
    DATA( DECATCBM,5 /* alternate text color blink */),
    DATA( DECBBSM, 5 /* bold and blink style */),
    DATA( DECECM,  5 /* erase color */),
  };
  /* *INDENT-ON* */

  return any_RQM(PASS_ARGS, dec_modes, TABLESIZE(dec_modes), 1);
}

#undef DATA

/******************************************************************************/

int
tst_DECRPM(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "ANSI Mode Report (DECRPM)",                         tst_ISO_DECRPM },
      { "DEC Mode Report (DECRPM)",                          tst_DEC_DECRPM },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("Request Mode (DECRQM)/Report Mode (DECRPM)"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * FIXME: The VT420 manual says that a valid response begins "DCS 0 $ r",
 * however I see "DCS 1 $ r" on a real VT420, consistently.
 */
int
any_decrqss(const char *msg, const char *func)
{
  char *report;
  const char *show;

  vt_move(1, 1);
  printf("Testing DECRQSS: %s\n", msg);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  decrqss(func);
  report = get_reply();
  vt_move(3, 10);
  chrprint(report);
  switch (parse_decrqss(report, func)) {
  case 1:
    show = "ok (valid request)";
    break;
  case 0:
    show = "invalid request";
    break;
  default:
    show = SHOW_FAILURE;
    break;
  }
  show_result("%s", show);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
rpt_DECSASD(MENU_ARGS)
{
  return any_decrqss(the_title, "$}");
}

static int
rpt_DECSCA(MENU_ARGS)
{
  return any_decrqss(the_title, "\"q");
}

static int
rpt_DECSCL(MENU_ARGS)
{
  return any_decrqss(the_title, "\"p");
}

static int
rpt_DECSCPP(MENU_ARGS)
{
  return any_decrqss(the_title, "$|");
}

static int
rpt_DECSLPP(MENU_ARGS)
{
  return any_decrqss(the_title, "t");
}

static int
rpt_DECSSDT(MENU_ARGS)
{
  return any_decrqss(the_title, "$~");
}

static int
rpt_DECSTBM(MENU_ARGS)
{
  return any_decrqss(the_title, "r");
}

static int
rpt_SGR(MENU_ARGS)
{
  return any_decrqss(the_title, "m");
}

static int
rpt_DECTLTC(MENU_ARGS)
{
  return any_decrqss(the_title, "'s");
}

static int
rpt_DECTTC(MENU_ARGS)
{
  return any_decrqss(the_title, "|");
}

int
tst_vt320_DECRQSS(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Select active status display (DECSASD)",            rpt_DECSASD },
      { "Set character attribute (DECSCA)",                  rpt_DECSCA },
      { "Set conformance level (DECSCL)",                    rpt_DECSCL },
      { "Set columns per page (DECSCPP)",                    rpt_DECSCPP },
      { "Set lines per page (DECSLPP)",                      rpt_DECSLPP },
      { "Set status line type (DECSSDT)",                    rpt_DECSSDT },
      { "Set top and bottom margins (DECSTBM)",              rpt_DECSTBM },
      { "Select graphic rendition (SGR)",                    rpt_SGR },
      { "Set transmit termination character (DECTTC)",       rpt_DECTTC },
      { "Transmission line termination character (DECTLTC)", rpt_DECTLTC },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Status-String Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * The main vt100 module tests CUP, HVP, CUF, CUB, CUU, CUD
 */
int
tst_vt320_cursor(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Pan down (SU)",                                tst_SU },
      { "Test Pan up (SD)",                                  tst_SD},
      { "Test Vertical Cursor Coupling (DECVCCM)",           not_impl },
      { "Test Page Cursor Coupling (DECPCCM)",               not_impl },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Cursor-Movement Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_vt320_report_terminal(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Restore Terminal State (DECRSTS)",                  not_impl },
      { "Terminal State Report (DECRQTS/DECTSR)",            tst_DECRQTSR },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Terminal State Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

int
tst_vt320_report_presentation(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Cursor Information Report (DECCIR)",                tst_DECCIR },
      { "Tab Stop Report (DECTABSR)",                        tst_DECTABSR },
      { "Request Mode (DECRQM)/Report Mode (DECRPM)",        tst_DECRPM },
      { "Restore Presentation State (DECRSPS)",              not_impl },
      { "Status-String Report (DECRQSS)",                    tst_vt320_DECRQSS },
      { "",                                                  0 }
  };
  /* *INDENT-ON* */

  int old_DECRPM = set_DECRPM(3);

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Presentation State Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  set_DECRPM(old_DECRPM);
  return MENU_NOHOLD;
}

/******************************************************************************/

int
tst_vt320_reports(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT220 features",                               tst_vt220_reports },
      { "Test Device Status Report (DSR)",                   tst_vt320_device_status },
      { "Test Presentation State Reports",                   tst_vt320_report_presentation },
      { "Test Terminal State Reports",                       tst_vt320_report_terminal },
      { "Test User-Preferred Supplemental Set (DECAUPSS)",   tst_DECRQUPSS },
      { "Test Window Report (DECRPDE)",                      tst_DECRQDE },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/* vt340/vt420 & up */
static int
tst_PageMovement(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Next Page (NP)",                               not_impl },
      { "Test Preceding Page (PP)",                          not_impl },
      { "Test Page Position Absolute (PPA)",                 not_impl },
      { "Test Page Position Backward (PPB)",                 not_impl },
      { "Test Page Position Relative (PPR)",                 not_impl },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("Page Format Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/* vt340/vt420 & up */
int
tst_vt320_screen(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT220 features",                               tst_vt220_screen },
      { "Test Status line (DECSASD/DECSSDT)",                tst_statusline },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Screen-Display Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

int
tst_vt320(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT220 features",                               tst_vt220 },
      { "Test cursor-movement",                              tst_vt320_cursor },
      { "Test page-movement controls",                       tst_PageMovement },
      { "Test reporting functions",                          tst_vt320_reports },
      { "Test screen-display functions",                     tst_vt320_screen },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT320 Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}
