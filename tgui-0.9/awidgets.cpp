#include <allegro.h>

#include "TGUI/tgui.h"
#include "TGUI/awidgets.h"

static int frameDarkenAmount = 60;
static int frameHighlightAmount = 70;
static int frameThickness = 5;
static int frameCaptionHeight = 16;
static int frameBgSections = 10;
static BITMAP* aWgtFrameCloseBmp = 0;

static int buttonPadding = 4;

static BITMAP* checkCheckedBmp = 0;
static BITMAP* checkUncheckedBmp = 0;
static int checkPadding = 8;

static BITMAP* optionLeftBmp = 0;
static BITMAP* optionRightBmp = 0;
static BITMAP* optionLeftDownBmp = 0;
static BITMAP* optionRightDownBmp = 0;
static int optionPadding = 16;
static int optionDefaultHeight = 10;

static BITMAP* sliderTabBmp = 0;
static int sliderDefaultTabWidth = 8;
static int sliderDefaultTabHeight = 16;

static BITMAP* linePatternBmp = 0;
static FONT* aWgtFont = font;

static int aWgtFocusColor;

/*
 * This must be called before using any other routines in
 * the Allegro Widgets
 */
void aWgtInit()
{
	aWgtFont = font;
	if (!linePatternBmp)
		linePatternBmp = create_bitmap(2, 2);
	if (!linePatternBmp)
		throw new std::bad_alloc();
	clear_to_color(linePatternBmp, makecol(255, 0, 255));
	putpixel(linePatternBmp, 0, 0, 0);
	putpixel(linePatternBmp, 1, 1, 0);

	aWgtFocusColor = makecol(0, 0, 0);
}

/*
 * Interpolate between two colors
 * Pass 0.0-1.0 for 100% color1 -> 100% color2
 */ 
int aWgtInterpolateColor(float ratio, int color1, int color2)
{
	int r1 = getr(color1);
	int g1 = getg(color1);
	int b1 = getb(color1);
	int r = +(int)(r1 + ((getr(color2) - r1) * ratio));
	int g = +(int)(g1 + ((getg(color2) - g1) * ratio));
	int b = +(int)(b1 + ((getb(color2) - b1) * ratio));

	return makecol(r, g, b);
}

/*
 * Make a color darker
 */
int aWgtDarken(int color)
{
	int r = MAX(0, getr(color) - frameDarkenAmount);
	int g = MAX(0, getg(color) - frameDarkenAmount);
	int b = MAX(0, getb(color) - frameDarkenAmount);
	return makecol(r, g, b);
}

/*
 * Make a color brighter
 */
int aWgtHighlight(int color)
{
	int r = MIN(255, getr(color) + frameHighlightAmount);
	int g = MIN(255, getg(color) + frameHighlightAmount);
	int b = MIN(255, getb(color) + frameHighlightAmount);
	return makecol(r, g, b);
}

void aWgtSet(int property, void* value)
{
	switch (property) {
		case AWGT_FONT:
			aWgtFont = (FONT*)value;
			break;
		case AWGT_FRAME_CLOSE_BMP:
			aWgtFrameCloseBmp = (BITMAP*)value;
			break;
		case AWGT_CHECKBOX_CHECKED_BMP:
			checkCheckedBmp = (BITMAP*)value;
			break;
		case AWGT_CHECKBOX_UNCHECKED_BMP:
			checkUncheckedBmp = (BITMAP*)value;
			break;
		case AWGT_OPTION_LEFT_BMP:
			optionLeftBmp = (BITMAP*)value;
			break;
		case AWGT_OPTION_RIGHT_BMP:
			optionRightBmp = (BITMAP*)value;
			break;
		case AWGT_OPTION_LEFT_DOWN_BMP:
			optionLeftDownBmp = (BITMAP*)value;
			break;
		case AWGT_OPTION_RIGHT_DOWN_BMP:
			optionRightDownBmp = (BITMAP*)value;
			break;
		case AWGT_SLIDER_TAB_BMP:
			sliderTabBmp = (BITMAP*)value;
			break;
		case AWGT_FOCUS_COLOR:
			aWgtFocusColor = *((int*)value);
			break;
	}
}

