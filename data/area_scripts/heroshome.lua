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

can_save = true

function start()
	startMusic("village.ogg")
	door = Object:new{number=2, x=144, y=232, width=16, base_height=8}
	mother = Object:new{number=3, x=104, y=64, anim_set="mother"}
	makeCharacter(3)
	setObjectDirection(3, DIRECTION_SOUTH)
	bed = Object:new{number=4, x=272, y=48, width=32, base_height=32}
end

function stop()
end

function update(step)
end

function activate(activator, activated)
	if (activated == 3) then
		if (checkMilestone(MS_SPOKE_TO_MYSTIC)) then
			doDialogue("Ami: The Mystic must be right... please find Eny before it's too late!$")
		else
			if (checkMilestone(MS_CHECKED_WELL)) then
				doDialogue("Ami: Oh dear... Please find Eny!$")
			else
				doDialogue("Ami: Coro, please go find Eny. Supper will be ready soon.$")
			end
		end
	elseif (activated == 4) then
		doDialogue("Coro: I'm so tired...$")
		fadeOut()
		revitalizePlayers()
		setObjectDirection(1, DIRECTION_SOUTH)
		setObjectPosition(1, 256, 64)
		fadeIn()
	end
end

function collide(obj1, obj2)
	if (obj1 == 2 or obj2 == 2) then
		fadeOut()
		startArea("village3-3")
		setObjectPosition(1, 192, 128)
		fadeIn()
	end
end
