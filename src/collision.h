#ifndef CD_H
#define CD_H

struct BoundingBox {
	int width;
	int height;
};

inline bool checkBoxCollision(int x1, int y1, int x2, int y2, int x3, int y3,
	int x4, int y4)
{
	if ((y2 < y3) || (y1 > y4) || (x2 < x3) || (x1 > x4))
		return false;
	return true;
}

#endif
