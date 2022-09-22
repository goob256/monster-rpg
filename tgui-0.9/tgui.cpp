#include <vector>

#include <allegro.h>

#if defined __linux__ || defined ALLEGRO_MACOSX
#include <sys/time.h>
#else
#include <winalleg.h>
#endif

#include "TGUI/tgui.h"

struct TGUI {
	int focus;
	std::vector<TGUIWidget*> widgets;
};

static std::vector<TGUI*> tguiStack;
static TGUI* activeGUI = 0;
static long tguiLastUpdate = tguiCurrentTimeMillis();
static bool tguiMouseReleased = true;
static TGUIWidget* tguiCurrentParent = 0;
static TGUIWidget* tguiClickedWidget = 0;

static std::vector<int> keyBuffer;

/*
 * The widget that was clicked on (or 0) and where it was clicked.
 */
TGUIWidget* tguiActiveWidget = 0;
TGUIPoint tguiActiveWidgetClickedPoint; // relative to widget
int tguiMouseButton;
TGUIPoint tguiMouseReleasePoint;
BITMAP* tguiBitmap = 0;
TGUIRect tguiDirtyRectangle = { -1, };

static TGUIWidget* tguiHoverWidget = 0;

static void tguiFindFocus()
{
	for (int i = 0; i < activeGUI->widgets.size(); i++) {
		if (activeGUI->widgets[i]->getFocus()) {
			activeGUI->focus = i;
			return;
		}
	}
}

/*
 * Returns true if the shift keys are pressed, otherwise false.
 */
static bool tguiShiftsPressed(int shifts)
{
	int keysToCheck[6] = { 0, };
	int numKeysToCheck = 0;

	if (shifts & TGUI_KEYFLAG_SHIFT) {
		keysToCheck[numKeysToCheck++] = KEY_LSHIFT;
		keysToCheck[numKeysToCheck++] = KEY_RSHIFT;
	}
	else {
		if (shifts & TGUI_KEYFLAG_LSHIFT) {
			keysToCheck[numKeysToCheck++] = KEY_LSHIFT;
		}
		if (shifts & TGUI_KEYFLAG_RSHIFT) {
			keysToCheck[numKeysToCheck++] = KEY_RSHIFT;
		}
	}
	if (shifts & TGUI_KEYFLAG_CONTROL) {
		keysToCheck[numKeysToCheck++] = KEY_LCONTROL;
		keysToCheck[numKeysToCheck++] = KEY_RCONTROL;
	}
	else {
		if (shifts & TGUI_KEYFLAG_LCONTROL) {
			keysToCheck[numKeysToCheck++] = KEY_LCONTROL;
		}
		if (shifts & TGUI_KEYFLAG_RCONTROL) {
			keysToCheck[numKeysToCheck++] = KEY_RCONTROL;
		}
	}
	if (shifts & TGUI_KEYFLAG_ALT) {
		keysToCheck[numKeysToCheck++] = KEY_ALT;
		keysToCheck[numKeysToCheck++] = KEY_ALTGR;
	}
	else {
		if (shifts & TGUI_KEYFLAG_LALT) {
			keysToCheck[numKeysToCheck++] = KEY_ALT;
		}
		if (shifts & TGUI_KEYFLAG_RALT) {
			keysToCheck[numKeysToCheck++] = KEY_ALTGR;
		}
	}

	for (int i = 0; i < numKeysToCheck; i++) {
		if (!key[keysToCheck[i]])
			return false;
	}

	return true;
}

/*
 * Returns true if a hotkey is pressed, otherwise false.
 */
static bool tguiHotkeyPressed(int hotkey)
{
	int shifts = (hotkey >> 16) & 0xffff;
	int k = hotkey & 0xffff;

	if (tguiShiftsPressed(shifts) && key[k])
		return true;
	return false;
}

/*
 * Returns the widget that the pixel x, y fall on,
 * or 0 if none is found.
 */
