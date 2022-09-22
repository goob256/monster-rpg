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
	outportal = Object:new{number=2, x=144, y=216, width=32, base_height=8}
	upportal = Object:new{number=3, x=32, y=48, width=16, base_height=8}
	if (not checkMilestone(MS_GAINED_MORYT) or checkMilestone(MS_LOST_MORYT)) then
		moryt = Object:new{number=4, x=144, y=80, anim_set="Moryt"}
		makeCharacter(4)
		setObjectDirection(4, DIRECTION_SOUTH)
	end
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		fadeOut()
		startArea("seaside2-2")
		setObjectPosition(1, 144, 160)
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj1 == 1)) then
		fadeOut()
		startArea("morytshome1")
		setObjectPosition(1, 32, 64)
		setObjectDirection(1, DIRECTION_SOUTH)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 4) then
		if (not checkMilestone(MS_LOST_MORYT)) then
			doDialogue("Moryt: I have heard the news about your son...$")
			doDialogue("Moryt: Allow me to join you to help you recover him!$")
			playSound("partner")
			setMilestone(MS_GAINED_MORYT, true)
			setPartner(PARTNER_MORYT)
			removeObject(4)
			moryt = nil
		end
	end
end

function update(step)
end
