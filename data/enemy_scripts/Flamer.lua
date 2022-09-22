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

id = nil

start_hp = 500
start_latency = 35

function get_size()
	return 19, 55, 51, 8
end

function get_power()
	return 200
end

function get_weapon_defense(weapon)
	return 250
end

function get_spell_defense(spell_name)
	if (spell_name == "Fire") then
		return 1000
	elseif (spell_name == "Rain") then
		return -60
	else
		return 0
	end
end

function start(theID)
	id = theID
	setCombatantHP(id, start_hp)
	setCombatantMaxHP(id, start_hp)
	setCombatantLatency(id, start_latency)
	setCombatantExperience(id, start_experience)
end

function update(step)
end

function get_action()
	--if (randint(2) == 1) then
	--	return TURN_ATTACK, 0, "", get_random_player()
	--else
		return TURN_SPELL, 0, "Fire", get_random_player()
	--end
end

function stop()
end