static TGUIWidget* tguiFindPixelOwner(int x, int y)
{
	for (int i = activeGUI->widgets.size()-1; i >= 0; i--) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (tguiPointOnWidget(widget, x, y))
			return widget;
	}
	return 0;
}

/*
 * Update the dirty rectangle to include a rectangle.
 */
void tguiUpdateDirtyRectangle(int x1, int y1, int x2, int y2)
{
	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		int wx1 = widget->getX();
		int wy1 = widget->getY();
		int wx2 = widget->getX() + widget->getWidth() - 1;
		int wy2 = widget->getY() + widget->getHeight() - 1;
		if (y1 <= wy2 && wy1 <= y2 && x1 <= wx2 && wx1 <= x2) {
			x1 = MIN(x1, wx1);
			x2 = MAX(x2, wx2);
			y1 = MIN(y1, wy1);
			y2 = MAX(y2, wy2);
		}
	}
	if (tguiDirtyRectangle.x1 < 0) {
		tguiDirtyRectangle.x1 = MAX(0, x1);
		tguiDirtyRectangle.y1 = MAX(0, y1);
		tguiDirtyRectangle.x2 = x2;
		tguiDirtyRectangle.y2 = y2;
	}
	else {
		tguiDirtyRectangle.x1 = MAX(0, MIN(x1, tguiDirtyRectangle.x1));
		tguiDirtyRectangle.y1 = MAX(0, MIN(y1, tguiDirtyRectangle.y1));
		tguiDirtyRectangle.x2 = MAX(x2, tguiDirtyRectangle.x2);
		tguiDirtyRectangle.y2 = MAX(y2, tguiDirtyRectangle.y2);
	}
}

/*
 * Make a widget dirty (to be redrawn)
 */
void tguiMakeDirty(TGUIWidget* widget)
{
	int x1 = widget->getX();
	int y1 = widget->getY();
	int x2 = x1 + widget->getWidth();
	int y2 = y1 + widget->getHeight();
	tguiUpdateDirtyRectangle(x1, y1, x2, y2);
}

/*
 * Returns true if the point x, y falls on a particular widget.
 */
bool tguiPointOnWidget(TGUIWidget* widget, int x, int y)
{
	if ((x > widget->getX()) && (x < (widget->getX() + widget->getWidth())) &&
			(y > widget->getY()) &&
			(y < (widget->getY() + widget->getHeight())))
		return true;
	return false;
}

/*
 * Redraws the GUI and clears the dirty rectangle.
 */
void tguiClearDirtyRectangle()
{
	tguiDrawRect(tguiDirtyRectangle.x1, tguiDirtyRectangle.y1,
		tguiDirtyRectangle.x2, tguiDirtyRectangle.y2);
	tguiDirtyRectangle.x1 = -1;
}

long tguiCurrentTimeMillis()
{
#if defined __linux__ || defined ALLEGRO_MACOSX
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#else
	return timeGetTime();
#endif
}

/*
 * Delete the active widget set and allocates a blank one.
 * Must be called before any other GUI functions are called.
 */
void tguiInit(BITMAP* dest)
{
	tguiBitmap = dest;

	if (activeGUI) {
		tguiDeleteActive();
	}

	activeGUI = new TGUI;
	activeGUI->focus = 0;

	tguiUpdateDirtyRectangle(0, 0, dest->w, dest->h);

	tguiLastUpdate = tguiCurrentTimeMillis();
}

/*
 * Clean up. You should call tguiDeleteActive first if there
 * are any widgets you haven't cleared up yet.
 */
void tguiShutdown()
{
	delete activeGUI;
	tguiStack.clear();
}

/*
 * Set the widget that has the keyboard focus.
 */
void tguiSetFocus(int widget)
{
	if (activeGUI->widgets.size() <= 0)
		return;
	tguiMakeDirty(activeGUI->widgets[activeGUI->focus]);
	activeGUI->widgets[activeGUI->focus]->setFocus(false);
	activeGUI->widgets[widget]->setFocus(true);
	tguiActiveWidget = activeGUI->widgets[widget];
	tguiMakeDirty(tguiActiveWidget);
	activeGUI->focus = widget;
}

