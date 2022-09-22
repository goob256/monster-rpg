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

-- constants
BUFFER_WIDTH = 320
BUFFER_HEIGHT = 240

TILE_SIZE = 16

DIRECTION_NORTH = 1
DIRECTION_EAST = 2
DIRECTION_SOUTH = 3
DIRECTION_WEST = 4

MOVE_NONE = 1
MOVE_WANDER = 2
MOVE_WALK = 3

min_battle_dist = 20
max_battle_dist = 50
last_player_x = 0
last_player_y = 0
accumulated_distance = 0
curr_battle_dist = 0
battle_probability = 10
initial_player_pos_set = false

battle_types = {}
num_battle_types = 0

function init_random_battles(min_dist, max_dist, probability)
	min_battle_dist = min_dist
	max_battle_dist = max_dist
	curr_battle_dist = 0
	battle_probability = probability
end

function time_for_battle()
	if (initial_player_pos_set == false) then
		last_player_x, last_player_y = getObjectPosition(1)
		accumulated_distance = getAccumulatedDistance()
		initial_player_pos_set = true
	end
	new_player_x, new_player_y = getObjectPosition(1)
	if (new_player_x < 0 or new_player_y < 0 or
		(new_player_x+TILE_SIZE) >= BUFFER_WIDTH or
		(new_player_y+8) >= BUFFER_HEIGHT) then
		return false
	end
	change_x = math.abs(last_player_x - new_player_x)
	change_y = math.abs(last_player_y - new_player_y)
	if (not (change_x == 0) and not (change_y == 0)) then
		change = math.sqrt(change_x*change_x + change_y*change_y)
	elseif (not (change_x == 0)) then
		change = change_x
	else
		change = change_y
	end
	moved = false
	if (not (change == 0)) then
		moved = true
	end
	curr_battle_dist = curr_battle_dist + change
	curr_battle_dist = curr_battle_dist + accumulated_distance
	accumulated_distance = 0
	last_player_x = new_player_x
	last_player_y = new_player_y
	if (curr_battle_dist < min_battle_dist) then
		return false
	elseif (curr_battle_dist > max_battle_dist) then
		curr_battle_dist = 0
		return true
	elseif (moved) then
		n = randint(1000)
		if (n <= battle_probability) then
			curr_battle_dist = 0
			return true
		else
			return false
		end
	end
end

-- battle_type is:
-- array of { id, x, y }
function add_battle_type(battle_type)
	num_battle_types = num_battle_types + 1
	battle_types[num_battle_types] = battle_type
end

function check_battle()
	if (time_for_battle()) then
		battle_type = randint(num_battle_types)
		startBattle(battle_types[battle_type], get_battle_terrain())
		accumulated_distance = 0
		setAccumulatedDistance(0)
	else
		setAccumulatedDistance(curr_battle_dist)
	end
end

function get_battle_terrain()
	x, y = getObjectPosition(1)
	x = x + TILE_SIZE/2
	l1, l2, l3 = getTileIndices(x/TILE_SIZE+1, y/TILE_SIZE+1)
	if (l1 == 127 or l2 == 127 or l3 == 127) then
		return "forest"
	elseif (l1 == 128 or l2 == 128 or l3 == 128) then
		return "plains"
	elseif (l1 == 154 or l2 == 154 or l3 == 154) then
		return "cave"
	elseif (l1 == 2 or l2 == 2 or l3 == 2) then
		return "sand"
	elseif (l1 == 310 or l2 == 310 or l3 == 310) then
		return "inforest"
	else
		return "ohnoes"
	end
end

function dist(x1, y1, x2, y2)
	dx = x1 - x2
	dy = y1 - y2
	return math.sqrt(dx*dx + dy*dy)
end

function closeto(n1, n2)
	ACCEPTABLE_DISTANCE = 0.9
	if (n1 >= (n2-ACCEPTABLE_DISTANCE) and n1 <= (n2+ACCEPTABLE_DISTANCE)) then
		return true
	else
		return false
	end
end

function a_or_and(s)
	ss = string.lower(string.sub(s,1,1))
	if (ss == "a" or ss == "e" or ss == "i" or ss == "o" or ss == "u") then
		return "an"
	else
		return "a"
	end
end

Object =  {
	MAX_WANDER_DIST = 16,
	MIN_WANDER_DIST = 8,
	move_type = MOVE_NONE,
	number = -1,
	x = 0,
	y = 0,
	startx = 0,
	starty = 0,
	endx = 0,
	endy = 0,
	width = TILE_SIZE,
	height = 8,
	anim_set = nil,
	rest = 1000,
	next_move = 0
}

