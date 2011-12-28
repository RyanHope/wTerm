/* $Id: draw.h,v 1.3 2010/05/28 08:15:28 tom Exp $ */

typedef struct {
  int top;
  int left;
  int bottom;
  int right;
} BOX;

extern int make_box_params(BOX *box, int vmargin, int hmargin);
extern void draw_box_outline(BOX *box, int mark);
extern void draw_box_filled(BOX *box, int mark);
extern void draw_box_caption(BOX *box, int margin, const char **c);
