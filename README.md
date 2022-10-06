# Monster RPG

Monster RPG is a classic style JRPG in the theme of old NES titles of the 80s
and 90s. Like those RPGs, you gain experience and level up through combat,
while talking to characters and advancing in your quest to save your child
from the hands of a monster.

[Monster RPG 2](http://www.nooskewl.ca/monster-rpg-2) is a "loose sequel" to Monster RPG.

## Building

The game can be built for Linux, Mac OS X and Windows systems.

### Dependencies

* [Allegro 4.4](http://liballeg.org)
* [FudgeFont](http://fudgefont.sourceforge.net)
* [Lua 5.1](http://www.lua.org)
* [Freetype 2](http://freetype.org)
* [Ogg & Vorbis](http://www.xiph.org/downloads)

### Build instructions (Linux)

First, build the TGUI dependency:
```
cd tgui-0.9
make -f Makefile.unix
```

Then build the game engine:
```
cd ../src
make -f Makefile.unix
```

To play the game, run the ```src/monster``` executable from the main repo directory
which contains the ```data``` folder.
