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
	portal = Object:new{number=2, x=160, y=184, width=16, base_height=8}
	shopkeeper = Object:new{number=3, x=160, y=120, anim_set="shopkeeper2"}
	makeCharacter(3)
	setObjectDirection(3, DIRECTION_SOUTH)
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 2 and obj2 == 1) or (obj1 == 1 and obj2 == 2)) then
		fadeOut()
		startArea("seaside2-0")
		setObjectPosition(1, 144, 96)
		fadeIn()
	end
end

function activate(activator, activated)
	if (activated == 3) then
		doDialogue("Shopkeeper: Hello, how can I help you?$")
		--doShop("Shopkeeper", 7, ITEM_CURE2, 25, ITEM_CURE3, 60, ITEM_HEAL, 80, ITEM_HERB, 75, ITEM_BAIT, 50, ITEM_BONE_BLADE, 750, ITEM_BONEMAIL, 750)
		doShop("Shopkeeper", 5, ITEM_CURE2, 25, ITEM_CURE3, 60, ITEM_HERB, 75, ITEM_BONE_BLADE, 750, ITEM_BONEMAIL, 750)
	end
end

function update(step)
end
