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
	startMusic("loner.ogg")
	hermit = Object:new{number=2, x=128, y=102, anim_set="hermit"}
	makeCharacter(2)
	setObjectDirection(2, DIRECTION_SOUTH)
	scroll_chest = Chest:new{number=3, x=192, y=96, base_height=16, anim_set="chest", milestone=MS_SCROLL_FIRE, id=ITEM_SCROLL_FIRE}
	exit = Object:new{number=4, x=160, y=168, width=16, base_height=8}
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 4) or (obj1 == 4 and obj2 == 1)) then
		fadeOut()
		startArea("otherside66-61")
		setObjectPosition(1, 160, 128)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 2) then
		if (checkMilestone(MS_SCROLL_FIRE)) then
			doDialogue("Hermit: Best to get out of the desert now...$")
		else
			doDialogue("Hermit: Heat is easy to come by out here... take the contents of that chest, it is of no use to me.$")
		end
	elseif (activated == 3) then
		scroll_chest:activate()
	end
end

function update(step)
end
