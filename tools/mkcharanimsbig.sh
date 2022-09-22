#!/bin/sh

mkanim $1_stand_n.ani stand_n 1 $2 $1_stand_n.pcx 0
mkanim $1_stand_e.ani stand_e 1 $2 $1_stand_e.pcx 0
mkanim $1_stand_s.ani stand_s 1 $2 $1_stand_s.pcx 0
mkanim $1_stand_w.ani stand_w 1 $2 $1_stand_w.pcx 0
mkanim $1_walk_n.ani walk_n $3 $2 $1_walk_n.pcx $5 $5
mkanim $1_walk_e.ani walk_e $4 $2 $1_walk_e.pcx $6 $6 $6 $6
mkanim $1_walk_s.ani walk_s $3 $2 $1_walk_s.pcx $5 $5
mkanim $1_walk_w.ani walk_w $4 $2 $1_walk_w.pcx $6 $6 $6 $6
mkanimset $1.anims $1 8 $1_stand_n.ani $1_stand_e.ani $1_stand_s.ani \
	$1_stand_w.ani $1_walk_n.ani $1_walk_e.ani $1_walk_s.ani $1_walk_w.ani
