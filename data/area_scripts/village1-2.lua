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

function create_portal()
	startMusic("village.ogg")
	portal = Object:new{number=3, x=0, y=112, width=8, base_height=16}
end

function start()
	if (checkMilestone(MS_GAVE_LOOKING_SCOPE)) then
		gate = Door:new{number=2, x=0, y=112, width=16, base_height=16, anim_set="gate_h"}
		guard = nil
		create_portal()
	else
		gate = Door:new{number=2, x=0, y=112, width=16, base_height=16, anim_set="gate_h", locked=true}
		guard = Object:new{number=3, x=16, y=104, width=16, base_height=8, anim_set="guard"}
		makeCharacter(3)
		setObjectDirection(3, DIRECTION_SOUTH)
	end
end

function stop()
end

function update(step)
	if (not (guard == nil)) then
		guard:move(step)
	end
	gate:update(step)
end

function activate(activator, activated)
	if (activated == 2) then
		gate:activate()
	elseif ((not (guard == nil)) and activated == 3) then
		if (checkMilestone(MS_BOUGHT_LOOKING_SCOPE)) then
			doDialogue("Guard: WOW!!! Is that a looking scope? Thanks!!!$")
			setMilestone(MS_GAVE_LOOKING_SCOPE, true)
			gate.locked = false
			setObjectSolid(3, false)
			guard.move_type = MOVE_WALK
			guard.destx = 16
			guard.desty = 64
			-- Stop player from walking
			player_dir = getObjectDirection(1)
			setObjectSubAnimation(1, player_dir)
			stopObject(1)
			updateUntilArrival(3, 16, 64)
			guard.destx = BUFFER_WIDTH-17
			guard.desty = 64
			updateUntilArrival(3, BUFFER_WIDTH-17, 64)
			removeObject(3)
			guard = nil
			create_portal()
		else
			doDialogue("Guard: There are a lot of monsters in the area right now... I can't let you out.$")
		end
	end
end

function collide(obj1, obj2)
	if (guard == nil and ((obj1 == 1 and obj2 == 3) or (obj1 == 3 and obj2 == 1))) then
		fadeOut();
		setObjectPosition(1, 192, 200)
		startArea("worldone1-3")
		fadeIn()
	elseif ((obj1 == 1 and obj2 == 2) or (obj2 == 1 and obj1 == 2)) then
		if (objectIsSolid(2)) then
			gate:activate()
		end
	end
end
