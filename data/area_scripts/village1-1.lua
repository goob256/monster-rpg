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
	cow1 = Object:new{number=2, x=80, y=32, width=32, base_height=20, anim_set="cow1"}
	cow2 = Object:new{number=3, x=208, y=64, width=32, base_height=20, anim_set="cow2"}
	farmer = Object:new{number=4, x=144, y=120, width=16, base_height=8, anim_set="farmer"}
	makeCharacter(4)
	setObjectDirection(4, DIRECTION_SOUTH)
	door1 = Door:new{number=5, x=64, y=160, width=16, base_height=16, anim_set="homedoor_s"}
	door2 = Door:new{number=6, x=240, y=160, width=16, base_height=16, anim_set="homedoor_s"}
	portal1 = Object:new{number=7, x=64, y=160, width=16, base_height=8}
	portal2 = Object:new{number=8, x=240, y=160, width=16, base_height=8}
end

function stop()
end

function update(step)
	door1:update(step)
	door2:update(step)
end

function activate(activator, activated)
	if (activated == 2 or activated == 3) then
		doDialogue("MOOO!!!$")
	elseif (activated == 4) then
		if ((getPartner() > 0) and not checkMilestone(MS_GOT_MILK)) then
			doDialogue("Farmer: You fellers have been workin' hard... have some milk!$")
			playSound("give");
			setMaxHP(getMaxHP()+50)
			setHP(getMaxHP())
			setMilestone(MS_GOT_MILK, true)
		elseif (checkMilestone(MS_GOT_MILK)) then
			doDialogue("Farmer: Don't get greedy now...$")
		else
			doDialogue("Farmer: These here are my cows, and they ain't for sale!$")
		end
	elseif (activated == 5) then
		door1:activate()
	elseif (activated == 6) then
		door2:activate()
	end
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 7) or (obj1 == 7 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 160, 176)
		startArea("village-shop1")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 8) or (obj1 == 8 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 144, 192)
		startArea("farmershome1")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 5) or (obj1 == 5 and obj2 == 1)) then
		if (objectIsSolid(5)) then
			door1:activate()
		end
	elseif ((obj1 == 1 and obj2 == 6) or (obj1 == 6 and obj2 == 1)) then
		if (objectIsSolid(6)) then
			door2:activate()
		end
	end
end