void aWgtTextout(BITMAP* bmp, FONT* font, char* text, int x, int y,
		int textColor, int shadowColor, int shadowType)
{
	switch (shadowType) {
		case AWGT_TEXT_NORMAL:
			break;
		case AWGT_TEXT_DROP_SHADOW:
			textout_ex(bmp, font, text, x+1, y+1,
					shadowColor, -1);
			break;
		case AWGT_TEXT_BORDER:
			textout_ex(bmp, font, text, x-1, y,
					shadowColor, -1);
			textout_ex(bmp, font, text, x, y-1,
					shadowColor, -1);
			textout_ex(bmp, font, text, x+1, y,
					shadowColor, -1);
			textout_ex(bmp, font, text, x, y+1,
					shadowColor, -1);
			break;
		case AWGT_TEXT_SQUARE_BORDER:
			textout_ex(bmp, font, text, x-1, y,
					shadowColor, -1);
			textout_ex(bmp, font, text, x-1, y-1,
					shadowColor, -1);
			textout_ex(bmp, font, text, x, y-1,
					shadowColor, -1);
			textout_ex(bmp, font, text, x+1, y-1,
					shadowColor, -1);
			textout_ex(bmp, font, text, x+1, y,
					shadowColor, -1);
			textout_ex(bmp, font, text, x+1, y+1,
					shadowColor, -1);
			textout_ex(bmp, font, text, x, y+1,
					shadowColor, -1);
			textout_ex(bmp, font, text, x-1, y+1,
					shadowColor, -1);
			break;
	}

	textout_ex(bmp, font, text, x, y, textColor, -1);
}

bool AllegroWidget::pointOnWidget(int mx, int my)
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

bool AWgtFrame::clickOnClose(int mx, int my)
{
	int x1;
	int x2;
	int y1;
	int y2;

	if (aWgtFrameCloseBmp) {
		x1 = x + width - 1 - frameThickness -
			aWgtFrameCloseBmp->w;
		x2 = x1 + aWgtFrameCloseBmp->w;
		y1 = y + frameThickness;
		y2 = y1 + aWgtFrameCloseBmp->h;
	}
	else {
		x1 = x + width - 1 - frameThickness -
			frameCaptionHeight;
		x2 = x1 + frameCaptionHeight;
		y1 = y + frameThickness;
		y2 = y1 + frameCaptionHeight;
	}

	if (mx >= x1 && my >= y1 && mx <= x2 && my <= y2)
		return true;
	return false;
}

