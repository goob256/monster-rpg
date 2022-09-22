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

NUM_SONGS = 2
SONG_LENGTH = 20000
song_playing = false
song_start = 0
orig_music_volume = getMusicVolume()
song_voice_number = 0
PENDANT_COST = 500

function start()
    startMusic("shop.ogg")
    listener = Object:new{number=2, x=16, y=152, anim_set="seaside-guy1"}
    makeCharacter(2)
    setObjectDirection(2, DIRECTION_EAST)
    musician = Object:new{number=3, x=40, y=96, anim_set="musician", no_input=true}
    makeCharacter(3)
    setObjectSubAnimationNamed(3, "stand_s")
    thief = Object:new{number=4, x=192, y=152, anim_set="thief"}
    makeCharacter(4)
    setObjectDirection(4, DIRECTION_WEST)
    bartender = Object:new{number=5, x=176, y=104, anim_set="bartender"}
    makeCharacter(5)
    setObjectDirection(5, DIRECTION_SOUTH)
end

function stop()
    setMilestone(MS_INN_HEARD_SONG, false)
    if (song_playing) then
	stopSound(song_voice_number)
	setMusicVolume(orig_music_volume)
    end
end

function update(step)
    if (song_playing) then
	if ((getTime() - song_start) > SONG_LENGTH) then
	    song_playing = false
	    setMusicVolume(orig_music_volume)
	    setObjectSubAnimationNamed(3, "stand_s")
	end
    end
end

function activate(activator, activated)
    if (activated == 2) then
	if (checkMilestone(MS_INN_HEARD_SONG)) then
	    doDialogue("Guy: I love that song!!!$")
	else
	    doDialogue("Guy: I really enjoy the music here...$")
	end
    elseif (activated == 3) then
	if (not song_playing) then
	    doDialogue("Musician: Sure, I'll play a song for you...$")
	    setMilestone(MS_INN_HEARD_SONG, true)
	    song_num = randint(NUM_SONGS)
	    setMusicVolume(0)
	    song_voice_number = playSound("guitar" .. song_num)
	    song_start = getTime()
	    song_playing = true
	    setObjectSubAnimationNamed(3, "play")
	end
    elseif (activated == 4) then
	if (checkMilestone(MS_GAVE_BACK_PENDANT)) then
	    doDialogue("Rupy: I've been really down on my luck lately...$")
	elseif (checkMilestone(MS_BOUGHT_PENDANT)) then
	    doDialogue("Rupy: Come see me some other time... I usually have some nice merchandise... hehe...$")
	else
	    answer = doQuestion("Rupy: I've got a crystal pendant for sale.", "" .. PENDANT_COST .. " gold. Are you interested?")
	    if (answer == true) then
		if (checkGold() < PENDANT_COST) then
		    doDialogue("Rupy: Ha... that's not enough!$")
		else
		    playSound("give")
		    setMilestone(MS_BOUGHT_PENDANT, true)
		    takeGold(PENDANT_COST)
		    doDialogue("Rupy: Nice doing business with you... hehe...$")
		end
	    else
		doDialogue("Rupy: Fine, I'll find someone else to sell it to...$")
	    end
	end
    elseif (activated == 5) then
	doDialogue("Bartender: We're lucky to have one of the greatest musicians around with us...$")
    end
end

function collide(obj1, obj2)
end
