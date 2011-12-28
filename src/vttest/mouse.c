/* $Id: mouse.c,v 1.19 2010/08/29 19:26:24 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

#define MCHR(c) (unsigned)((unsigned)((c) - ' ') & 0xff)

#define isQuit(c)   (((c) == 'q') || ((c) == 'Q'))
#define isReport(c) (get_level() >= 3 && (((c) == 'r') || ((c) == 'R')))
#define isClear(c)  ((c) == ' ')

#define ToData(n)  vt_move(4 + n, 10)

static int do_ExtMouse;
static int do_FocusEvent;
static int chars_high;
static int chars_wide;
static int pixels_high;
static int pixels_wide;

static void
show_mousehelp(void)
{
  if (get_level() >= 3)
    println("Press 'q' to quit, 'r' to report modes, ' ' to clear.");
  else
    println("Press 'q' to quit, ' ' to clear.");
}

static unsigned
xterm_coord(char *source, int *pos)
{
  unsigned result;

  if (do_ExtMouse) {
    int used;
    char *real_src = source + *pos;
    unsigned limit = (unsigned) strlen(real_src);

    used = conv_to_utf32((unsigned *) 0, real_src, limit);
    if (used > 0) {
      (void) conv_to_utf32(&result, real_src, limit);
      *pos += used;
      if (result > ' ')
        result -= ' ';
      else
        result = 0;
    } else {
      result = 0;
    }

  } else {
    result = MCHR(source[*pos]);
    *pos += 1;
  }
  return result;
}

static void
cat_button(char *dst, const char *src)
{
  if (*dst != '\0')
    strcat(dst, ", ");
  strcat(dst, src);
}

static char *
locator_button(unsigned b)
{
  static char result[80];

  if (b) {
    result[0] = 0;
    if (b & 1)
      cat_button(result, "right");
    if (b & 2)
      cat_button(result, "middle");
    if (b & 4)
      cat_button(result, "left");
    if (b & 8)
      cat_button(result, "M4");
  } else {
    strcpy(result, "no buttons down");
  }
  return result;
}

static const char *
locator_event(int e)
{
  const char *result;
  switch (e) {
  case 0:
    result = "locator unavailable";
    break;
  case 1:
    result = "request - received a DECRQLP";
    break;
  case 2:
    result = "left button down";
    break;
  case 3:
    result = "left button up";
    break;
  case 4:
    result = "middle button down";
    break;
  case 5:
    result = "middle button up";
    break;
  case 6:
    result = "right button down";
    break;
  case 7:
    result = "right button up";
    break;
  case 8:
    result = "M4 button down";
    break;
  case 9:
    result = "M4 button up";
    break;
  case 10:
    result = "locator outside filter rectangle";
    break;
  default:
    result = "unknown event";
    break;
  }
  return result;
}

static void
show_click(unsigned y, unsigned x, int c)
{
  cup((int) y, (int) x);
  putchar(c);
  vt_move((int) y, (int) x);
  fflush(stdout);
}

/* Print the corners of the highlight-region.  Note that xterm doesn't use
 * the last row.
 */
static void
show_hilite(int first, int last)
{
  /* *INDENT-OFF* */
  vt_move(first, 1);          printf("+");
  vt_move(last-1,  1);        printf("+");
  vt_move(first, min_cols);   printf("+");
  vt_move(last-1,  min_cols); printf("+");
  /* *INDENT-ON* */

  fflush(stdout);
}

static void
show_locator_rectangle(void)
{
  const int first = 10;
  const int last = 20;

  decefr(first, 1, last, min_cols);
  show_hilite(first, last);
}

#define SCALED(value,range) \
      ((value * (unsigned) range + (unsigned) (range - 1)) / (unsigned) range)

static int
show_locator_report(char *report, int row, int pixels)
{
  int Pe, Pb, Pp;
  unsigned Pr, Pc;
  int now = row;

  ToData(0);
  vt_el(2);
  chrprint(report);
  while ((report = skip_csi(report)) != 0
         && (sscanf(report,
                    "%d;%d;%u;%u&w", &Pe, &Pb, &Pr, &Pc) == 4
             || sscanf(report,
                       "%d;%d;%u;%u;%d&w", &Pe, &Pb, &Pr, &Pc, &Pp) == 5)) {
    vt_move(row, 10);
    vt_el(2);
    show_result("%s - %s (%d,%d)",
                locator_event(Pe),
                locator_button((unsigned) Pb),
                Pr, Pc);
    vt_el(0);
    if (pixels) {
      if (pixels_high > 0 && pixels_wide > 0) {
        Pr = SCALED(Pr, pixels_high);
        Pc = SCALED(Pc, pixels_wide);
        show_click(Pr, Pc, '*');
      }
    } else {
      show_click(Pr, Pc, '*');
    }
    report = strchr(report, '&') + 2;
    now = row++;
  }
  return now;
}

