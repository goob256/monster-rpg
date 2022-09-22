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

ROOM_COST = 75

function start()
    startMusic("shop.ogg");
    fda = Object:new{number=2, x=144, y=88, anim_set="fda"}
    makeCharacter(2)
    setObjectDirection(2, DIRECTION_SOUTH)
    exit = Object:new{number=3, x=144, y=200, width=16, base_height=8}
    nogo = Object:new{number=4, x=272, y=64, width=16, base_height=16}
end

function stop()
end

function update(step)
end

function activate(activator, activated)
    if (activated == 2) then
	cost = 0
	if (checkGold() < ROOM_COST) then
	    cost = checkGold()
	else
	    cost = ROOM_COST
	end
	response = doQuestion("Agent: Rooms are " .. cost .. " gold.", "Would you like one?")
	if (response == false) then
	    -- do nothing
	else
	    takeGold(cost)
	    fadeOut()
	    revitalizePlayers()
	    setObjectPosition(1, 272, 88)
	    setObjectDirection(1, DIRECTION_SOUTH)
	    setObjectDirection(2, DIRECTION_EAST)
	    redrawArea()
	    fadeIn()
	    doDialogue("Agent: I hope you slept well! Come back soon!$")
	    setObjectDirection(2, DIRECTION_SOUTH)
	end
    end
end

function collide(obj1, obj2)
    if (obj1 == 3 or obj2 == 3) then
	fadeOut()
	startArea("seaside0-0")
	setObjectPosition(1, 160, 128)
	fadeIn()
    elseif (obj1 == 4 or obj2 == 4) then
	doDialogue("Agent: *Ahem* ... You have to PAY to get a room!$")
	px, py = getObjectPosition(1)
	setObjectPosition(1, px, py+2)
    end
end
