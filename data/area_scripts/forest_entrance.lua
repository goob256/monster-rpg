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

player_scripted = false

function start()
	startMusic("forest.ogg")
	exit = Object:new{number=2, x=144, y=232, width=32, base_height=8}
	entrance = Object:new{number=3, x=144, y=0, width=32, height=16}
	if (not checkMilestone(MS_BURNED_LOG)) then
		log = Object:new{number=4, x=128,  y=80, width=64, base_height=32, total_height=32, anim_set="giant_log"}
	else
		log = Object:new{number=4, x=128,  y=80, width=64, base_height=32, total_height=32, anim_set="ashes"}
		setObjectSolid(4, false)
	end
end

function stop()
end

function collide(obj1, obj2)
	if ((obj1 == 1 and obj2 == 2) or (obj2 == 1 and obj1 == 2)) then
		fadeOut()
		startArea("otherside64-63")
		setObjectPosition(1, 192, 144)
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 3) or (obj2 == 1 and obj1 == 3)) then
		fadeOut()
		startArea("final_battle")
		setObjectPosition(1, 152, 224)
		fadeIn()
	end
end

function activate(activator, activated)
	if (not checkMilestone(MS_BURNED_LOG) and activated == 4) then
		if (checkMilestone(MS_LEARNED_FIRE)) then
			px, py = getObjectPosition(1)
			player = Object:scriptify{number=1, x=px, y=py, width=16, base_height=8, anim_set="Coro"}
			player.move_type = MOVE_WALK
			player.destx = px
			player.desty = py+32
			player_scripted = true
			updateUntilArrival(1, px, py+32)
			setObjectDirection(1, DIRECTION_NORTH)
			doDialogue("Coro: Here goes nothing...$")
			burnStuff(2, 144, 112, 176, 112)
			setObjectAnimationSet(4, "ashes")
			setObjectSolid(4, false)
			setObjectInputToPlayerControlled(1)
			setObjectDirection(1, DIRECTION_NORTH)
			player_scripted = false
			setMilestone(MS_BURNED_LOG, true)
		else
			doDialogue("Coro: This could be a problem...$")
		end
	end
end

function update(step)
	if (player_scripted == true) then
		player:move(step)
	end
end
