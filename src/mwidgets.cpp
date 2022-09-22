#include <allegro.h>

#include <fudgefont.h>

#include <TGUI/tgui.h>

#include "monster.h"
#include "gui.h"
#include "util.h"

static BITMAP* mArrowD = 0;	// down arrow (unselected)
static BITMAP* mArrowDS = 0;	// down arrow (selected)
static BITMAP* mArrowL = 0;	// left arrow (unselected)
static BITMAP* mArrowLS = 0;	// left arrow (selected)
static BITMAP* mArrowR = 0;	// right arrow (unselected)
static BITMAP* mArrowRS = 0;	// right arrow (selected)
static BITMAP* mBottom = 0;	// window bottom border
static BITMAP* mButtonSM = 0;	// button (selected middle)
static BITMAP* mButtonSS = 0;	// button (selected left side)
static BITMAP* mButtonUM = 0;	// button (unselected middle)
static BITMAP* mButtonUS = 0;	// button (unselected side)
static BITMAP* mCheckedS = 0;	// checkbox (checked selected)
static BITMAP* mCheckedU = 0;	// checkbox (checked unselected)
static BITMAP* mCorner = 0;	// window top left corner
static BITMAP* mLeft = 0;	// window left border
static BITMAP* mRight = 0;	// window right border
static BITMAP* mTabS = 0;	// slider tab
static BITMAP* mTabU = 0;	// slider tab
static BITMAP* mTop = 0;	// window top border
static BITMAP* mUncheckedS = 0;	// checkbox (unchecked selected)
static BITMAP* mUncheckedU = 0;	// checkbox (unchecked unselected)
static BITMAP* mTabCorner = 0;   // bottom left corner of frame tab

static int checkPadding = 4;
static int optionPadding = 4;

static FONT* mFont = 0;

DATAFILE* dat = 0;

const char SPEECH_STOP_CHAR = '$';
const int SPEECH_DELAY = 80;

const unsigned int ADVANCER_BLINK_DELAY = 500;

FONT* tinyfont = 0;

bool useBitmapFont = true;

int mWgtFontHeight()
{
	return text_height(mFont);
}

int mWgtTextWidth(const char* text)
{
	return text_length(mFont, text);
}

/*
 * This must be called before using any other routines in
 * the FL widgets.
 */
void mWgtInit() throw (std::bad_alloc)
{
	static bool inited = false;
	
	if (inited) {
		destroy_font(mFont);
		unload_datafile(dat);
	}

	int size = 10;
	PALETTE pal;

	if (useBitmapFont) {
		mFont = load_font(getResource("bmpfont.pcx"), pal, &size);
	}
	else {
		install_fudgefont();
		mFont = load_font(getResource("font.ttf"), pal, &size);
	}

	if (!mFont)
		throw new std::bad_alloc();
	
	select_palette(pal);

	/*
	 * Load the GUI bitmaps.
	 */
	dat = load_datafile(getResource("gui.dat"));
	if (!dat) {
		destroy_font(mFont);
		throw new std::bad_alloc();
	}

	mArrowD = (BITMAP*)dat[ARROWDU_BMP].dat;
	mArrowDS = (BITMAP*)dat[ARROWDS_BMP].dat;
	mArrowL = (BITMAP*)dat[ARROWL_BMP].dat;
	mArrowLS = (BITMAP*)dat[ARROWLS_BMP].dat;
	mArrowR = (BITMAP*)dat[ARROWR_BMP].dat;
	mArrowRS = (BITMAP*)dat[ARROWRS_BMP].dat;
	mBottom = (BITMAP*)dat[BOTTOM_BMP].dat;
	mButtonSM = (BITMAP*)dat[BUTTONSM_BMP].dat;
	mButtonSS = (BITMAP*)dat[BUTTONSS_BMP].dat;
	mButtonUM = (BITMAP*)dat[BUTTONUM_BMP].dat;
	mButtonUS = (BITMAP*)dat[BUTTONUS_BMP].dat;
	mCheckedS = (BITMAP*)dat[CHECKEDS_BMP].dat;
	mCheckedU = (BITMAP*)dat[CHECKEDU_BMP].dat;
	mCorner = (BITMAP*)dat[CORNER_BMP].dat;
	mLeft = (BITMAP*)dat[LEFT_BMP].dat;
	mRight = (BITMAP*)dat[RIGHT_BMP].dat;
	mTabS = (BITMAP*)dat[TABS_BMP].dat;
	mTabU = (BITMAP*)dat[TABU_BMP].dat;
	mTop = (BITMAP*)dat[TOP_BMP].dat;
	mUncheckedS = (BITMAP*)dat[UNCHECKEDS_BMP].dat;
	mUncheckedU = (BITMAP*)dat[UNCHECKEDU_BMP].dat;
	mTabCorner = (BITMAP*)dat[TABCORNER_BMP].dat;

	tinyfont = load_font(getResource("battle_gfx/tinyfont.pcx"), 0, 0);

	inited = true;
}

void mWgtShutdown()
{
	if (mFont)
		destroy_font(mFont);
	if (dat)
		unload_datafile(dat);
}

