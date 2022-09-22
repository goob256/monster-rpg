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

inited = false
scene_started = false
done_walking = false

function start()
    startMusic("cave.ogg")
    if (not checkMilestone(MS_DEFEATED_CYCLOPS)) then
        cyclops = Object:new{number=2, x=144, y=50, anim_set="../enemy_animations/Cyclops", width=30, base_height=20}
	setObjectTotalHeight(2, 50)
    else
	exit = Object:new{number=2, x=128, y=0, width=64, base_height=8}
    end
end

function stop()
end

function update(step)
    if (not inited) then
	start_x, start_y = getObjectPosition(1)
	inited = true
    else
	px, py = getObjectPosition(1)
	if (not scene_started and not checkMilestone(MS_DEFEATED_CYCLOPS)
		and (px > start_x or py ~= start_y)) then
	    scene_started = true
	    px, py = getObjectPosition(1)
	    player = Object:scriptify{number=1, x=0, y=py, width=16, base_height=8, anim_set="Coro"}
	    player.move_type = MOVE_WALK
	    player.destx = 152
	    player.desty = py
	    updateUntilArrival(1, 152, py)
	    player.move_type = MOVE_WALK
	    player.destx = 152
	    player.desty = 80
	    updateUntilArrival(1, 152, 80)
	    done_walking = true
	    setObjectInputToPlayerControlled(1)
	    setObjectDirection(1, DIRECTION_NORTH)
	    doDialogue("Coro: Allow me to pass!$")
	    doDialogue("Cyclops: AAAAAAAAAARRRGH!!!$")
	    startBattle("CyclopsBoss", get_battle_terrain())
	    if (checkMilestone(MS_DEFEATED_CYCLOPS)) then
		removeObject(2)
		cyclops = nil
		exit = Object:new{number=2, x=128, y=0, width=64, base_height=8}
	    end
	elseif (scene_started and not checkMilestone(MS_DEFEATED_CYCLOPS) and not done_walking) then
	    player:move(step)
	end
    end
end

function activate(activator, activated)
end

function collide(obj1, obj2)
    if (obj1 == 2 or obj2 == 2) then
	fadeOut()
	startArea("otherside64-64")
	setObjectPosition(1, 16, 216)
	setObjectDirection(1, DIRECTION_EAST)
	fadeIn()
    end
end
