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
	bird1 = StillBird:new{number=2, x=128, y=24, base_height=8, anim_set="redbird"}
	bird2 = StillBird:new{number=3, x=240, y=88, base_height=8, anim_set="redbird"}
	bird3 = StillBird:new{number=4, x=80, y=200, base_height=8, anim_set="redbird"}
	flyingbird = FlyingBird:new{number=5, width=10, base_height=10, anim_set="bluebird", direction=DIRECTION_SOUTH}
	if (checkMilestone(MS_GAVE_LOOKING_SCOPE)) then
		guard = Object:new{number=6, x=176, y=168, width=16, base_height=8, anim_set="lookingguard", move_type=MOVE_WANDER}
		makeCharacter(6)
		guard:wander()
	else
		guard = nil
	end
end

function stop()
end

function update(step)
	bird1:update(step)
	bird2:update(step)
	bird3:update(step)
	flyingbird:update(step)
	if (not (guard == nil)) then
		guard:move(step)
	end
end

function activate(activator, activated)
	if (activated == 2 or activated == 3 or activated == 4) then
		doDialogue("... TWEET ... TWEET ...$")
	elseif (not (guard == nil) and activated == 6) then
		doDialogue("Guard: I saw a bluebird today!!!$")
	end
end

function collide(obj1, obj2)
end
