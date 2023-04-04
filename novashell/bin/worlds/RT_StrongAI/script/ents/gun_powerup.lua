
function OnInit() //run upon initialization

	
end

function OnPostInit() //run during the entity's first logic update
		this:SetCollisionCategories(C_CATEGORIES_NONE);
		this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
		this:SetCategory(C_CATEGORY_ENTITY, false); //clear us of being in the entity category
		this:SetCategory(C_CATEGORY_POWER_UP, true);
		this:SetLayerByName("Main");
	
end

function OnKill() //run when removed

end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)
//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)
	
	this:SetCollisionListenCategories(C_CATEGORIES_NONE);
	
	//hide us
	this:SetLayerByName("Hidden Data");
	entity:RunFunction("OnGunUpgrade");
	this:PlaySound("audio/powerup.ogg");
	return;
end

