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
	door = Object:new{number=2, x=144, y=232, width=16, base_height=8}
	mystic = Object:new{number=3, x=160, y=72, anim_set="seaside-mystic"}
	makeCharacter(3)
	setObjectDirection(3, DIRECTION_WEST)
	father = Object:new{number=4, x=48, y=168, anim_set="seaside-father"}
	makeCharacter(4)
	setObjectDirection(4, DIRECTION_EAST)
	mother = Object:new{number=5, x=64, y=72, anim_set="seaside-mother"}
	makeCharacter(5)
	setObjectDirection(5, DIRECTION_SOUTH)
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		fadeOut()
		startArea("seaside1-2")
		setObjectPosition(1, 128, 144)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 3) then
		doDialogue("Mystic: Your son is still alive... I can sense that he is growing weak.$")
		setMilestone(MS_SPOKE_TO_MYSTIC, true)
	elseif (activated == 4) then
		doDialogue("Father: Our son disappeared from our village a few years back... We found tunnels leading deep underground...$")
		doDialogue("Father: Those who followed the tunnels did not return. They have been sealed, but we still have hope of seeing our son again one day...$")
	elseif (activated == 5) then
		doDialogue("Mother: I'm sorry to hear the news of your son...$")
		if (not checkMilestone(MS_SPOKE_TO_MYSTIC)) then
			doDialogue("Mother: My daughter is a mystic, perhaps she can give you news of him?$")
		end
	end
end

function update(step)
end
