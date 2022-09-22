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
	door_l = Door:new{number=2, x=112, y=160, base_height=16, anim_set="homedoor_s"}
	portal_l = Object:new{number=3, x=112, y=160, width=16, base_height=8}
	door_r = Door:new{number=4, x=208, y=160, base_height=16, anim_set="homedoor_s"}
	portal_r = Object:new{number=5, x=208, y=160, width=16, base_height=8}
	chicken1 = Object:new{number=6, x=48, y=48, move_type=MOVE_WANDER, anim_set="chicken", rest=2000}
	makeCharacter(6)
	chicken2 = Object:new{number=7, x=112, y=80, move_type=MOVE_WANDER, anim_set="chicken", rest=2000}
	makeCharacter(7)
end

function stop()
end

function update(step)
	door_l:update(step)
	door_r:update(step)
	chicken1:move(step)
	chicken2:move(step)
end

function activate(activator, activated)
	if (activated == 6 or activated == 7) then
		doDialogue("BUC BUC BUC...$")
	elseif (activated == 2) then
		door_l:activate()
	elseif (activated == 4) then
		door_r:activate()
	end
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1)) then
		fadeOut()
		startArea("victim0")
		setObjectPosition(1, 160, 151)
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 5) or (obj1 == 5 and obj2 == 1)) then
		fadeOut()
		startArea("thief0")
		setObjectPosition(1, 160, 151)
		fadeIn()
   	elseif ((obj1 == 1 and obj2 == 2) or (obj2 == 1 and obj1 == 2)) then
    		if (objectIsSolid(2)) then
			door_l:activate()
		end
   	elseif ((obj1 == 1 and obj2 == 4) or (obj2 == 1 and obj1 == 4)) then
    		if (objectIsSolid(4)) then
			door_r:activate()
		end
	end
end