/*
 * Set the widget that has the keyboard focus.
 */
void tguiSetFocus(TGUIWidget* widget)
{
	if (tguiActiveWidget) {
		tguiUpdateDirtyRectangle(tguiActiveWidget->getX(),
				tguiActiveWidget->getY(),
				tguiActiveWidget->getX()+tguiActiveWidget->getWidth(),
				tguiActiveWidget->getY()+tguiActiveWidget->getHeight());
		tguiActiveWidget->setFocus(false);
		tguiMakeDirty(tguiActiveWidget);
	}

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* w = activeGUI->widgets[i];
		if (w == widget) {
			w->setFocus(true);
			activeGUI->focus = i;
			tguiActiveWidget = widget;
			tguiMakeDirty(tguiActiveWidget);
			return;
		}
	}
}

TGUIWidget* tguiGetFocus()
{
	return tguiActiveWidget;
}

/*
 * Move the focus to the first widget before this currently
 * focused widget that accepts focus.
 */
void tguiFocusPrevious(void)
{
	int focus = activeGUI->focus-1;

	if (focus >= 0) {
		while (focus >= 0) {
			if (activeGUI->widgets[focus]->acceptsFocus()) {
				if (activeGUI->widgets[focus]->getParent() ==
						activeGUI->widgets[activeGUI->focus]->getParent()) {
					tguiMakeDirty(activeGUI->widgets[activeGUI->focus]);
					activeGUI->widgets[activeGUI->focus]->setFocus(false);
					activeGUI->widgets[focus]->setFocus(true);
					activeGUI->focus = focus;
					tguiActiveWidget = activeGUI->widgets[focus];
					tguiMakeDirty(tguiActiveWidget);
					break;
				}
			}
			focus--;
		}
	}
}

/*
 * Move the focus to the next widget that accepts focus.
 */
void tguiFocusNext(void)
{
	int focus = activeGUI->focus+1;

	while (focus < activeGUI->widgets.size()) {
		if (activeGUI->widgets[focus]->acceptsFocus()) {
			if (activeGUI->widgets[focus]->getParent() ==
					activeGUI->widgets[activeGUI->focus]->getParent()) {
				tguiMakeDirty(activeGUI->widgets[activeGUI->focus]);
				activeGUI->widgets[activeGUI->focus]->setFocus(false);
				activeGUI->widgets[focus]->setFocus(true);
				activeGUI->focus = focus;
				tguiActiveWidget = activeGUI->widgets[focus];
				tguiMakeDirty(tguiActiveWidget);
				break;
			}
		}
		focus++;
	}
}

/*
 * Delete the active set of widgets. You must call tguiInit or
 * tguiPop after this if you are going to use the GUI again.
 */
void tguiDeleteActive()
{
	activeGUI->widgets.clear();
	delete activeGUI;
	activeGUI = 0;
	tguiActiveWidget = 0;
	tguiCurrentParent = 0;
	tguiHoverWidget = 0;
}

/*
 * Move the entire GUI x pixels on the x axis and y on the y axis,
 * with positive x values moving towards the right and positive
 * y values moving down on the screen.
 */
void tguiTranslateAll(int x, int y)
{
	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* w = activeGUI->widgets[i];
		w->setX(w->getX() + x);
		w->setY(w->getY() + y);
		int minx;
		int maxx;
		int miny;
		int maxy;
		if (x < 0) {
			minx = w->getX() + x;
			maxx = w->getX() + w->getWidth() - 1;
		}
		else {
			minx = w->getX();
			maxx = w->getX() + x + w->getWidth() - 1;
		}
		if (y < 0) {
			miny = w->getY() + y;
			maxy = w->getY() + w->getHeight() - 1;
		}
		else {
			miny = w->getY();
			maxy = w->getY() + y + w->getHeight() - 1;
		}
		tguiUpdateDirtyRectangle(minx, miny, maxx, maxy);
	}
}

