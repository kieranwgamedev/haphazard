--[[
FILE: GnomeHealth.lua
PRIMARY AUTHOR: Max Rauffer

Copyright (c) 2017 DigiPen (USA) Corporation.
]]

ENEMY_LAYER = 16;

health = 6
healthBar = nil
invulTime = 0

INVULNERABLE_TIME = 2

LEVEL = "Level1.json"

function Start()

  healthBar = GameObject.FindByName(this:GetName().."Healthbar")

  local th = healthBar:GetSprite().textureHandler
  th.currentFrame = health
  healthBar:GetSprite().textureHandler = th

end

function Update(dt)
  
  if(invulTime > 0)
  then
    invulTime = invulTime - dt
  end
  
end

function OnCollisionEnter(other)

  if(invulTime <= 0 and this:GetDynamicCollider().colliderData:IsCollidingWithLayer(16))
  then
    health = health - 1

	local th = healthBar:GetSprite().textureHandler
	th.currentFrame = health
	healthBar:GetSprite().textureHandler = th

	invulTime = INVULNERABLE_TIME

	if(health <= 0)
	then
		if(this:GetName() == "Player1" and not GameObject.FindByName("Player2"):IsActive())
		then
			Engine.LoadLevel(LEVEL)
		elseif(this:GetName() == "Player2" and not GameObject.FindByName("Player1"):IsActive())
		then
			Engine.LoadLevel(LEVEL)
		end

		this:Deactivate();
	end
  end

end