static int
get_screensize(MENU_ARGS)
{
  char *report;
  char tmp = 0;

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  brc(14, 't'); /* report window's pixel-size */
  report = instr();
  if ((report = skip_csi(report)) == 0
      || sscanf(report, "4;%d;%d%c", &pixels_high, &pixels_wide, &tmp) != 3
      || tmp != 't'
      || pixels_high <= 0
      || pixels_wide <= 0) {
    pixels_high = -1;
    pixels_wide = -1;
  }

  brc(18, 't'); /* report window's char-size */
  report = instr();
  if ((report = skip_csi(report)) == 0
      || sscanf(report, "8;%d;%d%c", &chars_high, &chars_wide, &tmp) != 3
      || tmp != 't'
      || chars_high <= 0
      || chars_wide <= 0) {
    chars_high = 24;
    chars_wide = 80;
  }

  restore_ttymodes();
  return MENU_NOHOLD;
}

static void
show_dec_locator_events(MENU_ARGS, int mode, int pixels)
{
  int row, now;

first:
  vt_move(1, 1);
  ed(0);
  println(the_title);
  show_mousehelp();
  println("Mouse events will be marked with '*'");

  decelr((mode > 0) ? mode : ((mode == 0) ? 2 : -mode), pixels ? 1 : 2);

  if (mode < 0)
    show_locator_rectangle();
  else if (mode == 0)
    do_csi("'w");   /* see decefr() */

  decsle(1);    /* report button-down events */
  decsle(3);    /* report button-up events */
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  now = 4;
  for (;;) {
    char *report = instr();
    if (isQuit(*report)) {
      decrqlp(1);
      report = instr();
      show_locator_report(report, now + 1, pixels);
      break;
    } else if (isReport(*report)) {
      show_mousemodes();
      goto first;
    } else if (isClear(*report)) {
      goto first;
    }
    row = 4;
    while (now > row) {
      vt_move(now, 1);
      vt_el(2);
      now--;
    }
    now = show_locator_report(report, row, pixels);
    if (mode == 0) {
      decelr(2, pixels ? 1 : 2);
      do_csi("'w");   /* see decefr() */
    }
  }

  decelr(0, 0);
  restore_ttymodes();

  vt_move(max_lines - 2, 1);
}

/* Normal Mouse Tracking */
static void
show_mouse_tracking(MENU_ARGS, const char *the_mode)
{
  unsigned y = 0, x = 0;

first:
  vt_move(1, 1);
  ed(0);
  println(the_title);
  show_mousehelp();
  println("Mouse events will be marked with the button number.");

  sm(the_mode);
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (;;) {
    char *report = instr();

    if (isQuit(*report)) {
      break;
    } else if (isReport(*report)) {
      show_mousemodes();
      goto first;
    } else if (isClear(*report)) {
      goto first;
    }

    ToData(0);
    vt_el(2);
    chrprint(report);

    while ((report = skip_csi(report)) != 0
           && *report == 'M'
           && strlen(report) >= 4) {
      unsigned b = MCHR(report[1]);
      unsigned adj = 1;
      int pos = 2;
      unsigned xx = xterm_coord(report, &pos);
      unsigned yy = xterm_coord(report, &pos);

      ToData(1);
      vt_el(2);
      show_result("code 0x%x (%d,%d)", b, yy, xx);
      if (b & (unsigned) (~3)) {
        if (b & 4)
          printf(" shift");
        if (b & 8)
          printf(" meta");
        if (b & 16)
          printf(" control");
        if (b & 32)
          printf(" motion");
        if (b & 64)
          adj += 3;
      }
      b &= 3;
      if (b != 3) {
        b += adj;
        printf(" button %u", b);
        show_click(yy, xx, (int) (b + '0'));
      } else if (xx != x || yy != y) {
        printf(" release");
        show_click(yy, xx, '*');
      }
      x = xx;
      y = yy;
      report += 4;
    }
  }

  rm(the_mode);
  restore_ttymodes();

  vt_move(max_lines - 2, 1);
}

static int
test_dec_locator_event(MENU_ARGS)
{
  show_dec_locator_events(PASS_ARGS, 2, FALSE);
  return MENU_HOLD;
}

