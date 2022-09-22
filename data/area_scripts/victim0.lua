--
-- Copyright (c) 2007, Trent Gamblin
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--    -- Redistributions of source code must retain the above copyright
--       notice, this list of conditions and the following disclaimer.
--    -- Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--    -- Neither the name of the <organization> nor the
--       names of its contributors may be used to endorse or promote products
--       derived from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY <copyright holder> ``AS IS'' AND ANY
-- EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
-- DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--

function start()
	startMusic("village.ogg")
	outportal = Object:new{number=2, x=160, y=168, width=TILE_SIZE, base_height=8}
	upportal = Object:new{number=3, x=96, y=96, width=TILE_SIZE, base_height=8}
	victim = Object:new{number=4, x=160, y=112, anim_set="victim"}
	makeCharacter(4)
	setObjectDirection(4, DIRECTION_SOUTH)
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 112, 176)
		startArea("seaside0-2")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1)) then
		if (checkMilestone(MS_GAVE_BACK_PENDANT)) then
			fadeOut()
			setObjectPosition(1, 96, 112)
			setObjectDirection(1, DIRECTION_SOUTH)
			startArea("victim1");
			fadeIn()
		else
			setObjectDirection(4, DIRECTION_WEST)
			redrawArea()
			doDialogue("Lady: Please stay out of my bedroom...$")
			setObjectDirection(4, DIRECTION_SOUTH)
			px, py = getObjectPosition(1)
			setObjectPosition(1, px, py+2)
		end
	end
end

function activate(activator, activated)
	if (activated == 4) then
		if (checkMilestone(MS_GAVE_BACK_PENDANT)) then
			doDialogue("Lady: Thank you so much for getting my pendant back!$")
		elseif (checkMilestone(MS_BOUGHT_PENDANT)) then
			doDialogue("Lady: Oh my dear! That's my pendant! Thank you so much! I'm going to keep my door locked when I'm out from now on!$")
			setMilestone(MS_GAVE_BACK_PENDANT, true)
		else
			doDialogue("Lady: Someone has been stealing from me when I'm out... Who would do such a thing?$")
		end
	end
end

function update(step)
end
