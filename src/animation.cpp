#include <cstring>

#include <allegro.h>

#include "animation.h"
#include "debug.h"
#include "io.h"

char* Animation::getName()
{
	return name;
}

int Animation::getFrame()
{
	return currFrame;
}

void Animation::setFrame(int frame)
{
	currFrame = frame;
}

int Animation::getNumFrames()
{
	return numFrames;
}

int Animation::getDelay(int frame)
{
	return delays[frame];
}

void Animation::draw(BITMAP* bmp, int x, int y)
{
	draw_sprite(bmp, frames[currFrame], x, y);
}

void Animation::drawTrans(BITMAP* bmp, int x, int y)
{
	draw_trans_sprite(bmp, frames[currFrame], x, y);
}

void Animation::saveFrame(BITMAP* bmp, int delay, PACKFILE* f) throw (WriteError)
{
	for (int y = 0; y < bmp->h; y++) {
		for (int x = 0; x < bmp->w; x++) {
			int p = getpixel(bmp, x, y);
			int r = getr(p);
			int g = getg(p);
			int b = getb(p);
			my_pack_putc(r, f);
			my_pack_putc(g, f);
			my_pack_putc(b, f);
		}
	}
	iputl(delay, f);
}

void Animation::save(PACKFILE* f) throw (WriteError)
{
	iputl(strlen(name), f);
	for (int i = 0; name[i]; i++)
		my_pack_putc(name[i], f);
	iputl(numFrames, f);
	iputl(frames[0]->w, f);
	iputl(frames[0]->h, f);
	for (int i = 0; i < numFrames; i++)
		saveFrame(frames[i], delays[i], f);
}

void Animation::loadFrame(int w, int h, BITMAP* dest, int* delay, PACKFILE* f) throw (ReadError)
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int r = my_pack_getc(f);
			int g = my_pack_getc(f);
			int b = my_pack_getc(f);
			putpixel(dest, x, y, makecol(r, g, b));
		}
	}

	*delay = igetl(f);
}

void Animation::load(char *filename) throw (std::bad_alloc, ReadError)
{
	PACKFILE *f = pack_fopen(filename, "rp");
	if (!f)
		throw new ReadError();
	
	load(f);

	pack_fclose(f);
}

void Animation::load(PACKFILE* f) throw (std::bad_alloc, ReadError)
{
	int nameLen = igetl(f);

	name = (char*)malloc(nameLen+1);

	for (int i = 0; i < nameLen; i++)
		name[i] = my_pack_getc(f);
	name[nameLen] = 0;

	numFrames = igetl(f);
	int width = igetl(f);
	int height = igetl(f);

	try {
		frames = new BITMAP*[numFrames];

		for (int i = 0; i < numFrames; i++)
			frames[i] = 0;

		for (int i = 0; i < numFrames; i++) {
			frames[i] = create_bitmap(width, height);
			if (!frames[i])
				throw new std::bad_alloc();
		}

		delays = new int[numFrames];

		for (int i = 0; i < numFrames; i++) {
			loadFrame(width, height, frames[i], &delays[i], f);
		}
	}
	catch (...) {
		free(name);
		for (int i = 0; i < numFrames; i++)
			if (frames[i])
				destroy_bitmap(frames[i]);
		if (delays)
			delete[] delays;
		throw new std::bad_alloc();
	}
}

/*
 * Advance the animation one from, in a loop
 */
void Animation::advance()
{
	currFrame++;
	/*
	 * This is like "if (currFrame >= numFrames) currFrame = 0"
	 */
	currFrame %= numFrames;
}

/*
 * Returns true when animation has looped
 */
bool Animation::update(int step)
{
	count += step;
	if (count >= delays[currFrame]) {
		count -= delays[currFrame];
		advance();
		if (currFrame == 0) {
			return true;
		}
	}
	return false;
}

BITMAP* Animation::getBitmap(int frame)
{
	return frames[frame];
}

BITMAP* Animation::getCurrentBitmap()
{
	return getBitmap(currFrame);
}

