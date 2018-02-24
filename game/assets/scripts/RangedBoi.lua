--[[
FILE: RangedBoi.lua
PRIMARY AUTHOR: Brett Schiff

Copyright (c) 2017 DigiPen (USA) Corporation.
]]

-- 
PLAYER1LAYER = 1 << 7
PLAYER2LAYER = 1 << 8
PLAYER3LAYER = 1 << 9
PLAYER4LAYER = 1 << 10
GROUND_LAYER = 1 << 3

-- info about his vision
lookDirectionDegrees = 180
lookConeWidth = 45
lookConeLength = 5
lookNumRays = 5

-- info about shooting
numberOfShotsPerPattern = 3
timeBetweenShots = 1

timeBetweenFiringPatterns = 5

currTime = 0
currentlyInFiringPattern = false
shotsFired = 0

target = GameObject(0)

projectilePrefabName =  "RangedBoiProjectile.json"
projectileLaunchAngleDegrees = 45

function LookForGnomes()

	-- start downward and adjust according to the cone width
	local initialDirection = lookDirectionDegrees - (lookConeWidth / 2)
	local directionIncrease = lookConeWidth / (lookNumRays - 1)
	local endDirection = lookDirectionDegrees + (lookConeWidth / 2)

	-- raycast a number of times in the correct directions
	for currDirection = initialDirection, endDirection, directionIncrease
	do
		-- raycast from itself
		local cast = Raycast.RaycastAngle(this:GetSpaceIndex(), this:GetTransform().position, currDirection, lookConeLength, PLAYER1LAYER | PLAYER2LAYER | PLAYER3LAYER | PLAYER4LAYER | GROUND_LAYER)

		-- if a gnome was hit
		if(cast.gameObjectHit:IsValid() and cast.gameObjectHit:GetCollider().collisionLayer.layer ~= GROUND_LAYER)
		then

			print("Rangedboi: Rangedboi saw a gnome")
			currentlyInFiringPattern = true;
			currTime = timeBetweenShots
			shotsFired = 0
			return cast.gameObjectHit

		end

	end

	-- no gnome was found
	return GameObject(0)

end

function ShootAtTarget(target)
	
	-- spawn a projectile and get its info
	local projectile = GameObject.LoadPrefab("assets/prefabs/" .. projectilePrefabName)

	-- get the vector to the target
	targetPosition = target:GetTransform().position
	thisPosition = this:GetTransform().position
	vecToTarget = vec2(targetPosition.x - thisPosition.x, targetPosition.y - thisPosition.y)
	
	-- set the position of the projectile
	yPosition = thisPosition.y
	xPosition = thisPosition.x
	
	if(vecToTarget.x <= 0)
	then

		xPosition = xPosition - (this:GetCollider().dimensions.x / 2) - projectile:GetCollider().dimensions.x

	else
	
		xPosition = xPosition + (this:GetCollider().dimensions.x / 2) + projectile:GetCollider().dimensions.x

	end

	projectilePosition = vec2(xPosition, yPosition)
	projectile:GetTransform().position = projectilePosition

	-- set the velocity of the projectile
	projectile:GetRigidBody().velocity = vec3(vecToTarget.x, 2, 0)

end

function ShootAtCorrectTime(dt)

	-- if the enemy is between firing patterns
	if(currentlyInFiringPattern == false)
	then
		-- if the time has passed to go back into a firing pattern
		if(currTime >= timeBetweenFiringPatterns)
		then
			
			currentlyInFiringPattern = true
			shotsFired = 0
			currTime = currTime - timeBetweenFiringPatterns + timeBetweenShots
			return

		else -- continue waiting till the next firing pattern

			currTime = currTime + dt
			return

		end

	else -- the enemy is in a firing pattern

		if(currTime >= timeBetweenShots)
		then

			currTime = currTime - timeBetweenShots
			shotsFired = shotsFired + 1

			-- if the firing pattern has been completed, break
			if(shotsFired >= numberOfShotsPerPattern)
			then

				currentlyInFiringPattern = false
				shotsFired = 0

			end

		else -- continue waiting till the next shot

			currTime = currTime + dt
			return

		end
	end

	-- Raycast at the gnome to make sure it's still in vision
	directionTowardsTarget = vec2(- this:GetTransform().position.x + target:GetTransform().position.x, - this:GetTransform().position.y + target:GetTransform().position.y)

	theCast = Raycast.Cast(this:GetSpaceIndex(), this:GetTransform().position, directionTowardsTarget, lookConeLength, PLAYER1LAYER | PLAYER2LAYER | PLAYER3LAYER | PLAYER4LAYER | GROUND_LAYER)

	if(theCast.gameObjectHit:IsValid() and theCast.gameObjectHit:GetCollider().collisionLayer.layer ~= GROUND_LAYER)
	then
		
		ShootAtTarget(theCast.gameObjectHit)

	else

		currentlyInFiringPattern = false
		currTime = timeBetweenShots
		shotsFired = 0
		target = GameObject(0)

	end

end


-- Called at initialization
function Start()
	
	

end

-- Updates each frame
function Update(dt)
	
	-- if there is no valid target
	if(not target:IsValid())
	then

		-- look for gnomes
		target = LookForGnomes()

	else

		-- shoot at the target gnome
		ShootAtCorrectTime(dt)

	end
	

	

end

-- Other is a game object
function OnCollisionEnter(other)

	

end
