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
	outportal = Object:new{number=2, x=144, y=232, width=32, base_height=8}
	downportal = Object:new{number=3, x=144, y=16, width=32, base_height=24}
	librarian = Object:new{number=4, x=16, y=184, anim_set="librarian"}
	makeCharacter(4)
	setObjectDirection(4, DIRECTION_EAST)
	-- book = Object:new{number=, x=, y=, width=, base_height=}
	-- Far wall
	book1 = Object:new{number=5, x=16, y=16, width=32, base_height=32}
	book2 = Object:new{number=6, x=48, y=16, width=32, base_height=32}
	book3 = Object:new{number=7, x=80, y=16, width=32, base_height=32}
	book4 = Object:new{number=8, x=112, y=16, width=32, base_height=32}
	book5 = Object:new{number=9, x=176, y=16, width=32, base_height=32}
	book6 = Object:new{number=10, x=208, y=16, width=32, base_height=32}
	book7 = Object:new{number=11, x=240, y=16, width=32, base_height=32}
	book8 = Object:new{number=12, x=272, y=16, width=32, base_height=32}
	-- Inner rows
	book9 = Object:new{number=13, x=32, y=64, width=16, base_height=96}
	book10 = Object:new{number=14, x=64, y=64, width=16, base_height=96}
	book11 = Object:new{number=15, x=96, y=64, width=16, base_height=96}
	book12 = Object:new{number=16, x=128, y=64, width=16, base_height=96}
	book13 = Object:new{number=17, x=176, y=64, width=16, base_height=96}
	book14 = Object:new{number=18, x=208, y=64, width=16, base_height=96}
	book15 = Object:new{number=19, x=240, y=64, width=16, base_height=96}
	book16 = Object:new{number=20, x=272, y=64, width=16, base_height=96}
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		fadeOut()
		startArea("seaside1-0")
		setObjectPosition(1, 160, 96)
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1)) then
		fadeOut()
		startArea("library0")
		px, py = getObjectPosition(1)
		setObjectPosition(1, px, 48)
		setObjectDirection(1, DIRECTION_SOUTH)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 4) then
		if (randint(2) == 1) then
			doDialogue("Librarian: Feel free to browse our collection...$")
		else
			doDialogue("Librarian: Please try to keep quiet while in the library...$")
		end
	elseif (activated == 5) then
		doDialogue("Coro: \"Modern Surgery\".$");
	elseif (activated == 6) then
		doDialogue("Coro: \"Journal of Health\".$");
	elseif (activated == 7) then
		doDialogue("Coro: \"Medical Adviser\".$");
	elseif (activated == 8) then
		doDialogue("Coro: \"Herbal Remedies\".$");
	elseif (activated == 9) then
		doDialogue("Coro: \"Veterinary Journal\".$");
	elseif (activated == 10) then
		doDialogue("Coro: \"Mathematics\".$");
	elseif (activated == 11) then
		doDialogue("Coro: \"Science of Building\".$");
	elseif (activated == 12) then
		doDialogue("Coro: \"Linguistics\".$");
	elseif (activated == 13) then
		doDialogue("Coro: \"Sign Language\".$");
	elseif (activated == 14) then
		doDialogue("Coro: \"Ancient Sorcery\".$");
	elseif (activated == 15) then
		doDialogue("Coro: \"Advanced Gardening\".$");
	elseif (activated == 16) then
		doDialogue("Coro: \"Shipbuilding\".$");
	elseif (activated == 17) then
		doDialogue("Coro: \"Marine Sketches\".$");
	elseif (activated == 18) then
		doDialogue("Coro: \"Fisherman's Guide to the Sea\".$");
	elseif (activated == 19) then
		doDialogue("Coro: \"Navigation\".$");
	elseif (activated == 20) then
		doDialogue("Coro: \"Mountain Treking\".$");
	end
end

function update(step)
end