void AWgtFrame::draw()
{
	int tmpFrameCaptionHeight;

	if (!captionBar) {
		tmpFrameCaptionHeight = frameCaptionHeight;
		frameCaptionHeight = frameThickness;
	}

	int x1 = x + (frameThickness*3);
	int x2 = x + width - 1 - (frameThickness*3);
	int bgHeight = height - (frameThickness*5) - frameCaptionHeight - 1;
	int bgStartY = y + (frameThickness*2) + frameCaptionHeight + 1;

	int i;

	if (alpha < 255) {
		drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
		set_trans_blender(0, 0, 0, alpha);
	}

	for (i = 0; i < bgHeight; i++) {
		float ratio;
		if (smoothBg) {
			ratio = (float)i / (float)bgHeight;
		}
		else {
			int section = (i * 100) / bgHeight *
				frameBgSections / 100;
			ratio = (float)section / frameBgSections;
		}
		int color = aWgtInterpolateColor(ratio, top_color, bottom_color);
		int ly = bgStartY+i;
		line(tguiBitmap, x1, ly, x2, ly, color);
	}

	if (alpha < 255)
		drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

	rect(tguiBitmap, x1+1, bgStartY+1, x2-1, bgStartY+bgHeight-2, bottom_color);

	int endX = x + width - 1;
	int endY = y + height - 1;

	// outer border

	rect(tguiBitmap, x, y, endX, endY,
			aWgtDarken(aWgtDarken(border_color)));

	for (i = 1; i < frameThickness; i++)
		rect(tguiBitmap, x+i, y+i, endX-i, endY-i,
				aWgtDarken(border_color));

	// middle border
	
	int startX = x + frameThickness;
	int startY = y + frameThickness;
	endX = x + width - 1 - frameThickness;
	endY = y + height - 1 - frameThickness;

	for (i = 0; i < frameThickness; i++)
		rect(tguiBitmap, startX+i, startY+i, endX-i, endY-i,
				border_color);

	// caption rect

	startX += frameThickness;
	startY += frameThickness;
	endX -= frameThickness;
	endY = y + frameCaptionHeight + frameThickness;

	rectfill(tguiBitmap, startX, startY, endX, endY, border_color);

	// top inner border

	startY += frameCaptionHeight - frameThickness + 1;
	endY += frameThickness + 1;

	rectfill(tguiBitmap, startX, startY, endX, endY,
			aWgtDarken(border_color));

	// right inner border

	startX = endX - frameThickness;
	startY += frameThickness;
	endY += bgHeight + frameThickness - 1;

	rectfill(tguiBitmap, startX, startY, endX, endY,
			aWgtDarken(border_color));

	// left inner border

	startX = x1 - frameThickness;
	endX = startX + frameThickness;
	startY++;

	rectfill(tguiBitmap, startX, startY, endX, endY,
			aWgtHighlight(border_color));

	// bottom inner border

	startX += frameThickness;
	endX = x2-1;
	startY = endY - frameThickness;
	
	rectfill(tguiBitmap, startX, startY, endX, endY,
			aWgtHighlight(border_color));

	// top left triangle
	
	int tx1 = x + (frameThickness * 2);
	int ty1 = y + frameThickness + frameCaptionHeight + 2;
	int tx2 = tx1;
	int ty2 = ty1 + frameThickness;
	int tx3 = tx1 + frameThickness;
	int ty3 = ty2;

	triangle(tguiBitmap, tx1, ty1, tx2, ty2, tx3, ty3,
			aWgtHighlight(border_color));

	tx1 = x + width - 1 - (frameThickness * 3);
	ty1 = y + height - 1 - (frameThickness * 3) + 1;
	tx2 = tx1;
	ty2 = ty1 + frameThickness;
	tx3 = tx1 + frameThickness;
	ty3 = ty2;
	
	triangle(tguiBitmap, tx1, ty1, tx2, ty2, tx3, ty3,
			aWgtHighlight(border_color));

	aWgtTextout(tguiBitmap, aWgtFont, caption,
			x + (width / 2) -
			(text_length(aWgtFont, caption) / 2),
			y + frameThickness + (frameCaptionHeight / 2) -
			(text_height(aWgtFont) / 2),
			caption_color, captionBorderColor, captionShadowType);

	int color = aWgtDarken(aWgtDarken(border_color));

	x1 = x;
	x2 = x + frameThickness - 1;
	int y1 = y;
	int y2 = y + frameThickness - 1;

	line(tguiBitmap, x1, y1, x2, y2, color);

	x1 = x + width - 1;
	x2 = x1 - frameThickness + 1;

	line(tguiBitmap, x1, y1, x2, y2, color);

	x1 = x;
	x2 = x + frameThickness - 1;
	y1 = y + height - 2;
	y2 = y1 - frameThickness + 2;

	line(tguiBitmap, x1, y1, x2, y2, color);

	x1 = x + width - 1;
	x2 = x1 - frameThickness + 1;

	line(tguiBitmap, x1, y1, x2, y2, color);

	x1 = x + width - 1 - (frameThickness * 2);
	x2 = x1 - frameThickness;
	y1 = y + frameThickness + frameCaptionHeight + 1;
	y2 = y1 + frameThickness;
	
	line(tguiBitmap, x1, y1, x2, y2, color);

	color = aWgtHighlight(aWgtHighlight(border_color));

	x1 = x + (frameThickness * 2);
	x2 = x1 + frameThickness;
	y1 = y + height - 1 - (frameThickness * 2);
	y2 = y1 - frameThickness;

	line(tguiBitmap, x1, y1, x2, y2, color);

	if (closeable) {
		if (aWgtFrameCloseBmp) {
			x1 = x + width - frameThickness - 1 - aWgtFrameCloseBmp->w;
			y1 = y + frameThickness;
			draw_sprite(tguiBitmap, aWgtFrameCloseBmp, x1, y1);
		}
		else {
			x1 = x + width - frameThickness - 1 - frameCaptionHeight;
			y1 = y + frameThickness;
			x2 = x1 + frameCaptionHeight;
			y2 = y1 + frameCaptionHeight;
			line(tguiBitmap, x1, y1, x2, y2, 0);
			line(tguiBitmap, x1, y2, x2, y1, 0);
		}
	}

	if (!captionBar)
		frameCaptionHeight = tmpFrameCaptionHeight;
}

