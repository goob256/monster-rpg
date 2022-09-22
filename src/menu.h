#ifndef MENU_H
#define MENU_H

extern int pickItemSmall();
extern void loadMenuData() throw (ReadError);
extern void destroyMenuData();
extern void drawMenu(BITMAP* bmp);
extern bool pauseGame();
extern int selectSaveState(char* caption, char* s1, char* s2, char* s3, bool def);
extern bool prompt(bool dfault, char* s1, char* s2, char* s3);
extern void notify(char* s1, char* s2, char* s3);
extern void jumboNotify(char *s[16]);
extern void showItemDescription(int id);

#endif
