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
	door = Object:new{number=2, x=144, y=184, width=32, base_height=8}
	fire = Object:new{number=3, x=233, y=85, anim_set="fire"}
	shanty = Object:new{number=4, x=144, y=80, anim_set="Shanty"}
	makeCharacter(4)
	setObjectDirection(4, DIRECTION_SOUTH)
end

function stop()
end

function update(step)
end

function activate(activator, activated)
	if (activated == 4) then
		doDialogue("Shanty: It's a real struggle living in the wild... I have to make my own weapons!$")
		doShop("Shanty", 4, ITEM_WILD_MAN_SWORD, 350, ITEM_WILD_MAN_ARMOR, 350, ITEM_CURE, 10, ITEM_CURE2, 30)
	end
end

function collide(obj1, obj2)

	if (obj1 == 2 or obj2 == 2) then
		fadeOut()
		startArea("worldone2-3")
		setObjectPosition(1, 256, 96)
		fadeIn()
	end
end