int AWgtFrame::update(int millis)
{
	if (closed) {
		closed = false;
		return TGUI_RETURN;
	}
	if (!dragging)
		return TGUI_CONTINUE;

	int mx = mouse_x;
	int my = mouse_y;

	int dx = mx - lastMouseX;
	int dy = my - lastMouseY;

	if ((dx == 0) && (dy == 0))
		return TGUI_CONTINUE;

	lastMouseX = mx;
	lastMouseY = my;

	tguiTranslateWidget(this, dx, dy);

	return TGUI_CONTINUE;
}

void AWgtFrame::mouseDown()
{
	lastMouseX = mouse_x;
	lastMouseY = mouse_y;

	if (closeable) {
		if (clickOnClose(lastMouseX, lastMouseY)) {
			closeClicked = true;
			return;
		}
	}

	if (moveable)
		dragging = true;
}
	
void AWgtFrame::mouseUp(int x, int y)
{
	if (closeClicked) {
		if (clickOnClose(this->x+x, this->y+y))
			closed = true;
		closeClicked = false;
	}
	else
		dragging = false;
}

AWgtFrame::AWgtFrame(int x, int y, int width, int height,
			char* caption,
			bool closeable, int top_color,
			int bottom_color, int border_color,
			int caption_color, int captionBorderColor,
			int captionShadowType, int alpha,
			bool captionBar, bool moveable, bool smoothBg)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->hotkeys = 0;
	this->caption = caption;
	this->closeable = closeable;
	this->top_color = top_color;
	this->bottom_color = bottom_color;
	this->border_color = border_color;
	this->caption_color = caption_color;
	this->captionBorderColor = captionBorderColor;
	this->captionShadowType = captionShadowType;
	this->alpha = alpha;
	this->captionBar = captionBar;
	this->moveable = moveable;
	this->smoothBg = smoothBg;
	dragging = false;
	closed = false;
	closeClicked = false;
}

AWgtFrame::~AWgtFrame()
{
}

void AWgtFrame::setTopColor(int color)
{
	top_color = color;
}

void AWgtFrame::setBottomColor(int color)
{
	bottom_color = color;
}

void AWgtButton::draw()
{
	int color;

	if (focus) {
		drawing_mode(DRAW_MODE_MASKED_PATTERN, linePatternBmp, 0, 0);
		color = aWgtFocusColor;
	}
	else
		color = border_color;

	line(tguiBitmap, x+2, y, x+width-3, y, color);
	line(tguiBitmap, x, y+2, x, y+height-3, color);
	line(tguiBitmap, x+width-1, y+2, x+width-1, y+height-3, color);
	line(tguiBitmap, x+2, y+height-1, x+width-3, y+height-1, color);

	if (focus)
		drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

	int faceHeight = height - 2;
	int i;

	for (i = 0; i < faceHeight; i++) {
		float ratio = (float)i / (float)faceHeight;
		int color;
		if (depressed)
			color = aWgtInterpolateColor(ratio, bottom_color,
					top_color);
		else
			color = aWgtInterpolateColor(ratio, top_color,
					bottom_color);
		line(tguiBitmap, x+1, y+1+i, x+width-2, y+1+i, color);
	}

	putpixel(tguiBitmap, x+1, y+1, border_color);
	putpixel(tguiBitmap, x+width-2, y+1, border_color);
	putpixel(tguiBitmap, x+width-2, y+height-2, border_color);
	putpixel(tguiBitmap, x+1, y+height-2, border_color);

	int tx = x + (width/2) - (text_length(aWgtFont, text)/2);
	int ty = y + (height/2) - (text_height(aWgtFont)/2);

	if (depressed) {
		tx += 2;
		ty += 2;
		color = aWgtDarken(text_color);
	}
	else
		color = text_color;

	aWgtTextout(tguiBitmap, aWgtFont, text, tx, ty,
			color, shadowColor, shadowType);
}

int AWgtButton::update(int millis)
{
	if (clicked) {
		clicked = false;
		return TGUI_RETURN;
	}

	if (pressed) {
		int mx = mouse_x;
		int my = mouse_y;

		if (pointOnWidget(mx, my))
			depressed = true;
		else
			depressed = false;
	}

	return TGUI_CONTINUE;
}

void AWgtButton::mouseDown()
{
	pressed = true;
	depressed = true;
}
	
