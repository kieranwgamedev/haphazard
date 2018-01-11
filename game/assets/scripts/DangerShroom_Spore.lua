--[[
FILE: DangerShroom_Spore.lua
PRIMARY AUTHOR: Noah Sweet

Copyright (c) 2017 DigiPen (USA) Corporation.
]]

Lifetime = 3 -- How long they last 'coz damn they last long

function Start()
end -- fn end


function Update(dt)
  Lifetime = Lifetime - dt
  
  -- Timer runs out; destroy
  if (Lifetime <= 0)
  then
    this:Destroy()
  end
end -- fn end


function OnCollisionEnter(object)
	this:Destroy();
end -- fn end