static int
test_dec_locator_events(MENU_ARGS)
{
  show_dec_locator_events(PASS_ARGS, 1, FALSE);
  return MENU_HOLD;
}

static int
test_dec_locator_event_p(MENU_ARGS)
{
  show_dec_locator_events(PASS_ARGS, 2, TRUE);
  return MENU_HOLD;
}

static int
test_dec_locator_events_p(MENU_ARGS)
{
  show_dec_locator_events(PASS_ARGS, 1, TRUE);
  return MENU_HOLD;
}

static int
test_dec_locator_rectangle(MENU_ARGS)
{
  show_dec_locator_events(PASS_ARGS, -2, FALSE);
  return MENU_HOLD;
}

static int
test_dec_locator_unfiltered(MENU_ARGS)
{
  show_dec_locator_events(PASS_ARGS, 0, FALSE);
  return MENU_HOLD;
}

/* Any-Event Mouse Tracking */
static int
test_mouse_any_event(MENU_ARGS)
{
  show_mouse_tracking(PASS_ARGS, "?1003");
  return MENU_HOLD;
}

/* Button-Event Mouse Tracking */
static int
test_mouse_button_event(MENU_ARGS)
{
  show_mouse_tracking(PASS_ARGS, "?1002");
  return MENU_HOLD;
}

/* Mouse Highlight Tracking */
static int
test_mouse_hilite(MENU_ARGS)
{
  const int first = 10;
  const int last = 20;
  unsigned y = 0, x = 0;
  unsigned start_x, end_x;
  unsigned start_y, end_y;
  unsigned mouse_y, mouse_x;
  int pos;

first:
  vt_move(1, 1);
  ed(0);
  println(the_title);
  show_mousehelp();
  println("Mouse events will be marked with the button number.");
  printf("Highlighting range is [%d..%d)\n", first, last);
  show_hilite(first, last);

  sm("?1001");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (;;) {
    char *report = instr();
    if (isQuit(*report)) {
      break;
    } else if (isReport(*report)) {
      show_mousemodes();
      goto first;
    } else if (isClear(*report)) {
      goto first;
    }
    show_hilite(first, last);
    ToData(1);
    vt_el(2);
    chrprint(report);
    if ((report = skip_csi(report)) != 0) {
      if (*report == 'M'
          && strlen(report) >= 4) {
        unsigned b = MCHR(report[1]);

        b &= 7;
        pos = 2;
        x = xterm_coord(report, &pos);
        y = xterm_coord(report, &pos);
        if (b != 3) {
          /* send the xterm the highlighting range (it MUST be done first) */
          do_csi("1;%u;%u;%d;%d;T", x, y, 10, 20);
          /* now, show the mouse-click */
          if (b < 3)
            b++;
          show_click(y, x, (int) (b + '0'));
        }
        /* interpret the event */
        ToData(2);
        vt_el(2);
        show_result("tracking: code 0x%x (%d,%d)", b, y, x);
        fflush(stdout);
      } else if (*report == 'T' && strlen(report) == 7) {
        /* interpret the event */
        ToData(2);
        vt_el(2);

        pos = 1;
        start_x = xterm_coord(report, &pos);
        start_y = xterm_coord(report, &pos);
        end_x = xterm_coord(report, &pos);
        end_y = xterm_coord(report, &pos);
        mouse_x = xterm_coord(report, &pos);
        mouse_y = xterm_coord(report, &pos);

        show_result("done: start(%d,%d), end(%d,%d), mouse(%d,%d)",
                    start_y, start_x,
                    end_y, end_x,
                    mouse_y, mouse_x);
        if (start_y != y
            || start_x != x)
          show_click(start_y, start_x, 's');
        if (end_y != y
            || end_x != x)
          show_click(end_y, end_x, 'e');
        if (mouse_y != y
            || mouse_x != x)
          show_click(mouse_y, mouse_x, 'm');
      } else if (*report == 't' && strlen(report) == 3) {
        /* interpret the event */
        ToData(2);
        vt_el(2);

        pos = 1;
        end_x = xterm_coord(report, &pos);
        end_y = xterm_coord(report, &pos);

        show_result("done: end(%d,%d)",
                    end_y, end_x);
        if (end_y != y
            || end_x != x)
          show_click(end_y, end_x, 'e');
      }
    }
  }

  rm("?1001");
  restore_ttymodes();

  vt_move(max_lines - 2, 1);
  return MENU_HOLD;
}

