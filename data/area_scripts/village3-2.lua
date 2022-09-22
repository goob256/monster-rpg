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
	door1 = Door:new{number=2, x=96, y=96, base_height=16, anim_set="homedoor_s"}
	door2 = Door:new{number=3, x=224, y=64, base_height=16, anim_set="homedoor_s"}
	portal1 = Object:new{number=4, x=96, y=96, width=16, base_height=8}
	portal2 = Object:new{number=5, x=224, y=64, width=16, base_height=8}
	boy = Object:new{number=6, x=224, y=112, width=14, base_height=7, anim_set="boy1", move_type=MOVE_WANDER, rest=2000}
	makeCharacter(6)
end

function stop()
end

function update(step)
	door1:update(step)
	door2:update(step)
	boy:move(step)
end

function activate(activator, activated)
	if (activated == 6) then
		n = randint(2)
		if (n == 1) then
			doDialogue("Boy: No, I haven't seen Eny since I was fetching water this morning.$")
		else
			doDialogue("Boy: My daddy is the strongest man in town!$")
		end
	elseif (activated == 2) then
		door1:activate()
	elseif (activated == 3) then
		door2:activate()
	end
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 4) or (obj1 == 4 and obj2 == 1)) then
		fadeOut();
		setObjectPosition(1, 160, 176)
		startArea("mildredshome1")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 5) or (obj1 == 5 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 144, 192)
		startArea("guardshome1")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		if (objectIsSolid(2)) then
			door1:activate()
		end
	elseif ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1)) then
		if (objectIsSolid(3)) then
			door2:activate()
		end
	end
end
