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
	startMusic("shop.ogg")
	outportal = Object:new{number=2, x=160, y=184, width=TILE_SIZE, base_height=8}
	upportal = Object:new{number=3, x=96, y=96, width=TILE_SIZE, base_height=8}
	keeper = Object:new{number=4, x=160, y=120, width=16, base_height=8, anim_set="shopkeeper"}
	makeCharacter(4)
	setObjectDirection(4, DIRECTION_SOUTH)
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj1 == 2 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 64, 176)
		startArea("village1-1");
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1)) then
		fadeOut()
		setObjectPosition(1, 96, 112)
		setObjectDirection(1, DIRECTION_SOUTH)
		startArea("village-shop2")
		fadeIn()
	end
end

function activate(activated, activated)
	if (activated == 4) then
		if (checkMilestone(MS_BOUGHT_LOOKING_SCOPE)) then
			doDialogue("Shopkeeper: Hi there, see anything you're interested in?$")
			--doShop("Shopkeeper", 2, ITEM_CURE, 5, ITEM_HEAL, 10)
			doShop("Shopkeeper", 1, ITEM_CURE, 5)
		else
			doDialogue("Shopkeeper: Hi there, see anything you're interested in?$")
			n = doShop("Shopkeeper", 1, ITEM_LOOKING_SCOPE, 10)
			if (n > 0) then
				setMilestone(MS_BOUGHT_LOOKING_SCOPE, true)
			end
		end
	end
end

function update(step)
end
