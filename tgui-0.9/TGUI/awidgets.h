#ifndef AWIDGETS_H
#define AWIDGETS_H

const int AWGT_TEXT_NORMAL = 0; // no shadow
const int AWGT_TEXT_DROP_SHADOW = 1; // shadow down and to the right
const int AWGT_TEXT_BORDER = 2; // border all around text
const int AWGT_TEXT_SQUARE_BORDER = 3; // like BORDER but on corners too

/* Properties */
const int AWGT_FONT = 0;
const int AWGT_FRAME_CLOSE_BMP = 1;
const int AWGT_CHECKBOX_CHECKED_BMP = 2;
const int AWGT_CHECKBOX_UNCHECKED_BMP = 3;
const int AWGT_OPTION_LEFT_BMP = 4;
const int AWGT_OPTION_RIGHT_BMP = 5;
const int AWGT_OPTION_LEFT_DOWN_BMP = 6;
const int AWGT_OPTION_RIGHT_DOWN_BMP = 7;
const int AWGT_SLIDER_TAB_BMP = 8;
const int AWGT_FOCUS_COLOR = 9;

extern void aWgtInit();
extern int aWgtInterpolateColor(float ratio, int color1, int color2);
extern int aWgtDarken(int color);
extern int aWgtHighlight(int color);
extern void aWgtSet(int property, void* value);
extern void aWgtTextout(BITMAP* bmp, FONT* font, char* text, int x, int y,
		int textColor, int shadowColor, int shadowType);

class AllegroWidget : public TGUIWidget {
public:
	bool pointOnWidget(int x, int y);
};

class AWgtFrame : public AllegroWidget {
public:
	void draw();
	int update(int millis);
	void mouseDown();
	void mouseUp(int x, int y);
	AWgtFrame(int x, int y, int width, int height,
			char* caption,
			bool closeable, int top_color,
			int bottom_color, int border_color,
			int caption_color, int captionBorderColor,
			int captionShadowType, int alpha,
			bool captionBar, bool moveable,
			bool smoothBg);
	~AWgtFrame();

	void setTopColor(int color);
	void setBottomColor(int color);
protected:
	virtual bool clickOnClose(int mx, int my);
	char* caption;
	bool closeable;
	int top_color;
	int bottom_color;
	int border_color;
	int caption_color;
	int captionBorderColor;
	int captionShadowType;
	int shadowType;
	int alpha;
	bool captionBar;
	bool moveable;
	bool smoothBg;
	bool dragging;
	int lastMouseX;
	int lastMouseY;
	bool closed;
	bool closeClicked;
};

class AWgtButton : public AllegroWidget {
public:
	void draw();
	int update(int millis);
	void mouseDown();
	void mouseUp(int x, int y);
	bool handleKey(int key);
	bool acceptsFocus();
	AWgtButton(int x, int y, int width, int height,
			std::vector<int>* hotkeys, char* text,
			int top_color, int bottom_color,
			int border_color, int text_color,
			int shadowColor, int shadowType);
	~AWgtButton();
protected:
	char* text;
	int top_color;
	int bottom_color;
	int border_color;
	int text_color;
	int shadowColor;
	int shadowType;
	bool pressed; // mouseDown happened
	bool depressed; // currently down
	bool clicked; // pressed and released on button
};

class AWgtText : public AllegroWidget {
public:
	void draw();
	AWgtText(int x, int y, char* text, int textColor,
			int shadowColor, int shadowType);
protected:
	char* text;
	int textColor;
	int shadowColor;
	int shadowType;
};

class AWgtCheckbox : public AllegroWidget {
public:
	bool isChecked();
	void setChecked(bool checked);
	void draw();
	void mouseDown();
	bool handleKey(int key);
	bool acceptsFocus();
	AWgtCheckbox(int x, int y,
			std::vector<int>* hotkeys, char* text,
			int textColor, int shadowColor,
			int shadowType, bool checked);
	~AWgtCheckbox();
protected:
	char* text;
	int textColor;
	int shadowColor;
	int shadowType;
	bool checked;
};

class AWgtOption : public AllegroWidget {
public:
	int getSelected();
	void setSelected(int selected);
	void draw();
	void mouseDown();
	void mouseUp(int mx, int my);
	bool handleKey(int key);
	bool acceptsFocus();
	AWgtOption(int x, int y, int width, std::vector<char*>* options,
		int textColor, int shadowColor, int shadowType, int selected);
	~AWgtOption();
protected:
	int findWidth();
	std::vector<char*>* options;
	int textColor;
	int shadowColor;
	int shadowType;
	int selected;
	int leftArrowWidth;
	int rightArrowWidth;
	bool leftPressed;
	bool rightPressed;
};

class AWgtSlider : public AllegroWidget {
public:
	int getStop();
	void setStop(int stop);
	void draw();
	void mouseDown();
	void mouseUp(int mx, int my);
	bool handleKey(int key);
	bool acceptsFocus();
	int update(int step);
	AWgtSlider(int x, int y, int width, int numStops, int currStop,
			int lineColor, int shadowColor, int lineThickness);
	~AWgtSlider();
protected:
	int getStopPos(int stop);
	int tabWidth();
	int tabHeight();
	int numStops;
	int currStop;
	bool tabClicked;
	int clickX;
	int lineColor;
	int shadowColor;
	int lineThickness;
};

class AWgtIcon : public AllegroWidget {
public:
	void draw();
	int update(int millis);
	void mouseDown();
	void mouseUp(int x, int y);
	bool handleKey(int key);
	bool acceptsFocus();
	bool acceptsHover();
	AWgtIcon(int x, int y, bool pressable, BITMAP* normalBmp,
			BITMAP* hoverBmp, BITMAP* downBmp,
			std::vector<int>* hotkeys);
	~AWgtIcon();
protected:
	bool pressable;
	BITMAP* normalBmp;
	BITMAP* hoverBmp;
	BITMAP* downBmp;
	bool pressed; // mouseDown happened
	bool depressed; // currently down
	bool clicked; // pressed and released on button
};

#endif