/*
 * Like tguiTranslateAll, but only translates a widget and its
 * children.
 */
void tguiTranslateWidget(TGUIWidget* parent, int x, int y)
{
	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* w = activeGUI->widgets[i];
		if ((parent == w) || (w->getParent() == parent)) {
			int newx;
			int newy;
			if (w->getParent() == parent) {
				newx = w->getParent()->getX() + w->getRelativeX();
				newy = w->getParent()->getY() + w->getRelativeY();
			}
			else if (w->getParent()) {
				if (x < 0)
					newx = MAX(w->getParent()->getX(), w->getX()+x);
				else
					newx = MIN(w->getParent()->getX()+w->getParent()->getWidth()-
							w->getWidth(),
							w->getX()+x);
				if (y < 0)
					newy = MAX(w->getParent()->getY(), w->getY()+y);
				else
					newy = MIN(w->getParent()->getY()+
							w->getParent()->getHeight()-
							w->getHeight(),
							w->getY()+y);
			}
			else {
				newx = w->getX() + x;
				if (newx < (0 - w->getWidth() + TGUI_BORDER))
					newx = 0 - w->getWidth() + TGUI_BORDER;
				else if (newx > (tguiBitmap->w - TGUI_BORDER))
					newx = tguiBitmap->w - TGUI_BORDER;
				newy = w->getY() + y;
				if (newy < (0 - w->getHeight() + TGUI_BORDER))
					newy = 0 - w->getHeight() + TGUI_BORDER;
				else if (newy > (tguiBitmap->h - TGUI_BORDER))
					newy = tguiBitmap->h - TGUI_BORDER;
			}
			/* dirty rectangle coordinates */
			int x1;
			int y1;
			int x2;
			int y2;
			if (newx < w->getX()) {
				x1 = newx;
				x2 = w->getX() + w->getWidth();
			}
			else {
				x1 = w->getX();
				x2 = newx + w->getWidth();
			}
			if (newy < w->getY()) {
				y1 = newy;
				y2 = w->getY() + w->getHeight();
			}
			else {
				y1 = w->getY();
				y2 = newy + w->getHeight();
			}
			tguiUpdateDirtyRectangle(x1, y1, x2, y2);
			w->setX(newx);
			w->setY(newy);
		}
	}
}

/*
 * Add a widget to the GUI.
 */
void tguiAddWidget(TGUIWidget* widget)
{
	widget->setParent(tguiCurrentParent);
	if (tguiCurrentParent) {
		widget->setRelativeX(widget->getX() - tguiCurrentParent->getX());
		widget->setRelativeY(widget->getY() - tguiCurrentParent->getY());
	}
	widget->setFocus(false);
	activeGUI->widgets.push_back(widget);
}

/*
 * Update the GUI.
 * Passes all input to the relevant widgets.
 * Must be called frequently after a GUI is created.
 * Returns 0 normally, or if a widget requests,
 * it returns that widget. It's up to the caller to
 * query the widget as to why it wanted to return.
 */