static void set_font_color(int br, int bg, int bb, int fr, int fg, int fb)
{
	fudgefont_color_range(br, bg, bb, fr, fg, fb);
}

void mPrintf(BITMAP* bmp, int x, int y, int fore, int back, bool center,
		bool shadow, const char* fmt, ...)
{
	va_list ap;
	char text[1000];

	va_start(ap, fmt);
	vsnprintf(text, (sizeof(text)/sizeof(*text))-1, fmt, ap);

	int orig_fore = fore;
	int orig_back = useBitmapFont ? 0 : back;

	fore = back = -1;

	if (shadow) {
		if (!useBitmapFont)
			set_font_color(getr(orig_back), getg(orig_back), getb(orig_back),
					0, 0, 0);
		else {
			fore = orig_back;
			back = -1;
		}

		if (center) {
			textprintf_centre_ex(bmp, mFont, x, y-1, fore, back, "%s", text);
			textprintf_centre_ex(bmp, mFont, x-1, y, fore, back, "%s", text);
			textprintf_centre_ex(bmp, mFont, x+1, y, fore, back, "%s", text);
			textprintf_centre_ex(bmp, mFont, x, y+1, fore, back, "%s", text);
		}
		else {
			textprintf_ex(bmp, mFont, x, y-1, fore, back, "%s", text);
			textprintf_ex(bmp, mFont, x-1, y, fore, back, "%s", text);
			textprintf_ex(bmp, mFont, x+1, y, fore, back, "%s", text);
			textprintf_ex(bmp, mFont, x, y+1, fore, back, "%s", text);
		}

		if (!useBitmapFont)
			set_font_color(getr(0), getg(0), getb(0),
					getr(orig_fore), getg(orig_fore), getb(orig_fore));
		else {
			fore = orig_fore;
			back = -1;
		}

		if (center)
			textprintf_centre_ex(bmp, mFont, x, y, fore, back, "%s", text);
		else
			textprintf_ex(bmp, mFont, x, y, fore, back, "%s", text);
	}
	else {
		if (!useBitmapFont)
			set_font_color(getr(orig_back), getg(orig_back), getb(orig_back),
					getr(orig_fore), getg(orig_fore), getb(orig_fore));
		else {
			fore = orig_fore;
			back = -1;
		}

		if (center)
			textprintf_centre_ex(bmp, mFont, x, y, fore, back, "%s", text);
		else
			textprintf_ex(bmp, mFont, x, y, fore, back, "%s", text);
	}
}

void mPrintf2(BITMAP* bmp, int x, int y, int fore, int back, bool center,
		bool shadow, const char* fmt, ...)
{
	va_list ap;
	char text[1000];

	va_start(ap, fmt);
	vsnprintf(text, (sizeof(text)/sizeof(*text))-1, fmt, ap);

	int orig_fore = fore;
	int orig_back = useBitmapFont ? 0 : back;

	fore = back = -1;

	if (shadow) {
		if (!useBitmapFont)
			set_font_color(getr(orig_back), getg(orig_back), getb(orig_back),
					0, 0, 0);
		else {
			fore = orig_back;
			back = -1;
		}

		if (center) {
			textprintf_centre_ex(bmp, mFont, x, y-1, fore, back, "%s", text);
			textprintf_centre_ex(bmp, mFont, x-1, y, fore, back, "%s", text);
			textprintf_centre_ex(bmp, mFont, x+1, y, fore, back, "%s", text);
			textprintf_centre_ex(bmp, mFont, x, y+1, fore, back, "%s", text);
		}
		else {
			textprintf_ex(bmp, mFont, x, y-1, fore, back, "%s", text);
			textprintf_ex(bmp, mFont, x-1, y, fore, back, "%s", text);
			textprintf_ex(bmp, mFont, x+1, y, fore, back, "%s", text);
			textprintf_ex(bmp, mFont, x, y+1, fore, back, "%s", text);
		}
	}
	else {
		if (!useBitmapFont)
			set_font_color(getr(orig_back), getg(orig_back), getb(orig_back),
					getr(orig_fore), getg(orig_fore), getb(orig_fore));
		else {
			fore = orig_fore;
			back = -1;
		}

		if (center)
			textprintf_centre_ex(bmp, mFont, x, y, fore, back, "%s", text);
		else
			textprintf_ex(bmp, mFont, x, y, fore, back, "%s", text);
	}
}

/*
 * Interpolate between two colors
 * Pass 0.0-1.0 for 100% color1 -> 100% color2
 */ 
int interpolateColor(float ratio, int color1, int color2)
{
	int r1 = getr(color1);
	int g1 = getg(color1);
	int b1 = getb(color1);
	int r = +(int)(r1 + ((getr(color2) - r1) * ratio));
	int g = +(int)(g1 + ((getg(color2) - g1) * ratio));
	int b = +(int)(b1 + ((getb(color2) - b1) * ratio));

	return makecol(r, g, b);
}