void AWgtButton::mouseUp(int x, int y)
{
	pressed = false;
	depressed = false;
	if (x >= 0)
		clicked = true;
}

bool AWgtButton::handleKey(int key)
{
	switch (key >> 8) {
		case KEY_ENTER:
			clicked = true;
			return true;
		case KEY_LEFT:
		case KEY_UP:
			tguiFocusPrevious();
			return true;
		case KEY_RIGHT:
		case KEY_DOWN:
			tguiFocusNext();
			return true;
	}

	return false;
}

bool AWgtButton::acceptsFocus()
{
	return true;
}

AWgtButton::AWgtButton(int x, int y, int width, int height,
			std::vector<int>* hotkeys, char* text,
			int top_color, int bottom_color,
			int border_color, int text_color,
			int shadowColor, int shadowType)
{
	this->x = x;
	this->y = y;
	if (width < 0)
		this->width = text_length(aWgtFont, text) +
			(buttonPadding*2) + 2;
	else
		this->width = width;
	if (height < 0)
		this->height = text_height(aWgtFont) +
			(buttonPadding*2) + 2;
	else
		this->height = height;
	this->hotkeys = hotkeys;
	this->text = text;
	this->top_color = top_color;
	this->bottom_color = bottom_color;
	this->border_color = border_color;
	this->text_color = text_color;
	this->shadowColor = shadowColor;
	this->shadowType = shadowType;
	pressed = false;
	depressed = false;
	clicked = false;
}

AWgtButton::~AWgtButton()
{
}

void AWgtText::draw()
{
	aWgtTextout(tguiBitmap, aWgtFont, text, x, y,
			textColor, shadowColor, shadowType);
}

AWgtText::AWgtText(int x, int y, char* text, int textColor,
		int shadowColor, int shadowType)
{
	this->x = x;
	this->y = y;
	this->width = text_length(aWgtFont, text);
	this->height = text_height(aWgtFont);
	this->text = text;
	this->textColor = textColor;
	this->shadowColor = shadowColor;
	this->shadowType = shadowType;
	this->hotkeys = 0;
}

bool AWgtCheckbox::isChecked()
{
	return checked;
}

void AWgtCheckbox::setChecked(bool checked)
{
	this->checked = checked;
}

void AWgtCheckbox::draw()
{
	int tx;
	int ty;

	if (checkCheckedBmp) {
		if (checked)
			draw_sprite(tguiBitmap, checkCheckedBmp, x, y);
		else
			draw_sprite(tguiBitmap, checkUncheckedBmp, x, y);
		tx = x + checkCheckedBmp->w + checkPadding;
		ty = y + ((checkCheckedBmp->h - text_height(aWgtFont)) / 2);
	}
	else {
		int w = text_height(aWgtFont);
		rect(tguiBitmap, x, y, x+w-1, y+w-1, shadowColor);
		rectfill(tguiBitmap, x+1, y+1, x+w-2, y+w-2, textColor);
		if (checked) {
			line(tguiBitmap, x+1, y+1, x+w-2, y+w-2, shadowColor);
			line(tguiBitmap, x+1, y+w-2, x+w-2, y+1, shadowColor);
		}
		tx = x + w + checkPadding;
		ty = y;
	}

	aWgtTextout(tguiBitmap, aWgtFont, text, tx, ty, textColor, shadowColor,
			shadowType);

	if (focus) {
		drawing_mode(DRAW_MODE_MASKED_PATTERN, linePatternBmp, 0, 0);
		rect(tguiBitmap, tx-2, ty-2, tx+text_length(aWgtFont, text)+2,
				ty+text_height(aWgtFont)+2, aWgtFocusColor);
		drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
	}
}

void AWgtCheckbox::mouseDown()
{
	checked = !checked;
}

bool AWgtCheckbox::handleKey(int key)
{
	switch (key >> 8) {
		case KEY_ENTER:
			checked = !checked;
			tguiMakeDirty(this);
			return true;
		case KEY_UP:
		case KEY_LEFT:
			tguiFocusPrevious();
			return true;
		case KEY_DOWN:
		case KEY_RIGHT:
			tguiFocusNext();
			return true;
	}

	return false;
}

bool AWgtCheckbox::acceptsFocus()
{
	return true;
}
	