TGUIWidget* tguiUpdate()
{
	long currTime = tguiCurrentTimeMillis();
	long elapsed = currTime - tguiLastUpdate;
	tguiLastUpdate = currTime;

	tguiClearDirtyRectangle();

	if (tguiActiveWidget) {
		int key;
		while (keypressed()) {
			key = readkey();
			keyBuffer.push_back(key);
		}
		for (unsigned int i = 0; i < keyBuffer.size(); i++) {
			key = keyBuffer[i];
			if (!tguiActiveWidget->handleKey(key)) {
				simulate_keypress(key);
			}
		}
		keyBuffer.clear();
	}

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		unsigned int retVal = widget->update(elapsed);
		switch (retVal) {
			case TGUI_CONTINUE:
				break;
			case TGUI_RETURN:
				return widget;
		}
		std::vector<int>* hotkeys = widget->getHotkeys();
		if (hotkeys) {
			for (unsigned int j = 0; j < hotkeys->size(); j++) {
				int hotkey = (*hotkeys)[j];
				if (tguiHotkeyPressed(hotkey))
					widget->handleHotkey(hotkey);
			}
		}
	}

	if (tguiMouseReleased) {
		int saved_mouse_b = mouse_b;
		int saved_mouse_x = mouse_x;
		int saved_mouse_y = mouse_y;
		if (saved_mouse_b) {
			TGUIWidget* widget =
				tguiFindPixelOwner(saved_mouse_x, saved_mouse_y);
			if (widget) {
				tguiSetFocus(widget);
				tguiActiveWidgetClickedPoint.x =
					saved_mouse_x - widget->getX();
				tguiActiveWidgetClickedPoint.y =
					saved_mouse_y - widget->getY();
				tguiMouseButton = saved_mouse_b;
				if (!widget->getParent())
					tguiRaiseWidget(widget);
				widget->mouseDown();
				tguiClickedWidget = widget;
			}
			tguiMouseReleased = false;
		}
	}
	else {
		int saved_mouse_b = mouse_b;
		int saved_mouse_x = mouse_x;
		int saved_mouse_y = mouse_y;
		if (!saved_mouse_b) {
			if (tguiClickedWidget) {
				tguiMouseReleasePoint.x = saved_mouse_x;
				tguiMouseReleasePoint.y = saved_mouse_y;
				int relativeX;
				int relativeY;
				if (tguiPointOnWidget(tguiActiveWidget, saved_mouse_x,
							saved_mouse_y)) {
					relativeX = saved_mouse_x - tguiActiveWidget->getX();
					relativeY = saved_mouse_y - tguiActiveWidget->getY();
				}
				else {
					relativeX = -1;
					relativeY = -1;
				}
				tguiActiveWidget->mouseUp(relativeX, relativeY);
				tguiClickedWidget = 0;
				int x1 = tguiActiveWidget->getX();
				int y1 = tguiActiveWidget->getY();
				int x2 = tguiActiveWidget->getX()+tguiActiveWidget->getWidth();
				int y2 = tguiActiveWidget->getY()+tguiActiveWidget->getHeight();
				tguiUpdateDirtyRectangle(x1, y1, x2, y2);
			}
			tguiMouseReleased = true;
		}
	}

	TGUIWidget* hoverWidget = tguiFindPixelOwner(mouse_x, mouse_y);
	if (hoverWidget) {
		if (tguiHoverWidget && hoverWidget != tguiHoverWidget) {
			tguiHoverWidget->draw();
			tguiMakeDirty(tguiHoverWidget);
		}
		if (hoverWidget->acceptsHover()) {
			hoverWidget->draw();
			tguiMakeDirty(hoverWidget);
		}
		tguiHoverWidget = hoverWidget;
	}

	return 0;
}

/*
 * Must be called if you stop making calls to tguiUpdate
 * for some time.
 */
void tguiWakeUp()
{
	tguiLastUpdate = tguiCurrentTimeMillis();
}

/*
 * Draw all widgets.
 */
void tguiDraw()
{
	tguiDrawRect(0, 0, SCREEN_W, SCREEN_H);
}

/*
 * Draw every widget that falls into a rectangle.
 */
void tguiDrawRect(int x1, int y1, int x2, int y2)
{
	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		int wx1 = widget->getX();
		int wy1 = widget->getY();
		int wx2 = widget->getX() + widget->getWidth() - 1;
		int wy2 = widget->getY() + widget->getHeight() - 1;
		if (y1 <= wy2 && wy1 <= y2 && x1 <= wx2 && wx1 <= x2) {
			widget->draw();
		}
	}
}

/*
 * Push the current GUI on a stack.
 * You must call tguiInit after this and later tguiPop.
 */
