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
	downportal = Object:new{number=2, x=32, y=48, width=16, base_height=8}
	wife = Object:new{number=3, x=240, y=88, anim_set="moryts-wife"}
	makeCharacter(3)
	setObjectDirection(3, DIRECTION_SOUTH)
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj1 == 1)) then
		fadeOut()
		startArea("morytshome0")
		setObjectPosition(1, 32, 64)
		setObjectDirection(1, DIRECTION_SOUTH)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 3) then
		if (checkMilestone(MS_LOST_MORYT)) then
			doDialogue("Moryt's Wife: I know you will find your son...$")
		elseif (checkMilestone(MS_GAINED_MORYT)) then
			doDialogue("Moryt's Wife: You are blessed to have such a man on your side...$")
		else
			doDialogue("Moryt's Wife: Talk to my husband... Perhaps he can help you.$")
		end
	end
end

function update(step)
end
