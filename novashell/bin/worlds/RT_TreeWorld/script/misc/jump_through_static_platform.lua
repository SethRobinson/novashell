//Currently broken, can't jump through.  need to figure out how to make jump through platforms in the new engine..

//Assign this to any tilepic that you want to be a static platform yet won't cause the player
//to bump his head on it from below (he can jump up to it through it)


function OnInit() //run upon initialization
	this:SetDensity(0); //makes it immovable, speeds up processing too

end

function OnPostInit() //run during the entity's first logic update
	this:SetType(C_TYPE_PLATFORM); //so the player script can detect we're a platform and
	//not an entity, this way collisions can be ignored while on a ladder
	//this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true); //more control in our OnCollision
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	//LogMsg("Hit Static: Depth: " .. tostring(depth) .. " vNormal: " .. tostring(vNormal));
	if (entity:GetLinearVelocity().y >  0 ) then
		//it's the player and he's moving down, so yeah, let's be 'hard'.
		this:SetCollisionCategory(C_CATEGORY_PLAYER, true); //player collides with us
	else
		this:SetCollisionCategory(C_CATEGORY_PLAYER, false); //player falls through u
	end
	
end

function OnKill() //run when removed
end

