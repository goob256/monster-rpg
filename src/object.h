#ifndef OBJECT_H
#define OBJECT_H

#include "animation.h"
#include "input.h"
#include "collision.h"

enum ObjectType {
	OBJECT_NORMAL = 0,
	OBJECT_CHARACTER = 1
};

/*
 * These speeds correspond to pixels moved per millisecond.
 */
const float WALK_SPEED = 0.05f;
const int WALK_ANIM_DELAY = 800;
const int MIN_OBJECT_SIZE = 8;

class Area;

class Object {
public:
	void setAnimationSet(AnimationSet* anim_set);
	void setInput(Input* input);
	Input* getInput();
	void deleteInput();
	void draw(BITMAP* buf);
	void update(Area* area, int step);
	void updateAnimation(int step);
	int getX();	// get x/y as int
	int getY();
	float getFX();	// get x/y as float
	float getFY();
	bool isHigh();
	void setHigh(bool high);
	void setPosition(float x, float y);
	void setType(ObjectType type);
	void setWalkSpeed(float speed);
	void setDirection(Direction direction);
	Direction getDirection();
	void setHidden(bool hidden);
	bool isHidden();
	int getWidth();
	int getHeight();
	int getTotalHeight();
	void setTotalHeight(int th);
	void setSolid(bool solid);
	bool isSolid();
	void advanceAnimation();
	bool isAnimated();
	void setAnimated(bool animated);
	BoundingBox* getBoundingBox();
	void setAnimationFrame(int frame);
	void setSubAnimation(int sub);
	bool setSubAnimation(const char* name);
	/*
	 * Objects are created with their base on the
	 * highest point in the area on the tile where
	 * x,y falls.
	 */
	Object(int x, int y, int w, int base_h, int total_height);
	virtual ~Object();
protected:
	float x;
	float y;		// bottom (feet)
	/*
	 * The last known position where the object was
	 * on solid ground.
	 */
	float good_x;
	float good_y;
	float speed_xy;
	int totalHeight;
	BoundingBox base;
	AnimationSet* animSet;
	Input* input;
	bool high;
	ObjectType type;
	float walkSpeed;
	bool hidden;
	bool solid;
	bool animated;
};

extern Object* player;

#endif