function Object:new(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	o.startx = o.x
	o.starty = o.y
	o.destx = o.x
	o.desty = o.y
	o.rest = o.rest or Object.rest
	o.next_move = 0
	o.lastx = o.lastx or 0
	o.lasty = o.lasty or 0
	o.range = o.range or 40
	o.width = o.width or 16
	o.base_height = o.base_height or 8
	o.total_height = o.total_height or 16
	addObject(o.x, o.y, o.width, o.base_height, o.total_height);
	if (not (o.anim_set == nil)) then
		setObjectAnimationSet(o.number, o.anim_set)
	end
	if (not o.no_input) then
	    setObjectInputToScriptControlled(o.number);
	end
	if (not (o.move_type == nil) and (o.move_type == MOVE_WANDER)) then
		o:wander()
	end
	return o
end

function Object:scriptify(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	o.startx = o.x
	o.starty = o.y
	o.destx = o.x
	o.desty = o.y
	o.rest = o.rest or Object.rest
	o.next_move = 0
	o.lastx = o.lastx or 0
	o.lasty = o.lasty or 0
	setObjectInputToScriptControlled(o.number);
	if (not (o.move_type == nil) and (o.move_type == MOVE_WANDER)) then
		o:wander()
	end
	return o
end

function Object:rand_wander_dist()
	range = Object.MAX_WANDER_DIST - Object.MIN_WANDER_DIST
	return math.random() * range + Object.MIN_WANDER_DIST
end

function Object:rand_wander_direction()
	r = math.floor(math.random() * 4) + 1
	return r
end

function Object:wander()
	move_dist = self.rand_wander_dist()

	dx = self.x - self.startx
	dy = self.y - self.starty

	if (math.abs(dx) > self.range or math.abs(dy) > self.range) then
		if (dx < -self.range) then
			self.destx = self.x + 20
		elseif (dx > self.range) then
			self.destx = self.x - 20
		end
		if (dy < -self.range) then
			self.desty = self.y + 20
		elseif (dy > self.range) then
			self.desty = self.y - 20
		end
	else
		dir = self.rand_wander_direction()
		if (dir == DIRECTION_NORTH) then
			self.desty = self.y - move_dist
		elseif (dir == DIRECTION_EAST) then
			self.destx = self.x + move_dist
		elseif (dir == DIRECTION_SOUTH) then
			self.desty = self.y + move_dist
		else
			self.destx = self.x - move_dist
		end
	end

	if (self.destx < 0) then
		self.destx = 0
	elseif (self.destx >= (BUFFER_WIDTH-self.width)) then
		self.destx = BUFFER_WIDTH-self.width-1
	end

	if (self.desty < 0) then
		self.desty = 0
	elseif (self.desty >= (BUFFER_HEIGHT-self.height)) then
		self.desty = BUFFER_HEIGHT-self.height-1
	end
end

function Object:move(step)
	if (self.move_type == self.MOVE_NONE) then
		return
	end

	self.x, self.y = getObjectPosition(self.number)

	l = 0
	r = 0
	u = 0
	d = 0

	if (closeto(self.x, self.destx)) then
		self.x = self.destx
	elseif (self.x < self.destx) then
		r = 1
	else
		l = 1
	end
	
	if (closeto(self.y, self.desty)) then
		self.y = self.desty
	elseif (self.y < self.desty) then
		d = 1
	else
		u = 1
	end

	self.next_move = self.next_move + step
	if (self.x == self.destx and self.y == self.desty and self.next_move > self.rest) then
		self.next_move = 0
		if (self.move_type == MOVE_WANDER) then
			self:wander()
		end
	else
		moveObject(self.number, l, r, u, d)
	end

	if (self.next_move > self.rest) then
		if (self.lastx == self.x and self.lasty == self.y) then
			self.next_move = 0
			if (self.move_type == MOVE_WANDER) then
				self:wander()
			end
		end
	end

	self.lastx = self.x
	self.lasty = self.y
end

Door = {}

function Door:activate()
	if (not self.locked and self.count >= 250) then
		playSound("door")
		self.count = 0
		setObjectSolid(self.number, not objectIsSolid(self.number))
		if (objectIsSolid(self.number)) then
			--if (objectIsColliding(self.number)) then
			if (objectsAreColliding(self.number, 1)) then
				setObjectSolid(self.number, false)
			else
				advanceObjectAnimation(self.number)
			end
		else
			advanceObjectAnimation(self.number)
		end
	end
end

function Door:update(step)
	self.count = self.count + step
end

function Door:new(o)
	o = Object:new(o)
	o.activate = Door.activate
	o.update = Door.update
	o.locked = o.locked or false
	o.count = 0
	setObjectAnimated(o.number, false)
	return o
end

Chest = {}

function Chest:activate()
	if (not self.open) then
		if (self.itemtype == ITEM_GOLD) then
			advanceObjectAnimation(self.number)
			giveGold(self.quantity)
			setMilestone(self.milestone, true)
			msg = "You found " .. self.quantity .. " gold!$";
			playSound("give")
			doPickupMessage(msg)
			self.open = true
		else
			i = findInventoryItem(self.id)
			if (i < 0) then
				i = findFirstEmptyInventorySlot()
				if (i < 0) then
					doMessage("You can't carry anything else...$")
				else
					advanceObjectAnimation(self.number)
					setInventory(i, self.id, self.quantity)
					setMilestone(self.milestone, true)
					playSound("give")
					if (self.quantity > 1) then
						msg = "You found " .. self.quantity .. " " .. item_names[self.id] .. "s!$"
						doPickupMessage(msg)
					else
						msg = "You found " .. a_or_and(item_names[self.id]) .. " " .. item_names[self.id] .. "!$"
						doPickupMessage(msg)
					end
					self.open = true
				end
			else
				advanceObjectAnimation(self.number)
				addToInventory(i, self.quantity)
				playSound("give")
				if (self.quantity > 1) then
					msg = "You found " .. self.quantity .. " " .. item_names[self.id] .. "s!$"
					doPickupMessage(msg)
				else
					msg = "You found " .. a_or_and(item_names[self.id]) .. " " .. item_names[self.id] .. "!$"
					doPickupMessage(msg)
				end
				setMilestone(self.milestone, true)
				self.open = true
			end
		end
	end
end

function Chest:new(o)
	o = Object:new(o)
	o.activate = Chest.activate
	o.open = checkMilestone(o.milestone)
	if (o.open) then
		advanceObjectAnimation(o.number)
	end
	o.itemtype = o.itemtype or ITEM_ITEM
	o.id = o.id or 0
	o.quantity = o.quantity or 1
	setObjectAnimated(o.number, false)
	return o
end

StillBird = {}

function StillBird:update(step)
	self.change_count = self.change_count - step
	if (self.change_count <= 0) then
		self.direction = randint(4)
		setObjectSubAnimation(self.number, self.direction)
		self.change_count = randint(4000) + 1000
	end
	self.chirp_count = self.chirp_count - step
	if (self.chirp_count <= 0) then
		playSound("chirp")
		self.chirp_count = randint(5000) + 5000
	end
end

function StillBird:new(o)
	o = Object:new(o)
	o.update = StillBird.update
	o.direction = randint(4)
	setObjectSubAnimation(o.number, o.direction)
	setObjectAnimated(o.number, false)
	o.change_count = randint(4000) + 1000
	o.chirp_count = randint(5000)
	setObjectHigh(o.number)
	return o;
end

FlyingBird = {}

function FlyingBird:fly()
	self.flying = true
	if (self.direction == 1) then
		self.direction = 2
	else
		self.direction = 1
	end
	if (self.direction == 1) then
		self.y = BUFFER_HEIGHT-self.height
		self.speed = -0.08
	else
		self.y = 0
		self.speed = 0.08
	end
	setObjectSubAnimation(self.number, self.direction)
	if (randint(2) == 1) then
		self.xinc = -(math.random()*0.04+0.02)
	else
		self.xinc = math.random()*0.04+0.02
	end
	self.x = (150-randint(300))+BUFFER_WIDTH/2
end

function FlyingBird:update(step)
	if (self.flying) then
		self.y = self.y + self.speed * step
		self.x = self.x + self.xinc * step
		if (self.y <= 0 or self.x <= 0 or self.y+self.height >= BUFFER_HEIGHT
			or self.x+self.width >= BUFFER_WIDTH) then
			self.flying = false
			self.wait = 10000
			setObjectHidden(self.number, true)
		end
		setObjectPosition(self.number, self.x, self.y)
	else
		self.wait = self.wait - step
		if (self.wait <= 0) then
			self:fly()
			setObjectPosition(self.number, self.x, self.y)
			setObjectHidden(self.number, false)
		end
	end
end

function FlyingBird:new(o)
	o = Object:new(o)
	o.update = FlyingBird.update
	o.fly = FlyingBird.fly
	if (o.direction == nil) then
		o.direction = randint(2)
	elseif (o.direction == 1) then
		o.direction = 2
	else
		o.direction = 1
	end
	setObjectSolid(o.number, false)
	setObjectHigh(o.number, true)
	o:fly()
	o:update(1)
	return o
end

Duck = {}

function Duck:update(step)
	self.x = self.x + self.speed * step
	setObjectPosition(self.number, self.x, self.y)
	if (self.direction == DIRECTION_EAST) then
		tx = (self.x + self.width) / TILE_SIZE
	else
		tx = self.x / TILE_SIZE
	end
	ty = (self.y + self.height/2) / TILE_SIZE
	if (not tileIsSolid(tx, ty)) then
		self.speed = - self.speed;
		if (self.direction == DIRECTION_EAST) then
			setObjectSubAnimation(self.number, 2)
			self.direction = DIRECTION_WEST
		else
			setObjectSubAnimation(self.number, 1)
			self.direction = DIRECTION_EAST
		end
	end
end

function Duck:new(o)
	o = Object:new(o)
	o.update = Duck.update
	if (o.direction == DIRECTION_WEST) then
		setObjectSubAnimation(o.number, 2)
		o.speed = -0.01
	else
		o.speed = 0.01
	end
	setObjectAnimated(o.number, false)
	setObjectSolid(o.number, false)
	return o;
end

