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
	oldman = Object:new{number=2, x=112, y=112, move_type=MOVE_WANDER, anim_set="oldman1"}
	makeCharacter(2)
	door = Door:new{number=3, x=208, y=96, width=TILE_SIZE, base_height=TILE_SIZE, anim_set="homedoor_s"}
	portal = Object:new{number=4, x=208, y=96, width=TILE_SIZE, base_height=8}
end

function stop()
end

function update(step)
	oldman:move(step)
	door:update(step)
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 4) or (obj1 == 4 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 160, 176)
		startArea("oldmanshome1")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1)) then
		if (objectIsSolid(3)) then
			door:activate()
		end
	end
end

function activate(activator, activated)
	if (activated == 2) then
		if (checkMilestone(MS_OLDMANS_SWORD)) then
			doDialogue("Old Man: Oh... I see you found my old sword. Probably can't use it as good as me!$")
		else
			doDialogue("Old Man: Mildred and I are the only ones that have been here since before the walls...$")
		end
	elseif (activated == 3) then
		door:activate()
	end
end
