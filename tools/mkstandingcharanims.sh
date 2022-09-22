#!/bin/sh

mkanim $1_stand_n.ani stand_n 1 $2 $1_stand_n.pcx 333 333
mkanim $1_stand_e.ani stand_e 1 $2 $1_stand_e.pcx 333 333
mkanim $1_stand_s.ani stand_s 1 $2 $1_stand_s.pcx 333 333
mkanim $1_stand_w.ani stand_w 1 $2 $1_stand_w.pcx 333 333
mkanimset $1.anims $1 4 $1_stand_n.ani $1_stand_e.ani $1_stand_s.ani \
	$1_stand_w.ani
