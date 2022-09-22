#include <allegro.h>
#include <stdio.h>

#include "TGUI/tgui.h"
#include "TGUI/awidgets.h"

void gamepad2Keypresses(int step)
{
	const int REPEAT_DELAY = 250;

	static int l_count = 0;
	static int r_count = 0;
	static int u_count = 0;
	static int d_count = 0;
	static int b_count = 0;

	l_count -= step;
	r_count -= step;
	u_count -= step;
	d_count -= step;
	b_count -= step;

	poll_joystick();

	if (l_count <= 0 && joy[0].stick[0].axis[0].d1) {
		printf("left\n");
		l_count = REPEAT_DELAY;
		simulate_keypress(KEY_LEFT << 8);
	}
	if (r_count <= 0 && joy[0].stick[0].axis[0].d2) {
		printf("right\n");
		r_count = REPEAT_DELAY;
		simulate_keypress(KEY_RIGHT << 8);
	}
	if (u_count <= 0 && joy[0].stick[0].axis[1].d1) {
		printf("up\n");
		u_count = REPEAT_DELAY;
		simulate_keypress(KEY_UP << 8);
	}
	if (d_count <= 0 && joy[0].stick[0].axis[1].d2) {
		printf("down\n");
		d_count = REPEAT_DELAY;
		simulate_keypress(KEY_DOWN << 8);
	}
	if (b_count <= 0 && joy[0].button[0].b) {
		printf("button\n");
		b_count = REPEAT_DELAY;
		simulate_keypress(KEY_ENTER << 8);
	}
}

void drawBG(BITMAP* bmp)
{
	clear_to_color(bmp, makecol(0, 0, 50));
	line(bmp, 0, 0, bmp->w, bmp->h, makecol(0, 0, 0));
	line(bmp, 0, bmp->h, bmp->w, 0, makecol(0, 0, 0));
}