void drawFrame(BITMAP* bmp, int x, int y, int width, int height,
		int color, int alpha)
{
	int corners_width = mCorner->w * 2;
	int horiz_width = width - corners_width;
	int vert_height = height - corners_width;

	set_trans_blender(0, 0, 0, alpha);
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

	bool smoothBg = false;
	int topColor;
	int r = MIN(getr(color)+100, 255);
	int g = MIN(getg(color)+100, 255);
	int b = MIN(getb(color)+100, 255);
	topColor = makecol(r, g, b);

	for (int i = 0; i < vert_height+5; i++) {
		float ratio;
		if (smoothBg) {
			ratio = (float)i / (float)(vert_height+5);
		}
		else {
			int section = (i * 100) / (vert_height+5) *
				20 / 100;
			ratio = (float)section / 20;
		}
		int lineColor = interpolateColor(ratio, topColor, color);
		int ly = y+mCorner->h-2+i;
		line(bmp, x+mCorner->w-2, ly, x+mCorner->w+horiz_width+3,
			       	ly, lineColor);
	}

	solid_mode();

	draw_sprite(bmp, mCorner, x, y);
	draw_sprite_h_flip(bmp, mCorner, x+horiz_width+mCorner->w, y);
	draw_sprite_v_flip(bmp, mCorner, x, y+vert_height+mCorner->h);
	draw_sprite_vh_flip(bmp, mCorner, x+horiz_width+mCorner->w,
			y+vert_height+mCorner->h);
	stretch_blit(mTop, bmp, 0, 0, 1, mTop->h, x+mCorner->w,
			y, horiz_width, mTop->h);
	stretch_blit(mBottom, bmp, 0, 0, 1, mTop->h, x+mCorner->w,
			y+height-mBottom->h, horiz_width, mBottom->h);
	stretch_blit(mLeft, bmp, 0, 0, mLeft->w, 1, x, y+mCorner->h,
			mLeft->w, vert_height);
	stretch_blit(mRight, bmp, 0, 0, mLeft->w, 1,
			x+width-mRight->w, y+mCorner->h,
			mLeft->w, vert_height);
}

bool MWidget::pointOnWidget(int mx, int my)
{
	int x1;
	int y1;
	int x2;
	int y2;

	x1 = x;
	x2 = x1 + width - 1;
	y1 = y;
	y2 = y1 + height - 1;

	if (mx >= x1 && my >= y1 && mx <= x2 && my <= y2)
		return true;
	return false;
}

int MWgtFrame::getColor()
{
	return color;
}

void MWgtFrame::draw()
{
	drawFrame(tguiBitmap, x, y, width, height, color, alpha);
}

MWgtFrame::MWgtFrame(int x, int y, int width, int height, int color,
		int alpha)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->color = color;
	this->alpha = alpha;
	this->hotkeys = 0;
}

MWgtFrame::~MWgtFrame()
{
}

void MWgtButton::draw()
{
	BITMAP* side;
	BITMAP* mid;

	if (focus) {
		side = mButtonSS;
		mid = mButtonSM;
	}
	else {
		side = mButtonUS;
		mid = mButtonUM;
	}

	int midwid = width - (side->w * 2);
	draw_sprite(tguiBitmap, side, x, y);
	stretch_blit(mid, tguiBitmap, 0, 0, mid->w, mid->h, x+side->w, y,
			midwid, mid->h);
	draw_sprite_h_flip(tguiBitmap, side, x+side->w+midwid, y);

	int ty = y + (mid->h/2) - (text_height(mFont)/2);

	if (focus) {
		mPrintf(tguiBitmap, x+(width/2), ty,
			makecol(255, 255, 0),
			makecol(0, 0, 0),
			true, false, text);
	}
	else {
		mPrintf(tguiBitmap, x+(width/2), ty,
			makecol(255, 255, 255),
			makecol(0, 0, 0),
			true, false, text);
	}
}

int MWgtButton::update(int millis)
{
	if (pressed) {
		pressed = false;
		return TGUI_RETURN;
	}
	return TGUI_CONTINUE;
}

bool MWgtButton::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == key1 || k == KEY_ENTER) {
		playOgg("button.ogg");
		pressed = true;
		return true;
	}
	else if (k == keyLeft || k == keyUp) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyRight || k == keyDown) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtButton::acceptsFocus()
{
	return true;
}

MWgtButton::MWgtButton(int x, int y, int width, char* text)
{
	this->x = x;
	this->y = y;
	if (width < 0)
		this->width = text_length(mFont, text) +
			(mButtonSS->w*2) + 1;
	else
		this->width = width;
	this->height = mButtonSS->h;
	this->text = text;
	this->pressed = false;
	this->hotkeys = 0;
}

MWgtButton::~MWgtButton()
{
}

void MWgtText::setColor(int color)
{
	this->color = color;
}

void MWgtText::draw()
{
	int bg = ((MWgtFrame*)parent)->getColor();
	mPrintf(tguiBitmap, x, y, color, bg, false,
		true, text);
}

void MWgtText::setText(const char *text)
{
	this->text = text;
}

MWgtText::MWgtText(int x, int y, const char* text, int color)
{
	this->x = x;
	this->y = y;
	this->width = text_length(mFont, text);
	this->height = text_height(mFont);
	this->text = text;
	this->color = color;
	this->hotkeys = 0;
}

bool MWgtCheckbox::isChecked()
{
	return checked;
}

void MWgtCheckbox::setChecked(bool checked)
{
	this->checked = checked;
}