void tguiPush()
{
	tguiStack.push_back(activeGUI);
	activeGUI = new TGUI;
	activeGUI->focus = 0;
	tguiActiveWidget = 0;
	tguiUpdateDirtyRectangle(0, 0, tguiBitmap->w, tguiBitmap->h);
}

/*
 * Make the last GUI pushed onto the stack with tguiPush
 * active and remove it from the stack. You should have
 * deleted the active GUI with tguiDeleteActive before
 * calling this if you created a new GUI after calling
 * tguiPush.
 */
bool tguiPop()
{
	if (tguiStack.size() <= 0)
		return false;
	delete activeGUI;
	activeGUI = tguiStack[tguiStack.size()-1];
	std::vector<TGUI*>::iterator it = tguiStack.begin() + tguiStack.size() - 1;
	tguiStack.erase(it);
	tguiSetFocus(activeGUI->focus);
	return true;
}

/*
 * Create a hotkey that can contain keyshift flags as well
 * as regular keys, that can be used by the GUI widgets.
 */
int tguiCreateHotkey(int flags, int key)
{
	return ((flags & 0xffff) << 16) & (key & 0xffff);
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiRaiseChildren(TGUIWidget* parent)
{
	std::vector<TGUIWidget*> targetWidgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget->getParent() == parent) {
			targetWidgets.push_back(widget);
		}
	}

	int currIndex = 0;
	unsigned int i;

	for (i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget->getParent() != parent)
			activeGUI->widgets[currIndex++] = widget;
	}

	for (i = 0; i < targetWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = targetWidgets[i];
	}
	
	targetWidgets.clear();

	tguiFindFocus();
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiLowerChildren(TGUIWidget* parent)
{
	std::vector<TGUIWidget*> lowerWidgets;
	std::vector<TGUIWidget*> upperWidgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget->getParent() == parent) {
			lowerWidgets.push_back(widget);
		}
		else {
			upperWidgets.push_back(widget);
		}
	}

	int currIndex = 0;
	unsigned int i;

	for (i = 0; i < lowerWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = lowerWidgets[i];
	}
	
	for (i = 0; i < upperWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = upperWidgets[i];
	}
	
	lowerWidgets.clear();
	upperWidgets.clear();

	tguiFindFocus();
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiRaiseSingleWidget(TGUIWidget* widgetToRaise)
{
	std::vector<TGUIWidget*> targetWidgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget == widgetToRaise) {
			targetWidgets.push_back(widget);
		}
	}

	int currIndex = 0;
	unsigned int i;

	for (i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget != widgetToRaise) {
			activeGUI->widgets[currIndex++] = widget;
		}
	}

	for (i = 0; i < targetWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = targetWidgets[i];
	}
	
	targetWidgets.clear();

	tguiFindFocus();
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiLowerSingleWidget(TGUIWidget* widgetToLower)
{
	std::vector<TGUIWidget*> lowerWidgets;
	std::vector<TGUIWidget*> upperWidgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget == widgetToLower) {
			lowerWidgets.push_back(widget);
		}
		else {
			upperWidgets.push_back(widget);
		}
	}

	int currIndex = 0;
	unsigned int i;

	for (i = 0; i < lowerWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = lowerWidgets[i];
	}
	
	for (i = 0; i < upperWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = upperWidgets[i];
	}
	
	lowerWidgets.clear();
	upperWidgets.clear();

	tguiFindFocus();
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiLowerSingleWidgetBelow(TGUIWidget* widgetToLower,
		TGUIWidget* widgetAbove)
{
	std::vector<TGUIWidget*> widgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if (widget == widgetAbove) {
			widgets.push_back(widgetToLower);
			widgets.push_back(widgetAbove);
		}
		else if (widget != widgetToLower) {
			widgets.push_back(widget);
		}
	}

	activeGUI->widgets.clear();

	for (int i = 0; i < widgets.size(); i++) {
		activeGUI->widgets.push_back(widgets[i]);
	}

	widgets.clear();

	tguiFindFocus();
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiRaiseWidget(TGUIWidget* widgetToRaise)
{
	std::vector<TGUIWidget*> targetWidgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if ((widget == widgetToRaise) ||
				(widget->getParent() == widgetToRaise)) {
			targetWidgets.push_back(widget);
		}
	}

	int currIndex = 0;
	unsigned int i;

	for (i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if ((widget != widgetToRaise) &&
				(widget->getParent() != widgetToRaise))
			activeGUI->widgets[currIndex++] = widget;
	}

	for (i = 0; i < targetWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = targetWidgets[i];
	}
	
	targetWidgets.clear();

	tguiFindFocus();
}