int main(void)
{
	allegro_init();
	set_color_depth(32);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
	install_keyboard();
	install_mouse();
	install_timer();
	install_joystick(JOY_TYPE_AUTODETECT);

	BITMAP* buffer = create_bitmap(SCREEN_W, SCREEN_H);
	BITMAP* bg = create_bitmap(SCREEN_W, SCREEN_H);
	drawBG(bg);
	blit(bg, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

	aWgtInit();

	BITMAP* closeButtonBmp = load_bitmap("frameClose.pcx", 0);
	aWgtSet(AWGT_FRAME_CLOSE_BMP, closeButtonBmp);

	BITMAP* optionL = load_bitmap("optionLeft.pcx", 0);
	aWgtSet(AWGT_OPTION_LEFT_BMP, optionL);
	BITMAP* optionR = load_bitmap("optionRight.pcx", 0);
	aWgtSet(AWGT_OPTION_RIGHT_BMP, optionR);
	BITMAP* optionLD = load_bitmap("optionLeftDown.pcx", 0);
	aWgtSet(AWGT_OPTION_LEFT_DOWN_BMP, optionLD);
	BITMAP* optionRD = load_bitmap("optionRightDown.pcx", 0);
	aWgtSet(AWGT_OPTION_RIGHT_DOWN_BMP, optionRD);
	BITMAP* sliderTab = load_bitmap("sliderTab.pcx", 0);
	aWgtSet(AWGT_SLIDER_TAB_BMP, sliderTab);

	int focusColor = makecol(255, 255, 0);
	aWgtSet(AWGT_FOCUS_COLOR, (void*)(&focusColor));

	BITMAP* normal = create_bitmap(20, 20);
	clear_to_color(normal, makecol(255, 0, 0));
	BITMAP* down = create_bitmap(20, 20);
	clear_to_color(down, makecol(0, 255, 0));
	BITMAP* hover = create_bitmap(20, 20);
	clear_to_color(hover, makecol(255, 255, 0));

	tguiInit(buffer);
	AWgtFrame* frame = new AWgtFrame(0, 0, 400, 400, "Window",
			true,
			makecol(80, 80, 80), makecol(0, 0, 50),
			makecol(100, 100, 100), makecol(255, 255, 255), 
			0, AWGT_TEXT_BORDER, 200,
			true, true, false);
	AWgtFrame* frame2 = new AWgtFrame(50, 50, 400, 400, "Window 2",
			true,
			makecol(80, 80, 80), makecol(0, 0, 50),
			makecol(100, 100, 100), makecol(255, 255, 255), 
			0, AWGT_TEXT_BORDER, 200,
			true, true, false);
	AWgtButton* button = new AWgtButton(0, 0, -1, -1, 0, "Button",
			makecol(200, 200, 200), makecol(50, 50, 50),
			0, 0, 0, AWGT_TEXT_NORMAL);
	tguiCenterWidget(button, 200, 350);
	AWgtText* text = new AWgtText(0, 0, "This is some text",
			makecol(255, 255, 255), 0, AWGT_TEXT_DROP_SHADOW);
	tguiCenterWidget(text, 200, 50);
	AWgtCheckbox* checkbox = new AWgtCheckbox(0, 0, 0, "Checkbox",
			makecol(255, 255, 255), 0, AWGT_TEXT_DROP_SHADOW,
			false);
	tguiCenterWidget(checkbox, 200, 70);
	std::vector<char*> options;
	options.push_back("One");
	options.push_back("Number Two");
	options.push_back("Three");
	options.push_back("And Four");
	AWgtOption* option = new AWgtOption(0, 0, -1, &options,
			makecol(255, 255, 255), 0, AWGT_TEXT_DROP_SHADOW,
			1);
	tguiCenterWidget(option, 200, 85);
	AWgtSlider* slider = new AWgtSlider(0, 0, 250, 250, 1,
			makecol(255, 255, 255), 0, 2);
	tguiCenterWidget(slider, 200, 100);
	AWgtIcon* icon = new AWgtIcon(0, 0, true, normal, hover, down, 0);
	tguiCenterWidget(icon, 200, 120);
	tguiAddWidget(frame2);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(text);
	tguiAddWidget(checkbox);
	tguiAddWidget(option);
	tguiAddWidget(slider);
	tguiAddWidget(icon);
	tguiAddWidget(button);

	tguiSetFocus(slider);

	long timeStep = 60;

	int saved_mouse = mouse_b;
	int saved_mouse_x = mouse_x;
	int saved_mouse_y = mouse_y;

	while (!key[KEY_ESC]) {
		long start = tguiCurrentTimeMillis();
		if (tguiScreenIsDirty()) {
			int x1 = tguiDirtyRectangle.x1;
			int y1 = tguiDirtyRectangle.y1;
			int x2 = tguiDirtyRectangle.x2;
			int y2 = tguiDirtyRectangle.y2;
			blit(bg, buffer, x1, y1, x1, y1, x2-x1, y2-y1);
			tguiClearDirtyRectangle();
			show_mouse(NULL);
			blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
			show_mouse(screen);
		}
		int new_mouse = mouse_b;
		int new_mouse_x = mouse_x;
		int new_mouse_y = mouse_y;
		if (keypressed() || (new_mouse != saved_mouse) || new_mouse ||
			(new_mouse_x != saved_mouse_x) ||
			(new_mouse_y != saved_mouse_y)) {
			saved_mouse = new_mouse;
			saved_mouse_x = new_mouse_x;
			saved_mouse_y = new_mouse_y;
			TGUIWidget* widget = tguiUpdate();
			if (widget == frame || widget == frame2)
				break;
			else if (widget == icon)
				printf("icon pressed\n");
			else if (widget == button)
				printf("button pressed\n");
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		if (duration < timeStep)
			rest(timeStep-duration);
		gamepad2Keypresses(tguiCurrentTimeMillis() - start);
	}

	return 0;
}