AWgtCheckbox::AWgtCheckbox(int x, int y,
			std::vector<int>* hotkeys, char* text,
			int textColor, int shadowColor,
			int shadowType, bool checked)
{
	this->x = x;
	this->y = y;
	if (checkCheckedBmp) {
		this->width = text_length(aWgtFont, text) + checkPadding +
			checkCheckedBmp->w;
		this->height = checkCheckedBmp->h;
	}
	else {
		this->width = text_length(aWgtFont, text) + checkPadding +
			text_height(aWgtFont);
		this->height = text_height(aWgtFont);
	}
	this->hotkeys = hotkeys;
	this->text = text;
	this->textColor = textColor;
	this->shadowColor = shadowColor;
	this->shadowType = shadowType;
	this->checked = checked;
}

AWgtCheckbox::~AWgtCheckbox()
{
}

int AWgtOption::getSelected()
{
	return selected;
}

void AWgtOption::setSelected(int selected)
{
	this->selected = selected;
}

void AWgtOption::draw()
{
	if (leftPressed) {
		if (optionLeftDownBmp) {
			draw_sprite(tguiBitmap, optionLeftDownBmp, x, y);
		}
		else {
			line(tguiBitmap, x, y+(optionDefaultHeight/2),
					x+optionDefaultHeight, y, 0);
			line(tguiBitmap, x, y+(optionDefaultHeight/2),
					x+optionDefaultHeight,
					y+optionDefaultHeight, 0);
		}
	}
	else {
		if (optionLeftBmp) {
			draw_sprite(tguiBitmap, optionLeftBmp, x, y);
		}
		else {
			line(tguiBitmap, x, y+(optionDefaultHeight/2),
					x+optionDefaultHeight, y, 0);
			line(tguiBitmap, x, y+(optionDefaultHeight/2),
					x+optionDefaultHeight,
					y+optionDefaultHeight, 0);
		}
	}

	if (rightPressed) {
		if (optionRightDownBmp) {
			draw_sprite(tguiBitmap, optionRightDownBmp,
					x+width-optionRightBmp->w-1, y);
		}
		else {
			line(tguiBitmap, x+width-1, y+(optionDefaultHeight/2),
					x+width-optionDefaultHeight-1, y, 0);
			line(tguiBitmap, x+width-1, y+(optionDefaultHeight/2),
					x+width-optionDefaultHeight-1,
					y+optionDefaultHeight, 0);
		}
	}
	else {
		if (optionRightBmp) {
			draw_sprite(tguiBitmap, optionRightBmp,
					x+width-optionRightBmp->w-1, y);
		}
		else {
			line(tguiBitmap, x+width-1, y+(optionDefaultHeight/2),
					x+width-optionDefaultHeight-1, y, 0);
			line(tguiBitmap, x+width-1, y+(optionDefaultHeight/2),
					x+width-optionDefaultHeight-1,
					y+optionDefaultHeight, 0);
		}
	}

	int tx = x + (width/2) -
		(text_length(aWgtFont, (*options)[selected]) / 2);
	int ty = y + (height/2) - (text_height(aWgtFont) / 2);

	aWgtTextout(tguiBitmap, aWgtFont, (*options)[selected],
			tx, ty, textColor, shadowColor, shadowType);

	if (focus) {
		drawing_mode(DRAW_MODE_MASKED_PATTERN, linePatternBmp, 0, 0);
		rect(tguiBitmap, tx-2, ty-2,
			tx+text_length(aWgtFont, (*options)[selected])+2,
			ty+text_height(aWgtFont)+2, aWgtFocusColor);
		drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
	}
}

void AWgtOption::mouseDown()
{
	int cx = tguiActiveWidgetClickedPoint.x;

	if (cx <= leftArrowWidth) {
		leftPressed = true;
		if (selected)
			selected--;
	}
	else if (cx >= (width-rightArrowWidth)) {
		rightPressed = true;
		if (selected < (options->size()-1))
			selected++;
	}
}

void AWgtOption::mouseUp(int mx, int my)
{
	leftPressed = false;
	rightPressed = false;
}

bool AWgtOption::handleKey(int key)
{
	switch (key >> 8) {
		case KEY_LEFT:
			if (selected)
				selected--;
			tguiMakeDirty(this);
			return true;
		case KEY_RIGHT:
			if (selected < (options->size()-1))
				selected++;
			tguiMakeDirty(this);
			return true;
		case KEY_UP:
			tguiFocusPrevious();
			return true;
		case KEY_DOWN:
			tguiFocusNext();
			return true;
	}

	return false;
}