/*
 * Move all the widgets in a group forward to the front
 * of the display (end of vector).
 */
void tguiLowerWidget(TGUIWidget* widgetToLower)
{
	std::vector<TGUIWidget*> lowerWidgets;
	std::vector<TGUIWidget*> upperWidgets;

	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if ((widget == widgetToLower) ||
				(widget->getParent() == widgetToLower)) {
			lowerWidgets.push_back(widget);
		}
		else {
			upperWidgets.push_back(widget);
		}
	}

	int currIndex = 0;
	unsigned int i;

	for (i = 0; i < lowerWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = lowerWidgets[i];
	}
	
	for (i = 0; i < upperWidgets.size(); i++) {
		activeGUI->widgets[currIndex++] = upperWidgets[i];
	}
	
	lowerWidgets.clear();
	upperWidgets.clear();

	tguiFindFocus();
}

/*
 * Returns true if some widgets have moved, otherwise false.
 */
bool tguiScreenIsDirty()
{
	if (tguiDirtyRectangle.x1 < 0)
		return false;
	return true;
}

/*
 * Remove a widget and all of it's children.
 * You should call tguiSetFocus after this.
 */
void tguiDeleteWidget(TGUIWidget* widgetToDelete)
{
	std::vector<TGUIWidget*> widgetsToKeep;
	unsigned int i;

	for (i = 0; i < activeGUI->widgets.size(); i++) {
		TGUIWidget* widget = activeGUI->widgets[i];
		if ((widget != widgetToDelete) &&
				!tguiWidgetIsChildOf(widget, widgetToDelete)) {
			widgetsToKeep.push_back(widget);
		}
	}

	int numWidgetsToDelete = activeGUI->widgets.size();

	activeGUI->widgets.clear();

	for (int i = 0; i < widgetsToKeep.size(); i++) {
		activeGUI->widgets.push_back(widgetsToKeep[i]);
	}

	widgetsToKeep.clear();

	tguiMakeFresh();
}

/*
 * Sets the parent widget. Every call to tguiAddWidget after
 * this call will add a widget with this parent.
 */
void tguiSetParent(TGUIWidget* parent)
{
	tguiCurrentParent = parent;
}

/*
 * Returns true if a widget is in the active GUI, otherwise false
 */
bool tguiWidgetIsActive(TGUIWidget* widget)
{
	for (unsigned int i = 0; i < activeGUI->widgets.size(); i++) {
		if (activeGUI->widgets[i] == widget)
			return true;
	}
	return false;
}

/*
 * Center a widget on x,y
 */
void tguiCenterWidget(TGUIWidget* widget, int x, int y)
{
	widget->setX(x - (widget->getWidth() / 2));
	widget->setY(y - (widget->getHeight() / 2));
}

TGUIWidget* tguiGetActiveWidget()
{
	return tguiActiveWidget;
}

void tguiMakeFresh()
{
	tguiActiveWidget = 0;
	activeGUI->focus = 0;
}

bool tguiWidgetIsChildOf(TGUIWidget* widget, TGUIWidget* parent)
{
	if (parent == 0) {
		return widget->getParent() == 0;
	}
	else {
		while (widget != 0) {
			if (widget->getParent() == parent) {
				return true;
			}
			widget = widget->getParent();
		}
		return false;
	}
}
