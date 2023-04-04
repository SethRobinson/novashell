function OnInit()
	this:SetVisualProfile("~/slash.xml", "slash");
	this:LoadCollisionInfo("~/slash.col");
end

function OnPostInit()

	this:SetDensity(0);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
	this:SetCollisionCategories(C_CATEGORIES_NONE);
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

//we're only listening for the player, so we'll assume that's all we'll get..
function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)
	//stop listening for collisions
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, false);

	//Let's disappear, we got picked up
 	this:GetBrainManager():Add('FadeOutAndDelete','fade_speed_ms=100');
 
	GetSoundManager:Play("audio/effects/coin.ogg");
  GetTextManager:Add("[Slash weapon picked up, use with the CTRL key]", entity );
	entity:Data():Set("HasSlasher","true");
end
