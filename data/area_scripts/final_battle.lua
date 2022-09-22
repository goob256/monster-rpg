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
player_scripted = false
scene_done = false


function start()
	startMusic("forest.ogg")
	eny = Object:new{number=2, x=128, y=80, width=14, height=14, anim_set="Eny"}
	makeCharacter(2)
	setObjectDirection(2, DIRECTION_SOUTH)
	tiggy = Object:new{number=3, x=176, y=80, width=14, height=14, anim_set="Tiggy"}
	makeCharacter(3)
	setObjectDirection(3, DIRECTION_SOUTH)
	warlock = Object:new{number=4, x=144, y=62, width=33, height=49, anim_set="Warlock"}
end

function stop()
end

function collide(obj1, obj2)
end

function activate(activator, activated)
end

function update(step)
	if (not inited) then
		inited = true
	elseif (not player_scripted and not scene_done) then
		px, py = getObjectPosition(1)
		player = Object:scriptify{number=1, x=px, y=py, width=16, base_height=8, anim_set="Coro"}
		player.move_type = MOVE_WALK
		player.destx = px
		player.desty = 152
		player_scripted = true
		updateUntilArrival(1, px, player.desty)
		setObjectSubAnimationNamed(1, "stand_n")
		redrawArea()
		redrawScreen()
		rest(1800)
		doDialogue("Warlock: How inconvenient...$")
		doDialogue("Warlock: I was just about to extend my life...$")
		doDialogue("Warlock: Oh well, two more souls won't hurt...$")
		startBattle("WarlockBoss", get_battle_terrain())
		if (checkMilestone(MS_DEFEATED_WARLOCK) == true) then
			startMusic("silence.ogg")
			removeObject(4)
			if (getPartner() > 0) then
				px, py = getObjectPosition(1)
				moryt = Object:new{number=4, x=px, y=py, anim_set="Moryt"}
				makeCharacter(4)
				setObjectDirection(4, DIRECTION_NORTH)
				setObjectSolid(4, false)
				player.destx = px
				player.desty = py - 32
				updateUntilArrival(1, px, player.desty)
				setObjectSubAnimationNamed(1, "stand_s")
				redrawArea()
				redrawScreen()
				rest(1000)
				doDialogue("Coro: Moryt, how can I ever thank you?$")
				doDialogue("Moryt: The monsters were a threat to us all...$")
				doDialogue("Moryt: Now that their creator is gone, we're safe.$")
				doDialogue("Moryt: I'll take Tiggy back to his parents.$")
				setObjectSubAnimationNamed(1, "stand_n")
				redrawArea()
				redrawScreen()
				doDialogue("Coro: Eny! Let's go home!...$")
			else
				px, py = getObjectPosition(1)
				player.destx = px
				player.desty = py - 32
				updateUntilArrival(1, px, player.desty)
				setObjectSubAnimationNamed(1, "stand_n")
				redrawArea()
				redrawScreen()
				doDialogue("Coro: Eny! Thank God you're safe!$")
				rest(1000)
				doDialogue("Coro: Let's take your friend back to Seaside, then go home...$")
			end
			showEnding()
			setPlayersDead(true)
		end
		scene_done = true
	elseif (not scene_done) then
		player:move(step)
	end
end
