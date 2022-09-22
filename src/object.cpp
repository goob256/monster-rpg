#include <allegro.h>

#include "monster.h"

void Object::setAnimationSet(AnimationSet* anim_set)
{
	if (animSet)
		delete animSet;
	animSet = anim_set;
}


void Object::setInput(Input* input)
{
	this->input = input;
}

Input* Object::getInput()
{
	return input;
}

void Object::deleteInput()
{
    delete input;
    input = 0;
}

void Object::draw(BITMAP* buf)
{
	if (animSet) {
		int draw_y = (int)y - (totalHeight - base.height);
		animSet->draw(buf, (int)x, draw_y);
	}
}

void Object::update(Area* area, int fullstep)
{
	InputDescriptor still;
	InputDescriptor* id;

	if (input) {
		input->update();

		id = input->getDescriptor();
	}
	else {
		still.left = false;
		still.right = false;
		still.up = false;
		still.down = false;
		still.button1 = false;
		still.button2 = false;
		id = &still;
	}

	if (id->button1 || (key_shifts & KB_SHIFT_FLAG) || (config.getGamepadAvailable() && joy[0].button[config.getJoyButton3()].b)) {
		fullstep *= 2;
	}

	bool moving;

	if (id->left || id->right || id->up || id->down)
		moving = true;
	else
		moving = false;

	float newx = x;
	float newy = y;

	/*
	 * Move no more than 1 pixel at a time.
	 */
	int step;
	int steps_done;
	int step_step = 1;

	for (steps_done = 0; steps_done < fullstep;) {

		if ((steps_done + step_step) >= fullstep)
			step = fullstep - steps_done;
		else
			step = step_step;

		steps_done += step;

		if (id->left)
			newx -=  walkSpeed * step;
		if (id->right)
			newx += walkSpeed * step;
		if (id->up)
			newy -= walkSpeed * step;
		if (id->down)
			newy += walkSpeed * step;
	
		if (type == OBJECT_CHARACTER) {
			BoundingBox box;
			memcpy(&box, &base, sizeof(BoundingBox));
			box.width -= TILE_SIZE/4;
			float xx = newx + TILE_SIZE/8;
			if (!area->checkCollision(this, (int)xx, (int)y, &box)) {
				x = newx;
			}
			else {
				newx = x;
			}
			
			xx = newx + TILE_SIZE/8;

			if (!area->checkCollision(this, (int)xx, (int)newy, &box)) {
				y = newy;
			}
		}
		else {
			if (!area->checkCollision(this, (int)newx, (int)y, &base)) {
				x = newx;
			}
			else {
				newx = x;
			}

			if (!area->checkCollision(this, (int)newx, (int)newy, &base)) {
				y = newy;
			}
		}
	}

	if (type == OBJECT_CHARACTER && input) {
		if (moving)
			animSet->setSubAnimation(4+id->direction);
		else
			animSet->setSubAnimation(id->direction);

	}

	if (animated && animSet) {
		animSet->update(fullstep);
	}
}

/*
 * Update just the animation.
 */
void Object::updateAnimation(int step)
{
	animSet->update(step);
}

int Object::getX()
{
	return (int)x;
}


int Object::getY()
{
	return (int)y;
}

float Object::getFX()
{
	return x;
}


float Object::getFY()
{
	return y;
}

bool Object::isHigh()
{
	return high;
}


void Object::setHigh(bool high)
{
	this->high = high;
}

void Object::setPosition(float x, float y)
{
	this->x = x;
	this->y = y;
}

void Object::setType(ObjectType type)
{
	this->type = type;
}

/*
 * Set the speed the object walks (or moves) at.
 */
void Object::setWalkSpeed(float speed)
{
	walkSpeed = speed;
}

/*
 * Set an objects direction and change its
 * animation appropriately.
 */
void Object::setDirection(Direction direction)
{
	input->setDirection(direction);
	
	if (type == OBJECT_CHARACTER) {
		debug_message("Setting sub animation %d\n", direction);
		animSet->setSubAnimation(direction);
	}
}

Direction Object::getDirection()
{
	if (input) {
		InputDescriptor* id = input->getDescriptor();
		return id->direction;
	}
	else
		return DIRECTION_SOUTH;
}

/*
 * Hide or unhide an object.
 */
void Object::setHidden(bool hidden)
{
	this->hidden = hidden;
}

bool Object::isHidden()
{
	return hidden;
}

int Object::getWidth()
{
	return base.width;
}

int Object::getHeight()
{
	return base.height;
}

int Object::getTotalHeight()
{
    return totalHeight;
}

void Object::setTotalHeight(int th)
{
    totalHeight = th;
}

void Object::setSolid(bool solid)
{
	this->solid = solid;
}

bool Object::isSolid()
{
	return solid;
}

void Object::advanceAnimation()
{
	Animation* a = animSet->getCurrentAnimation();
	a->advance();
}

bool Object::isAnimated()
{
	return animated;
}

void Object::setAnimated(bool animated)
{
	this->animated = animated;
}

BoundingBox* Object::getBoundingBox()
{
	return &base;
}

void Object::setAnimationFrame(int frame)
{
	animSet->setFrame(frame);
}

void Object::setSubAnimation(int sub)
{
	animSet->setSubAnimation(sub);
}

bool Object::setSubAnimation(const char* name)
{
    return animSet->setSubAnimation(name);
}

Object::Object(int x, int y, int w, int base_h, int total_height) :
	x((float)x),
	y((float)y),
	totalHeight(total_height),
	animSet(0),
	input(0),
	high(false),
	type(OBJECT_NORMAL),
	walkSpeed(WALK_SPEED),
	hidden(false),
	solid(true),
	animated(true)
{
	base.width = w;
	base.height = base_h;
}

Object::~Object()
{
	if (animSet)
		delete animSet;
	if (input)
		delete input;
}