Animation::Animation(char* name, int numFrames, BITMAP** frames, int* delays) throw (std::bad_alloc) :
	frames(frames),
	delays(delays),
	count(0),
	numFrames(numFrames),
	currFrame(0)
{
	this->name = strdup(name);
	if (!this->name)
		throw new std::bad_alloc();

	/*
	this->delays = 0;

	try {
		this->frames = new BITMAP*[numFrames];
		for (int i = 0; i < numFrames; i++) {
			this->frames[i] = 0;
		}
		for (int i = 0; i < numFrames; i++) {
			this->frames[i] = create_bitmap(frames[i]->w,
					frames[i]->h);
			if (this->frames[i] == 0)
				throw new std::bad_alloc();
			blit(frames[i], this->frames[i], 0, 0, 0, 0,
					frames[i]->w, frames[i]->h);
		}
		this->delays = new int[numFrames];
		for (int i = 0; i < numFrames; i++)
			this->delays[i] = delays[i];
	}
	catch (std::bad_alloc e) {
		free(this->name);
		if (this->frames) {
			for (int i = 0; i < numFrames; i++) {
				if (this->frames[i])
					destroy_bitmap(this->frames[i]);
				else
					break;
			}
			delete[] this->frames;
		}
		if (this->delays)
			delete[] this->delays;
		throw new std::bad_alloc();
	}
	*/
}

Animation::Animation() :
    name(0),
    frames(0),
    delays(0),
    count(0),
    currFrame(0)
{
}

Animation::~Animation()
{
	if (name)
		free(name);
	for (int i = 0; i < numFrames; i++)
		destroy_bitmap(frames[i]);
	if (frames)
		delete[] frames;
	if (delays)
		delete[] delays;
}
	
void AnimationSet::setSubAnimation(int anim)
{
	currAnim = anim;
}

/*
 * Returns true if an animation by this name exists
 */
bool AnimationSet::setSubAnimation(const char* subName)
{
	for (int i = 0; i < numAnims; i++) {
		if (!strcmp(anims[i]->getName(), subName)) {
			debug_message("Set sub animation to %s\n", subName);
			currAnim = i;
			return true;
		}
	}
	return false;
}

void AnimationSet::setFrame(int frame)
{
	anims[currAnim]->setFrame(frame);
}

int AnimationSet::getFrame()
{
	return anims[currAnim]->getFrame();
}

void AnimationSet::draw(BITMAP* dest, int x, int y)
{
	anims[currAnim]->draw(dest, x, y);
}

bool AnimationSet::update(int step)
{
    bool ret = anims[currAnim]->update(step);
    return ret;
}

Animation* AnimationSet::getCurrentAnimation()
{
	return anims[currAnim];
}

/*
 * Reset to frame 0
 */
void Animation::reset()
{
	currFrame = 0;
	count = 0;
}

AnimationSet::AnimationSet(const char* filename) throw (std::bad_alloc, ReadError) :
	currAnim(0)
{
	PACKFILE* f = pack_fopen(filename, "rp");
	if (!f)
		throw new ReadError();

	int nameLen = igetl(f);
	name = (char*)malloc(nameLen+1);
	if (!name) {
		pack_fclose(f);
		throw new std::bad_alloc();
	}

	anims = 0;

	try {
		for (int i = 0; i < nameLen; i++)
			name[i] = my_pack_getc(f);
		name[nameLen] = 0;
		numAnims = igetl(f);
		anims = new Animation*[numAnims];
		for (int i = 0; i < numAnims; i++)
			anims[i] = 0;
		for (int i = 0; i < numAnims; i++) {
			anims[i] = new Animation();
			anims[i]->load(f);
		}
	}
	catch (...) {
		free(name);
		if (anims) {
			for (int i = 0;  i < numAnims; i++) {
				if (anims[i])
					delete anims[i];
			}
			delete[] anims;
		}
		throw new ReadError();
	}

	pack_fclose(f);

	debug_message("Animation set %s loaded\n", filename);
}

AnimationSet::~AnimationSet()
{
	for (int i = 0; i < numAnims; i++)
		delete anims[i];
	delete[] anims;
	free(name);
}