void MWgtCheckbox::draw()
{
	if (checked) {
		if (focus) {
			draw_sprite(tguiBitmap, mCheckedS, x, y);
		}
		else {
			draw_sprite(tguiBitmap, mCheckedU, x, y);
		}
	}
	else {
		if (focus) {
			draw_sprite(tguiBitmap, mUncheckedS, x, y);
		}
		else {
			draw_sprite(tguiBitmap, mUncheckedU, x, y);
		}
	}

	int bg = ((MWgtFrame*)parent)->getColor();
	int ty = y + (mCheckedS->h/2) - (text_height(mFont)/2);

	if (focus) {
		mPrintf(tguiBitmap, x+mCheckedS->w+checkPadding, ty,
			makecol(255, 255, 0), bg, false,
			true, text);
	}
	else {
		mPrintf(tguiBitmap, x+mCheckedS->w+checkPadding, ty,
			makecol(255, 255, 255), bg, false,
			true, text);
	}
}

bool MWgtCheckbox::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == key1 || k == KEY_ENTER) {
		playOgg("button.ogg");
		checked = !checked;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == keyUp || k == keyLeft) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyDown || k == keyRight) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtCheckbox::acceptsFocus()
{
	return true;
}
	
MWgtCheckbox::MWgtCheckbox(int x, int y, char* text)
{
	this->x = x;
	this->y = y;
	this->width = mCheckedS->w + checkPadding + text_length(mFont, text);
	this->height = text_height(mFont);
	this->text = text;
	this->checked = false;
	this->hotkeys = 0;
}

MWgtCheckbox::~MWgtCheckbox()
{
}

unsigned int MWgtOption::getSelected()
{
	return selected;
}

void MWgtOption::setSelected(unsigned int selected)
{
	this->selected = selected;
}

void MWgtOption::draw()
{
	BITMAP* arrowl;
	BITMAP* arrowr;
	int color;

	if (focus) {
		arrowl = mArrowLS;
		arrowr = mArrowRS;
		color = makecol(255, 255, 0);
	}
	else {
		arrowl = mArrowL;
		arrowr = mArrowR;
		color = makecol(255, 255, 255);
	}

	draw_sprite(tguiBitmap, arrowl, x, y);
	draw_sprite(tguiBitmap, arrowr, x+width-arrowr->w, y);
	int ty = y + arrowl->h/2 - text_height(mFont)/2;
	int bg = ((MWgtFrame*)parent)->getColor();
	mPrintf(tguiBitmap, x+(width/2), ty, color, bg, true,
		true, (*options)[selected]);
}

bool MWgtOption::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == keyLeft) {
		if (selected)
			selected--;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == keyRight) {
		if (selected < (options->size()-1))
			selected++;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == keyUp) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyDown) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtOption::acceptsFocus()
{
	return true;
}
	
MWgtOption::MWgtOption(int x, int y, int width, std::vector<char*>* options)
{
	this->x = x;
	this->y = y;
	this->options = options;
	if (width < 0)
		this->width = findWidth();
	else
		this->width = width;
	this->height = MAX(mArrowL->h, text_height(mFont));
	this->hotkeys = 0;
	this->selected = 0;
}

MWgtOption::~MWgtOption()
{
}

int MWgtOption::findWidth()
{
	int w = optionPadding * 2;

	w += mArrowL->w;
	w += mArrowR->w;

	int max = 0;

	for (unsigned int i = 0; i < options->size(); i++) {
		if (text_length(mFont, (*options)[i]) > max) {
			max = text_length(mFont, (*options)[i]);
		}
	}

	w += max;

	return w;
}

int MWgtSlider::getStop()
{
	return currStop;
}

void MWgtSlider::setStop(int stop)
{
	currStop = stop;
}

int MWgtSlider::getStopPos(int stop)
{
	if (stop == 0)
		return x + (mTabS->w / 2);
	else if (stop >= (numStops-1))
		return x+width-(mTabS->w / 2);
	else {
		return x + (width / (numStops - 1) * stop);
	}
}

void MWgtSlider::draw()
{
	int ly = y + mTabS->h / 2;

	line(tguiBitmap, x, ly, x+width-1, ly, makecol(0, 0, 0));

	int tx = getStopPos(currStop) - mTabS->w/2;

	if (focus) {
		draw_sprite(tguiBitmap, mTabS, tx, y);
	}
	else {
		draw_sprite(tguiBitmap, mTabU, tx, y);
	}
}

bool MWgtSlider::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == keyLeft) {
		if (currStop)
			currStop--;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == keyRight) {
		if (currStop < (numStops-1))
			currStop++;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == keyUp) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyDown) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtSlider::acceptsFocus()
{
	return true;
}

MWgtSlider::MWgtSlider(int x, int y, int width, int numStops)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = mTabS->h;
	this->numStops = MIN(width, numStops);
	this->currStop = numStops/2;
	this->hotkeys = 0;
}

MWgtSlider::~MWgtSlider()
{
}

void MWgtIcon::draw()
{
	if (focus && selectable) {
		draw_sprite(tguiBitmap, selected, x, y);
	}
	else {
		draw_sprite(tguiBitmap, unselected, x, y);
	}
}