bool AWgtOption::acceptsFocus()
{
	return true;
}
	
AWgtOption::AWgtOption(int x, int y, int width,
			std::vector<char*>* options,
			int textColor, int shadowColor,
			int shadowType, int selected)
{
	this->x = x;
	this->y = y;
	this->options = options;
	if (width < 0)
		this->width = findWidth();
	else
		this->width = width;
	if (optionLeftBmp)
		this->leftArrowWidth = optionLeftBmp->w;
	else
		this->leftArrowWidth = optionDefaultHeight;
	if (optionRightBmp)
		this->rightArrowWidth = optionRightBmp->w;
	else
		this->rightArrowWidth = optionDefaultHeight;
	if (optionLeftBmp)
		this->height = optionLeftBmp->h;
	else if (optionRightBmp)
		this->height = optionRightBmp->h;
	else
		this->height = optionDefaultHeight;
	this->textColor = textColor;
	this->shadowColor = shadowColor;
	this->shadowType = shadowType;
	this->selected = selected;
	this->leftPressed = false;
	this->rightPressed = false;
	this->hotkeys = 0;
}

AWgtOption::~AWgtOption()
{
}

int AWgtOption::findWidth()
{
	int w = optionPadding * 2;

	if (optionLeftBmp)
		w += optionLeftBmp->w;
	else
		w += optionDefaultHeight;
	
	if (optionRightBmp)
		w += optionLeftBmp->w;
	else
		w += optionDefaultHeight;

	int max = 0;

	for (unsigned int i = 0; i < options->size(); i++) {
		if (text_length(aWgtFont, (*options)[i]) > max) {
			max = text_length(aWgtFont, (*options)[i]);
		}
	}

	w += max;

	return w;
}

int AWgtSlider::getStop()
{
	return currStop;
}

void AWgtSlider::setStop(int stop)
{
	currStop = stop;
}

int AWgtSlider::getStopPos(int stop)
{
	if (stop == 0)
		return x + (tabWidth() / 2);
	else if (stop >= (numStops-1))
		return x+width-(tabWidth() / 2);
	else {
		return x + (width / (numStops - 1) * stop);
	}
}

int AWgtSlider::tabWidth()
{
	if (sliderTabBmp)
		return sliderTabBmp->w;
	else
		return sliderDefaultTabWidth;
}

int AWgtSlider::tabHeight()
{
	if (sliderTabBmp)
		return sliderTabBmp->h;
	else
		return sliderDefaultTabHeight;
}

void AWgtSlider::draw()
{
	int ly = y + (tabHeight() / 2) - lineThickness;

	rectfill(tguiBitmap, x, ly, x+width-1, ly+lineThickness, lineColor);
	rectfill(tguiBitmap, x, ly+lineThickness, x+width-1,
			ly+(lineThickness*2), shadowColor);

	if (focus) {
		drawing_mode(DRAW_MODE_MASKED_PATTERN, linePatternBmp, 0, 0);
		rect(tguiBitmap, x-1, ly-1, x+width, ly+(lineThickness*2)+1,
				aWgtFocusColor);
		drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
	}

	int tx = getStopPos(currStop) - (tabWidth() / 2);

	if (sliderTabBmp) {
		draw_sprite(tguiBitmap, sliderTabBmp, tx, y);
	}
	else {
		rect(tguiBitmap, tx, y, tx+sliderDefaultTabWidth-1,
				y+sliderDefaultTabHeight-1, shadowColor);
		rectfill(tguiBitmap, tx+1, y+1, tx+sliderDefaultTabWidth-2,
				y+sliderDefaultTabHeight-2, lineColor);
	}
}

void AWgtSlider::mouseDown()
{
	int mx = tguiActiveWidgetClickedPoint.x;

	int tcx = getStopPos(currStop) - x; // center of tab;
	int tlx = tcx - (tabWidth() / 2); // left of tab
	int trx = tcx + (tabWidth() / 2); // right of tab

	if (mx < tlx) {
		if (currStop)
			currStop--;
	}
	else if (mx > trx) {
		if (currStop < (numStops-1))
			currStop++;
	}
	else {
		tabClicked = true;
		clickX = mouse_x;
	}
}

