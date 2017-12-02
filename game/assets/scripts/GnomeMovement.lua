--[[
FILE: GnomeMovement.lua
PRIMARY AUTHOR: Lya Vera

Copyright (c) 2017 DigiPen (USA) Corporation.
]]

-- Variables
moveSpeed   = 2
jumpSpeed   = 4.5
fallSpeed   = 2

tossSpeed   = 5

stackHeight = 0.5 -- Move to start function (set using gnome size)
stackTimer  = 0   -- Timer until player can stack again
stackParent = nil -- Player at the bottom of the stack for stacked update

lastPos     = nil -- Last position (for reviving); Vector type?
moveDir     = 0

-- Bools
jumpEnabled  = false
moveEnabled  = true
stackEnabled = false
tossEnabled  = false
onGround     = true

-- Enums
PLAYER_LAYER  =  4
PLAYER_NUM    = -1 -- Set temporarily as invalid number
STACK_TIME    =  1

-- Move directions
MOVE_LEFT  = -1 -- Player moving left
MOVE_IDLE  =  0 -- Player idle
MOVE_RIGHT =  1 -- Player moving right

DEADZONE = 0.5 -- Joystick dead zone

-- Gamepad Buttons found under KeyMap.h
JUMP   = 0 -- A
ATTACK = 1 -- B
TOSS   = 3 -- Y

HORIZONTAL_AXIS = 0

function UpdateMovement(dt)
  HandleTimer(dt)

  -- Connections
  local playerBody = this:GetRigidBody()
  local playerTransform = this:GetTransform()

  newVelocity = playerBody.velocity
  acceleration = playerBody.acceleration

  -- Calculate x velocity
  newVelocity.x = moveDir * moveSpeed

  -- Calculate y valocity
  if (jumpEnabled == true and onGround == true)
  then
    newVelocity.y = jumpSpeed
    jumpEnabled = false
    onGround = false
  end

  -- TODO: using dt to calculate y velocity doesn't work :<

  -- Update player velocity
  playerBody.velocity = newVelocity
end -- fn end

function HandleTimer(dt)
  -- Decrement timer
  if (stackTimer > 0)
  then
    stackTimer = stackTimer - dt
    -- Timer reaches 0, reset
    if (stackTimer <= 0)
    then
      stackTimer = 0
      -- Allow players to collide
      SetLayersColliding(PLAYER_LAYER, PLAYER_LAYER)
    end
  end
end -- fn end

-- More of Kieran's code
function StackedUpdate(dt)
  -- Update position based on parent
  local playerPos = this:GetTransform().position
  local otherPos = stackParent:GetTransform().position
  playerPos.y = otherPos.y + stackHeight
  playerPos.x = otherPos.x
  this:GetTransform().position = playerPos

  -- Update velocity based on parent
  local playerVelocity = this:GetRigidBody().velocity
  local otherVelocity = stackParent:GetRigidBody().velocity
  playerVelocity = otherVelocity

  -- Player jumps off
  if (jumpEnabled)
  then
    stackEnabled = false
    playerVelocity.y = jumpSpeed
    stackTimer = STACK_TIME -- Set timer
  end

  -- Update velocity
  this:GetRigidBody().velocity = playerVelocity
end -- fn end

function TossUpdate(dt)



end -- fn end

-- Updates each frame
function Update(dt)
  -- Can we add playerID's to the player objects? :<

  -- TEMP move to start function
  -- Determine player once (which is why initial value is -1)
  if (PLAYER_NUM < 0)
  then
    local name = this:GetName()
    SetKeyboardControls(name)
  end

  -- Retrieve input
  if (GamepadsConnected() > 0)
  then
    print("Using gamepads")
    GetInputGamepad()
  else
    GetInputKeyboard()
  end

  -- Player is stacked
  if (stackEnabled)
  then
    StackedUpdate(dt)
  -- Update regular player movement
  else
    UpdateMovement(dt)
  end
end -- fn end