int MWgtIcon::update(int millis)
{
	if (pressed) {
		pressed = false;
		return TGUI_RETURN;
	}
	return TGUI_CONTINUE;
}

bool MWgtIcon::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == key1 || k == KEY_ENTER) {
		playOgg("button.ogg");
		pressed = true;
		return true;
	}
	else if (k == keyLeft || k == keyUp) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyRight || k == keyDown) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtIcon::acceptsFocus()
{
	return selectable;
}

MWgtIcon::MWgtIcon(int x, int y, BITMAP* unselected, BITMAP* selected,
		bool selectable)
{
	this->x = x;
	this->y = y;
	this->width = unselected->w;
	this->height = unselected->h;
	this->unselected = unselected;
	this->selected = selected;
	this->selectable = selectable;
	this->hotkeys = 0;
	pressed = false;
}

MWgtIcon::~MWgtIcon()
{
}
	
bool MWgtSpeech::advance()
{
	if (text[currChar] == 0)
		return false;
	else if (text[currChar] == SPEECH_STOP_CHAR) {
		currChar++;
		startChar = currChar;
		if (text[currChar] == 0)
			return false;
	}
	else {
		while (text[currChar] != SPEECH_STOP_CHAR)
			currChar++;
	}
	return true;
}

bool MWgtSpeech::atStop()
{
	return (text[currChar] == SPEECH_STOP_CHAR);
}

/*
 * Returns the number of lines in the biggest block
 * in the text.
 */
int MWgtSpeech::getMaxLines()
{
	startChar = 0;
	currChar = 0;
	maxLines = 0;
	int lines;

	while (advance()) {
		lines = 1;
		int c = startChar;
		int currX = x;
		int currY = y;
		while (c < currChar) {
			if (text[c] == ' ') {
				char word[100];
				int i;
				for (i = c+1; text[i] != ' ' && text[i] != SPEECH_STOP_CHAR && text[i]; i++)
					word[i-c-1] = text[i];
				word[i-c-1] = 0;
				if ((currX + text_length(mFont, "  ") +
						text_length(mFont, word)) > (x+width)) {
					currX = x;
					lines++;
				}
				else {
					currX += text_length(mFont, "  ");
				}
			}
			else {
				char s[2];
				s[0] = text[c];
				s[1] = 0;
				currX += text_length(mFont, s);
			}
			c++;
		}
		if (lines > maxLines)
			maxLines = lines;
		if (!advance())
			break;
	}

	startChar = 0;
	currChar = 0;

	return maxLines;
}

void MWgtSpeech::draw()
{
	int bg = ((MWgtFrame*)parent)->getColor();

	bool shadow = true;
	
	for (int loops = 0; loops < 2; loops++) {
		int currX = x;
		int currY = y;
		int c = startChar;

		while (c < currChar) {
			if (text[c] == ' ') {
				char word[100];
				int i;
				for (i = c+1; text[i] != ' ' && text[i] != SPEECH_STOP_CHAR && text[i]; i++)
					word[i-c-1] = text[i];
				word[i-c-1] = 0;
				if ((currX + text_length(mFont, "  ") +
						text_length(mFont, word)) > (x+width)) {
					currX = x;
					currY += text_height(mFont)+3;
				}
				else {
					currX += text_length(mFont, "  ");
				}
			}
			else {
				char s[2];
				s[0] = text[c];
				s[1] = 0;
				mPrintf2(tguiBitmap, currX, currY, makecol(255, 255, 255),
					bg, false, shadow, s);
				if (shadow && !useBitmapFont)
					mPrintf2(tguiBitmap, currX, currY, makecol(255, 255, 255),
						bg, false, shadow, s);
				currX += text_length(mFont, s);
			}
			c++;
		}
		shadow = false;
	}
}

int MWgtSpeech::update(int step)
{
	if (text[currChar] == 0)
		return TGUI_RETURN;

	count += step;
	if (count >= SPEECH_DELAY) {
		count = 0;
		if (text[currChar] != SPEECH_STOP_CHAR)
			currChar++;
	}

	tguiMakeDirty(this);

	return TGUI_CONTINUE;
}

/*
 * Speech format:
 * Some Text.$Second paragraph.$
 */
MWgtSpeech::MWgtSpeech(int x, int y, int width, const char* text)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = 0;
	this->text = text;
	this->currChar = 0;
	this->startChar = 0;
	this->count = 0;
	this->hotkeys = 0;
}

MWgtSpeech::~MWgtSpeech()
{
}

void MWgtAdvancer::draw()
{
	bool yellow = (unsigned)tguiCurrentTimeMillis() %
		(ADVANCER_BLINK_DELAY*4) < ADVANCER_BLINK_DELAY;

	if (target->atStop()) {
		if (yellow)
			draw_sprite(tguiBitmap, mArrowDS, x, y);
		else
			draw_sprite(tguiBitmap, mArrowD, x, y);
	}
	else {
		if (yellow) {
			draw_sprite(tguiBitmap, mArrowRS, x, y);
			draw_sprite(tguiBitmap, mArrowRS, x+4, y);
		}
		else {
			draw_sprite(tguiBitmap, mArrowR, x, y);
			draw_sprite(tguiBitmap, mArrowR, x+4, y);
		}
	}
}

