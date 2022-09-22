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
	startMusic("silence.ogg")
	upportal = Object:new{number=2, x=144, y=16, width=32, base_height=24}
	scroll = Chest:new{number=3, x=96, y=128, base_height=16, anim_set="invisible_chest", milestone=MS_SCROLL_RAIN, id=ITEM_SCROLL_RAIN}
	-- Far wall
	book1 = Object:new{number=4, x=16, y=16, width=32, base_height=32}
	book2 = Object:new{number=5, x=48, y=16, width=32, base_height=32}
	book3 = Object:new{number=6, x=80, y=16, width=32, base_height=32}
	book4 = Object:new{number=7, x=112, y=16, width=32, base_height=32}
	book5 = Object:new{number=8, x=176, y=16, width=32, base_height=32}
	book6 = Object:new{number=9, x=208, y=16, width=32, base_height=32}
	book7 = Object:new{number=10, x=240, y=16, width=32, base_height=32}
	book8 = Object:new{number=11, x=272, y=16, width=32, base_height=32}
	-- Top inner rows
	book9 = Object:new{number=12, x=32, y=64, width=16, base_height=48}
	book10 = Object:new{number=13, x=64, y=64, width=16, base_height=48}
	book11 = Object:new{number=14, x=96, y=64, width=16, base_height=48}
	book12 = Object:new{number=15, x=128, y=64, width=16, base_height=48}
	book13 = Object:new{number=16, x=176, y=64, width=16, base_height=48}
	book14 = Object:new{number=17, x=208, y=64, width=16, base_height=48}
	book15 = Object:new{number=18, x=240, y=64, width=16, base_height=48}
	book16 = Object:new{number=19, x=272, y=64, width=16, base_height=48}
	-- Bottom inner rows
	book17 = Object:new{number=20, x=32, y=160, width=16, base_height=48}
	book18 = Object:new{number=21, x=64, y=160, width=16, base_height=48}
	book19 = Object:new{number=22, x=96, y=160, width=16, base_height=48}
	book20 = Object:new{number=23, x=128, y=160, width=16, base_height=48}
	book21 = Object:new{number=24, x=176, y=160, width=16, base_height=48}
	book22 = Object:new{number=25, x=208, y=160, width=16, base_height=48}
	book23 = Object:new{number=26, x=240, y=160, width=16, base_height=48}
	book24 = Object:new{number=27, x=272, y=160, width=16, base_height=48}
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		fadeOut()
		startArea("library1")
		px, py = getObjectPosition(1)
		setObjectPosition(1, px, 48)
		setObjectDirection(1, DIRECTION_SOUTH)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 3) then
		scroll:activate()
	elseif (activated == 4) then
		doDialogue("Coro: \"The Great Kingdom\".$")
	elseif (activated == 5) then
		doDialogue("Coro: \"Kings and Queens\".$")
	elseif (activated == 6) then
		doDialogue("Coro: \"Famous Knights\".$")
	elseif (activated == 7) then
		doDialogue("Coro: \"History of Seaside\".$")
	elseif (activated == 8) then
		doDialogue("Coro: \"Local Geography\".$")
	elseif (activated == 9) then
		doDialogue("Coro: \"Book of Maps\".$")
	elseif (activated == 10) then
		doDialogue("Coro: \"History of War\".$")
	elseif (activated == 11) then
		doDialogue("Coro: \"The Dragon Warrior\".$")
	elseif (activated == 12) then
		doDialogue("Coro: \"Forging Weaponry\".$")
	elseif (activated == 13) then
		doDialogue("Coro: \"Women's Handbook\".$")
	elseif (activated == 14) then
		doDialogue("Coro: \"Stone Craftsmanship\".$")
	elseif (activated == 15) then
		doDialogue("Coro: \"Crafting Furniture\".$")
	elseif (activated == 16) then
		doDialogue("Coro: \"Musical Scores\".$")
	elseif (activated == 17) then
		doDialogue("Coro: \"Book of Paintings\".$")
	elseif (activated == 18) then
		doDialogue("Coro: \"Great Artists\".$")
	elseif (activated == 19) then
		doDialogue("Coro: \"Seaside Poetry\".$")
	elseif (activated == 20) then
		doDialogue("Coro: \"Practicing Good Manners\".$")
	elseif (activated == 21) then
		doDialogue("Coro: \"Children's Games\".$")
	elseif (activated == 22) then
		doDialogue("Coro: \"Children's Fables\".$")
	elseif (activated == 23) then
		doDialogue("Coro: \"Little Miss Goose\".$")
	elseif (activated == 24) then
		doDialogue("Coro: \"The Magic Bean\".$")
	elseif (activated == 25) then
		doDialogue("Coro: \"Voyage Across the Sea\".$")
	elseif (activated == 26) then
		doDialogue("Coro: \"Fantastic Journeys\".$")
	elseif (activated == 27) then
		doDialogue("Coro: \"Ancient Myths\".$")
	end
end

function update(step)
end
