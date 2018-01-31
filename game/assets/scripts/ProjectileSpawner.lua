--[[
FILE: ProjectileSpawner.lua
PRIMARY AUTHOR: Max Rauffer

Copyright (c) 2017 DigiPen (USA) Corporation.
]]

-- Spawning settings
relativeFireDirection = true
aimDirection = vec2(1,0)

relativeOffset = true

fireSpeed = 10

parented = false -- TODO. Wait til parenting via script works properly

-- Local variables
local direction = vec2(1,0)	-- Left = -1    Right = 1
local timer = 0
local canFire = true

function Update(dt)
	if(canFire == false)
	then
		timer = timer - dt
		if(timer <= 0)
		then
			canFire = true
		end
	end
end

function SetDir(dir)
	direction = dir
end

function SetAim(aim)
	aimDirection = aim
end

function Fire(PrefabName)
	if (canFire == false) then return end

	local proj = GameObject.LoadPrefab("assets/prefabs/" .. PrefabName)
	
	local info = proj:GetScript("ProjectileInfo.lua")

	-- Set offset
	off = vec2(info.offsetX, info.offsetY)

	if(relativeOffset)
	then
		off = vec2(off.x * direction.x, off.y * direction.y)
	end

	proj:GetTransform().position = vec2(this:GetTransform().position.x + off.x, this:GetTransform().position.y + off.y)

	-- Set Speed
	local dir = aimDirection

	proj:GetRigidBody().velocity = vec3(dir.x * info.speed, dir.y * info.speed, 0)

	info.StartLifeTimer()

	canFire = false
	timer = info.cooldownTime
end