bool MWgtAdvancer::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == key1 || k == KEY_ENTER) {
		target->advance();
		return true;
	}
	return false;
}

MWgtAdvancer::MWgtAdvancer(int x, int y, MWgtSpeech* target)
{
	this->x = x;
	this->y = y;
	this->width = MAX(mArrowDS->w, mArrowRS->w);
	this->height = MAX(mArrowDS->h, mArrowRS->h);
	this->target = target;
	this->hotkeys = 0;
}

void MWgtTextButton::draw()
{
	int bg;

	if (parent)
		bg = ((MWgtFrame*)parent)->getColor();
	else
		bg = makecol(0, 96, 160);

	if (focus) {
		mPrintf(tguiBitmap, x, y,
			makecol(255, 255, 0),
			bg,
			false, true, text);
	}
	else {
		mPrintf(tguiBitmap, x, y,
			makecol(255, 255, 255),
			bg,
			false, true, text);
	}
}

int MWgtTextButton::update(int millis)
{
	if (pressed) {
		pressed = false;
		return TGUI_RETURN;
	}
	return TGUI_CONTINUE;
}

bool MWgtTextButton::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == key1 || k == KEY_ENTER) {
		playOgg("button.ogg");
		pressed = true;
		return true;
	}
	else if (k == keyLeft || k == keyUp) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyRight || k == keyDown) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtTextButton::acceptsFocus()
{
	return true;
}

MWgtTextButton::MWgtTextButton(int x, int y, char* text)
{
	this->x = x;
	this->y = y;
	this->width = text_length(mFont, text);
	this->height = text_height(mFont);
	this->text = text;
	this->pressed = false;
	this->hotkeys = 0;
}

MWgtTextButton::~MWgtTextButton()
{
}

void MWgtInventory::draw()
{
	int bg = ((MWgtFrame*)parent)->getColor();
	lua_State* luaState = currArea->getLuaState();

	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < columns; col++) {
			int i = (row*columns)+col+head;
			if (i >= MAX_INVENTORY)
				break;
			int color;
			if (arrange && i == pressed1) {
				color = makecol(0, 255, 0);
			}
			else if (i == selected) {
				color = makecol(255, 255, 0);
			}
			else {
				color = makecol(255, 255, 255);
			}
			int xx = x + width * col / columns;
			int yy = y + row * text_height(mFont);
			if (arrange && i == pressed1) {
				int ay = yy + text_height(mFont)/2 - mArrowR->h/2;
				draw_sprite(tguiBitmap, mArrowR, xx+3, ay);
			}
			if (i == selected) {
				int ay = yy + text_height(mFont)/2 - mArrowRS->h/2;
				draw_sprite(tguiBitmap, mArrowRS, xx, ay);
			}
			if (inventory[i].id < 0)
				continue;
			char text[200];
			callLua(luaState, "get_item_name", "i>s", inventory[i].id+1);
			snprintf(text, (sizeof(text)/sizeof(*text))-1, "%s (%d)", lua_tostring(luaState, -1), inventory[i].quantity);
			lua_pop(luaState, 1);
			mPrintf(tguiBitmap, xx+16, yy, color, bg, false,
				true, text);
		}
	}
}

int MWgtInventory::update(int step)
{
	if (done) {
		done = false;
		return TGUI_RETURN;
	}
	else
		return TGUI_CONTINUE;
}

bool MWgtInventory::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == KEY_ENTER || k == key1) {
		playOgg("button.ogg");
		if (arrange) {
			if (pressed1 < 0) {
				pressed1 = selected;
			}
			else {
				pressed2 = selected;
				done = true;
			}
		}
		else {
			pressed1 = selected;
			done = true;
		}
	}
	else if (k == key2) {
		playOgg("button.ogg");
		if (arrange) {
			if (pressed1 < 0) {
				done = true;
			}
			else {
				pressed1 = -1;
			}
		}
		else {
			pressed1 = -1;
			done = true;
		}
	}
	else if (k == keyLeft) {
		if (selected) {
			selected--;
			if (selected < head)
				head -= columns;
		}
	}
	else if (k == keyRight) {
		if (selected+1 < MAX_INVENTORY) {
			selected++;
			if ((selected-head) >= (rows*columns)) {
				head += columns;
			}
		}
	}
	else if (k == keyUp) {
		selected -= columns;
		if (selected < 0)
			selected = 0;
		if (selected < head)
			head -= columns;
	}
	else if (k == keyDown) {
		selected += columns;
		if (selected >= MAX_INVENTORY)
			selected = MAX_INVENTORY-1;
		if ((selected-head) >= (rows*columns))
			head += columns;
	}
	else {
		return false;
	}
	tguiMakeDirty(this);
	return true;
}

bool MWgtInventory::acceptsFocus()
{
	return true;
}

void MWgtInventory::getPressed(int* p1, int* p2)
{
	if (p1)
		*p1 = pressed1;
	if (p2)
		*p2 = pressed2;
}

int MWgtInventory::getSelected()
{
	return selected;
}

int MWgtInventory::getHead()
{
	return head;
}