/* Normal Mouse Tracking */
static int
test_mouse_normal(MENU_ARGS)
{
  show_mouse_tracking(PASS_ARGS, "?1000");
  return MENU_HOLD;
}

/* X10 Mouse Compatibility */
static int
test_X10_mouse(MENU_ARGS)
{
first:
  vt_move(1, 1);
  ed(0);
  println(the_title);
  show_mousehelp();
  println("Mouse events will be marked with the button number.");

  sm("?9");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (;;) {
    char *report = instr();
    if (isQuit(*report)) {
      break;
    } else if (isReport(*report)) {
      show_mousemodes();
      goto first;
    } else if (isClear(*report)) {
      goto first;
    }
    ToData(0);
    vt_el(2);
    chrprint(report);
    if ((report = skip_csi(report)) != 0
        && *report == 'M'
        && strlen(report) >= 4) {
      int pos = 2;
      int x = (int) xterm_coord(report, &pos);
      int y = (int) xterm_coord(report, &pos);
      cup(y, x);
      printf("%u", MCHR(report[1]) + 1);
      vt_move(y, x);
      fflush(stdout);
    }
  }

  rm("?9");
  restore_ttymodes();

  vt_move(max_lines - 2, 1);
  return MENU_HOLD;
}

/*
 * DEC locator events are implemented on DECterm, to emulate VT220.
 */
static int
tst_dec_locator_events(MENU_ARGS)
{
  static char pixel_screensize[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                0 },
    { "One-Shot",                                            test_dec_locator_event },
    { "Repeated",                                            test_dec_locator_events },
    { "One-Shot (pixels)",                                   test_dec_locator_event_p },
    { "Repeated (pixels)",                                   test_dec_locator_events_p },
    { "Filter Rectangle",                                    test_dec_locator_rectangle },
    { "Filter Rectangle (unfiltered)",                       test_dec_locator_unfiltered },
    { pixel_screensize,                                      get_screensize },
    { "",                                                    0 }
  };
  /* *INDENT-ON* */

  chars_high = 24;
  chars_wide = 80;
  pixels_high = -1;
  pixels_wide = -1;
  do {
    vt_clear(2);
    __(title(0), println("DEC Locator Events"));
    __(title(2), println("Choose test type:"));
    if (pixels_high > 0 && pixels_wide > 0) {
      sprintf(pixel_screensize,
              "XFree86 xterm: screensize %dx%d chars, %dx%d pixels",
              chars_high, chars_wide, pixels_high, pixels_wide);
    } else {
      strcpy(pixel_screensize, "XFree86 xterm: screensize unknown");
    }
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/*
 * The "extended-mouse" control allows sending coordinates larger than 8-bits.
 */
static int
toggle_ExtMouse(MENU_ARGS)
{
  do_ExtMouse = !do_ExtMouse;
  if (do_ExtMouse)
    sm("?1005");
  else
    rm("?1005");
  return MENU_NOHOLD;
}

/*
 * The "focus-event" control adds an event when entering/leaving the window.
 */
static int
toggle_FocusEvent(MENU_ARGS)
{
  do_FocusEvent = !do_FocusEvent;
  if (do_FocusEvent)
    sm("?1004");
  else
    rm("?1004");
  return MENU_NOHOLD;
}

/*
 * xterm generally implements mouse escape sequences (except for dtterm and
 * DECterm).  XFree86 xterm (and newer) implements additional controls.
 */
int
tst_mouse(MENU_ARGS)
{
  static char txt_ExtMouse[80];
  static char txt_FocusEvent[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                0 },
    { txt_ExtMouse,                                          toggle_ExtMouse },
    { txt_FocusEvent,                                        toggle_FocusEvent },
    { "X10 Mouse Compatibility",                             test_X10_mouse },
    { "Normal Mouse Tracking",                               test_mouse_normal },
    { "Mouse Highlight Tracking",                            test_mouse_hilite },
    { "Mouse Any-Event Tracking (XFree86 xterm)",            test_mouse_any_event },
    { "Mouse Button-Event Tracking (XFree86 xterm)",         test_mouse_button_event },
    { "DEC Locator Events (DECterm)",                        tst_dec_locator_events },
    { "",                                                    0 }
  };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    sprintf(txt_ExtMouse, "Mode: %s coordinates",
            do_ExtMouse ? "UTF-8" : "normal");
    sprintf(txt_FocusEvent, "Mode: %sFocus-event",
            do_FocusEvent ? "" : "no");
    __(title(0), println("XTERM mouse features"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}