void AWgtSlider::mouseUp(int mx, int my)
{
	tabClicked = false;
}

bool AWgtSlider::handleKey(int key)
{
	switch (key >> 8) {
		case KEY_LEFT:
			if (currStop)
				currStop--;
			tguiMakeDirty(this);
			return true;
		case KEY_RIGHT:
			if (currStop < (numStops-1))
				currStop++;
			tguiMakeDirty(this);
			return true;
		case KEY_UP:
			tguiFocusPrevious();
			return true;
		case KEY_DOWN:
			tguiFocusNext();
			return true;
	}

	return false;
}

bool AWgtSlider::acceptsFocus()
{
	return true;
}

int AWgtSlider::update(int step)
{
	if (!tabClicked)
		return TGUI_CONTINUE;

	tguiUpdateDirtyRectangle(x, y, x+width, y+height);

	int mx = mouse_x;
	int relativeX = mx - x;

	if (mx < clickX) {
		for (int stop = 0; stop < numStops; stop++) {
			int pos = getStopPos(stop) - x;
			if (pos > relativeX) {
				currStop = stop;
				break;
			}
		}
	}
	else if (mx > clickX) {
		for (int stop = numStops-1; stop >= 0; stop--) {
			int pos = getStopPos(stop) - x;
			if (pos < relativeX) {
				currStop = stop;
				break;
			}
		}
	}

	return TGUI_CONTINUE;
}

AWgtSlider::AWgtSlider(int x, int y, int width, int numStops, int currStop,
		int lineColor, int shadowColor, int lineThickness)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = tabHeight();
	this->numStops = MIN(width, numStops);
	this->currStop = MIN(numStops, currStop);
	this->tabClicked = false;
	this->lineColor = lineColor;
	this->shadowColor = shadowColor;
	this->lineThickness = lineThickness;
	this->hotkeys = 0;
}

AWgtSlider::~AWgtSlider()
{
}

void AWgtIcon::draw()
{
	if (pressable && depressed)
		draw_sprite(tguiBitmap, downBmp, x, y);
	else if (pressable && tguiPointOnWidget(this, mouse_x, mouse_y))
		draw_sprite(tguiBitmap, hoverBmp, x, y);
	else
		draw_sprite(tguiBitmap, normalBmp, x, y);

	if (focus) {
		drawing_mode(DRAW_MODE_MASKED_PATTERN, linePatternBmp, 0, 0);
		rect(tguiBitmap, x, y, x+width-1, y+height-1, aWgtFocusColor);
		drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
	}
}

int AWgtIcon::update(int millis)
{
	if (clicked) {
		clicked = false;
		return TGUI_RETURN;
	}

	if (pressed) {
		int mx = mouse_x;
		int my = mouse_y;

		if (pointOnWidget(mx, my))
			depressed = true;
		else
			depressed = false;
	}

	return TGUI_CONTINUE;
}

void AWgtIcon::mouseDown()
{
	pressed = true;
	depressed = true;
}
	
void AWgtIcon::mouseUp(int x, int y)
{
	pressed = false;
	depressed = false;
	if (x >= 0)
		clicked = true;
}

bool AWgtIcon::handleKey(int key)
{
	switch (key >> 8) {
		case KEY_ENTER:
			clicked = true;
			return true;
		case KEY_LEFT:
		case KEY_UP:
			tguiFocusPrevious();
			return true;
		case KEY_RIGHT:
		case KEY_DOWN:
			tguiFocusNext();
			return true;
	}

	return false;
}

bool AWgtIcon::acceptsFocus()
{
	return true;
}

bool AWgtIcon::acceptsHover()
{
	return true;
}

AWgtIcon::AWgtIcon(int x, int y, bool pressable,
		BITMAP* normalBmp, BITMAP* hoverBmp, BITMAP* downBmp,
		std::vector<int>* hotkeys)
{
	this->x = x;
	this->y = y;
	this->width = normalBmp->w;
	this->height = normalBmp->h;
	this->pressable = pressable;
	this->normalBmp = normalBmp;
	this->hoverBmp = hoverBmp;
	this->downBmp = downBmp;
	this->hotkeys = hotkeys;
	pressed = false;
	depressed = false;
	clicked = false;
}

AWgtIcon::~AWgtIcon()
{
}