MWgtInventory::MWgtInventory(int x, int y, int width, int columns, int rows,
	bool arrange, int selected, int head, Item* inventory)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = rows*text_height(mFont);
	this->columns = columns;
	this->rows = rows;
	this->arrange = arrange;
	this->selected = selected;
	this->head = head;
	this->inventory = inventory;
	this->pressed1 = -1;
	this->pressed2 = -1;
	this->hotkeys = 0;
	this->done = false;
}

MWgtInventory::~MWgtInventory()
{
}

int MWgtSelector::getPosition()
{
	return position;
}

void MWgtSelector::setPosition(int newpos)
{
	position = newpos;
}

int MWgtSelector::update(int step)
{
	if (done) {
		done = false;
		return TGUI_RETURN;
	}
	return TGUI_CONTINUE;
}

void MWgtSelector::draw()
{
	if (position < 0)
		return;

	int ax = (*positions)[position]->x;
	int ay = (*positions)[position]->y;

	BITMAP* bmp;

	if (left) {
		if (blink && (((unsigned)tguiCurrentTimeMillis() % 1000) < 500)) {
			bmp = mArrowL;
		}
		else {
			bmp = mArrowLS;
		}
	}
	else {
		if (blink && (((unsigned)tguiCurrentTimeMillis() % 1000) < 500)) {
			bmp = mArrowR;
		}
		else {
			bmp = mArrowRS;
		}
	}

	draw_sprite(tguiBitmap, bmp, ax, ay);
}

bool MWgtSelector::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == KEY_ENTER || k == key1) {
		playOgg("button.ogg");
		done = true;
		return true;
	}
	else if (k == keyLeft || k == keyUp) {
		if (position)
			position--;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == keyRight || k == keyDown) {
		if (position+1 < positions->size())
			position++;
		tguiMakeDirty(this);
		return true;
	}
	else if (k == key2) {
		playOgg("button.ogg");
		position = -1;
		done = true;
		return true;
	}

	return false;
}

MWgtSelector::MWgtSelector(std::vector<Point*>* positions, bool left, bool blink)
{
	this->positions = positions;
	this->position = 0;
	this->left = left;
	this->done = false;
	this->hotkeys = 0;
	this->x = (*positions)[0]->x;
	this->y = (*positions)[0]->y;
	this->width = tguiBitmap->w - x;
	this->height = tguiBitmap->h - y;
	this->blink = blink;
}

MWgtSelector::~MWgtSelector()
{
}

void MWgtSpellSelector::draw()
{
	int bg = ((MWgtFrame*)parent)->getColor();
	lua_State* luaState = currArea->getLuaState();

	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < columns; col++) {
			int i = (row*columns)+col+head;
			if (i >= MAX_SPELLS)
				break;
			int color;
			if (arrange && i == pressed1) {
				color = makecol(0, 255, 0);
			}
			else if (i == selected) {
				color = makecol(255, 255, 0);
			}
			else {
				color = makecol(255, 255, 255);
			}
			int xx = x + width * col / columns;
			int yy = y + row * text_height(mFont);
			if (arrange && i == pressed1) {
				int ay = yy + text_height(mFont)/2 - mArrowR->h/2;
				draw_sprite(tguiBitmap, mArrowR, xx+3, ay);
			}
			if (i == selected) {
				int ay = yy + text_height(mFont)/2 - mArrowRS->h/2;
				draw_sprite(tguiBitmap, mArrowRS, xx, ay);
			}
			if (spells[i] == 0)
				continue;
			char text[200];
			snprintf(text, (sizeof(text)/sizeof(*text))-1, "%s", spells[i]);
			mPrintf(tguiBitmap, xx+16, yy, color, bg, false,
				true, text);
		}
	}
}

int MWgtSpellSelector::update(int step)
{
	if (done) {
		done = false;
		return TGUI_RETURN;
	}
	else
		return TGUI_CONTINUE;
}

bool MWgtSpellSelector::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == KEY_ENTER || k == key1) {
		playOgg("button.ogg");
		if (arrange) {
			if (pressed1 < 0) {
				pressed1 = selected;
			}
			else {
				pressed2 = selected;
				done = true;
			}
		}
		else {
			pressed1 = selected;
			done = true;
		}
	}
	else if (k == key2) {
		playOgg("button.ogg");
		if (arrange) {
			if (pressed1 < 0) {
				done = true;
			}
			else {
				pressed1 = -1;
			}
		}
		else {
			pressed1 = -1;
			done = true;
		}
	}
	else if (k == keyLeft) {
		if (selected) {
			selected--;
			if (selected < head)
				head -= columns;
		}
	}
	else if (k == keyRight) {
		if (selected+1 < MAX_INVENTORY) {
			selected++;
			if ((selected-head) >= (rows*columns)) {
				head += columns;
			}
		}
	}
	else if (k == keyUp) {
		selected -= columns;
		if (selected < 0)
			selected = 0;
		if (selected < head)
			head -= columns;
	}
	else if (k == keyDown) {
		selected += columns;
		if (selected >= MAX_INVENTORY)
			selected = MAX_INVENTORY-1;
		if ((selected-head) >= (rows*columns))
			head += columns;
	}
	else {
		return false;
	}
	tguiMakeDirty(this);
	return true;
}