-- Other is a game object
function OnCollisionEnter(other)
  -- TODO: Change checks from name to tags

  -- Get name (for onGround)
  local otherName = other:GetName()

  -- Player collides with ground
  if (other:HasTag("Ground"))
  then
    onGround = true
  -- Player collides with other player
  elseif(onGround == false)
  then
    if (other:HasTag("Player") and stackEnabled == false)
    then
      StackPlayers(other)
    end
  -- Player collides with a coin
  elseif(other:HasTag("Coin"))
  then
    -- TODO: Play a coin pickup effect
    -- Switch to coin script
    -- Destroy coin
    -- GameObject:GetScript(filename of script)

  end
end -- fn end

-- Kieran's stack code
function StackPlayers(other)
  -- Get transforms
  local playerTransform = this:GetTransform()
  local otherTransform = other:GetTransform()
    
  -- Get positions
  local playerPos = playerTransform.position
  local otherPos = otherTransform.position

  local snapDistance = 1 -- horizontal distance from other gnome
  local xDistance = playerPos.x -- x-axis distance between players

  -- How do I get the absolute value? Hmmmm
  if (playerPos.x > otherPos.x)
  then
    xDistance = playerPos.x - otherPos.x
  else
    xDistance = otherPos.x - playerPos.x
  end

  -- TODO: Change to use raycast downwards to check for gnome collision
  if (playerPos.y > otherPos.y)
  then
    -- Players are stacked
    stackEnabled = true

    -- Set bottom player as parent
    stackParent = other

    -- Don't detect collision between these layers
    SetLayersNotColliding(PLAYER_LAYER, PLAYER_LAYER)

    -- TEMP SOLUTION: make sure gnome is drawn in front
    --if (playerTransform.zLayer < otherTransform.zLayer)
    --then
    --  playerTransform.zLayer = otherTransform.zLayer - 1
    --end
  end
end -- fn end

-- TEMP
function SetKeyboardControls(name)
  if (name == "Player1")
  then
    PLAYER_NUM = 0

    -- TEMPORARY
    KEY_JUMP  = 87 -- W
    KEY_DOWN  = 83 -- S
    KEY_LEFT  = 65 -- A
    KEY_RIGHT = 68 -- D
    KEY_TOSS  = 84 -- T

  else
    PLAYER_NUM = 1

    -- TEMPORARY
		KEY_JUMP  = 265 -- Up
		KEY_DOWN  = 264 -- Down
		KEY_LEFT  = 263 -- Left
		KEY_RIGHT = 262 -- Right
    KEY_TOSS  = 334 -- Numpad_Add
  end
end -- fn end

-- Gamepad input
function GetInputGamepad()
  -- Player moves right
  if (GamepadGetAxis(PLAYER_NUM, HORIZONTAL_AXIS) > DEADZONE)
  then
    moveDir = MOVE_RIGHT
  -- Player moves left
  elseif (GamepadGetAxis(PLAYER_NUM, HORIZONTAL_AXIS) < -DEADZONE)
  then
    moveDir = MOVE_LEFT
  -- Player does not move
  else
    moveDir = MOVE_IDLE
  end

  -- Player jumps
  if (GamepadIsPressed(PLAYER_NUM, JUMP))
  then
    jumpEnabled = true
  else
    jumpEnabled = false
  end

  -- Player tosses
  if (GamepadIsPressed(PLAYER_NUM, TOSS))
  then
    tossEnabled = true
  else
    tossEnabled = false
  end
end -- fn end

-- Keyboard input
function GetInputKeyboard()
  -- Player moves right
  if (IsPressed(KEY_RIGHT))
  then
    moveDir = MOVE_RIGHT
  -- Player moves left
  elseif (IsPressed(KEY_LEFT))
  then
    moveDir = MOVE_LEFT
  -- Player does not move
  else
    moveDir = MOVE_IDLE
  end

  -- Player jumps
  if (IsPressed(KEY_JUMP))
  then
    jumpEnabled = true
  else
    jumpEnabled = false
  end

  -- Player tosses (and is not stacked on other player)
  if (IsPressed(KEY_TOSS) and stackEnabled == false)
  then
    tossEnabled = true
  else
    tossEnabled = false
  end
end -- fn end