bool MWgtSpellSelector::acceptsFocus()
{
	return true;
}

void MWgtSpellSelector::getPressed(int* p1, int* p2)
{
	if (p1)
		*p1 = pressed1;
	if (p2)
		*p2 = pressed2;
}

int MWgtSpellSelector::getSelected()
{
	return selected;
}

int MWgtSpellSelector::getHead()
{
	return head;
}

MWgtSpellSelector::MWgtSpellSelector(int x, int y, int width, int columns, int rows,
	bool arrange, int selected, int head, char** spells)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = rows*text_height(mFont);
	this->columns = columns;
	this->rows = rows;
	this->arrange = arrange;
	this->selected = selected;
	this->head = head;
	this->spells = spells;
	this->pressed1 = -1;
	this->pressed2 = -1;
	this->hotkeys = 0;
	this->done = false;
}

MWgtSpellSelector::~MWgtSpellSelector()
{
}

int MWgtTabs::getColor()
{
	return color;
}

void MWgtTabs::draw()
{
	int currX = x;

	for (unsigned int i = 0; i < tabNames.size(); i++) {
		int textColor;
		if (i == currTab) {
			rectfill(tguiBitmap, currX - mCorner->w, y,
				currX + mWgtTextWidth(tabNames[i].c_str())+mCorner->w,
				y+mCorner->h+12, color);
			draw_sprite(tguiBitmap, mCorner,
					currX - mCorner->w - 5,
					y-5);
			draw_sprite_h_flip(tguiBitmap, mCorner,
					currX + mWgtTextWidth(tabNames[i].c_str())+ 5,
					y-5);
			stretch_sprite(tguiBitmap, mTop, currX-5, y-5,
					mWgtTextWidth(tabNames[i].c_str())+10,
					mTop->h);
			stretch_sprite(tguiBitmap, mLeft,
					currX - mCorner->w - 5,
					y+mCorner->h-5,
					mLeft->w,
					18);
			stretch_sprite(tguiBitmap, mRight,
					currX + mWgtTextWidth(tabNames[i].c_str())+ 5+mCorner->w-mRight->w,
					y+mCorner->h-5,
					mRight->w,
					18);
			draw_sprite(tguiBitmap, mTabCorner,
				currX - mCorner->w - 5,
				y+mCorner->h-5+18-mTabCorner->h);
			draw_sprite_h_flip(tguiBitmap, mTabCorner,
				currX + mWgtTextWidth(tabNames[i].c_str())+ 5+mCorner->w-mRight->w,
				y+mCorner->h-5+18-mTabCorner->h);
		}
		if (i == currTab && focus) {
			textColor = makecol(255, 255, 0);
		}
		else {
			textColor = makecol(255, 255, 255);
		}
		mPrintf(tguiBitmap, currX, y, textColor, makecol(0, 0, 0),
				false, true, tabNames[i].c_str());
		currX += mWgtTextWidth(tabNames[i].c_str()) + PADDING;
	}
}

bool MWgtTabs::handleKey(int key)
{
	int key1 = config.getKey1();
	int key2 = config.getKey2();
	int keyLeft = config.getKeyLeft();
	int keyRight = config.getKeyRight();
	int keyUp = config.getKeyUp();
	int keyDown = config.getKeyDown();

	int k = (key >> 8);

	if (k == keyLeft) {
		if (currTab > 0) {
			currTab--;
		}
		focusCurrentTab();
		return true;
	}
	else if (k == keyRight) {
		if (currTab < tabNames.size()-1) {
			currTab++;
		}
		focusCurrentTab();
		return true;
	}
	else if (k == keyUp) {
		tguiFocusPrevious();
		return true;
	}
	else if (k == keyDown) {
		tguiFocusNext();
		return true;
	}

	return false;
}

bool MWgtTabs::acceptsFocus()
{
	return true;
}
	
MWgtTabs::MWgtTabs(int x, int y, int color, int alpha,
		std::vector<std::string>& tabNames,
		std::vector<MWgtFrame*>& frames)
{
	this->x = x;
	this->y = y;
	this->hotkeys = 0;
	this->currTab = 0;
	this->tabNames = tabNames;
	this->frames = frames;
	this->color = color;
	this->width = findWidth();
	this->height = findHeight();

	focusCurrentTab();
}

MWgtTabs::~MWgtTabs()
{
}

void MWgtTabs::focusCurrentTab()
{
	setParent(frames[currTab]);
	tguiLowerChildren(frames[currTab]);
	tguiLowerWidget(this);
	tguiLowerSingleWidget(frames[currTab]);
	for (unsigned int i = 0; i < frames.size(); i++) {
		if (i != currTab) {
			tguiLowerWidget(frames[i]);
		}
	}
	tguiMakeDirty(frames[currTab]);
	tguiMakeDirty(this);
}

int MWgtTabs::findWidth()
{
	int width = mWgtTextWidth(tabNames[0].c_str());

	for (unsigned int i = 1; i < tabNames.size(); i++) {
		width += PADDING + mWgtTextWidth(tabNames[i].c_str());
	}

	return width;
}

int MWgtTabs::findHeight()
{
	return mWgtFontHeight()+10;
}

