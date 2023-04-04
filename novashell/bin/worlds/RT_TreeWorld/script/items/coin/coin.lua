function OnInit()
	
	this:SetVisualProfile("~/coin.xml", "ent_coin");
	this:LoadCollisionInfo("~/coin.col");
	this:SetEnableRotationPhysics(false);
 	m_bPickedup = false;
 	this:SetCollisionCategory(C_CATEGORY_PLAYER, false); //don't bump into the player, slows him down annoyingly
 	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true); //but we do want our OnCollision to get called if he touches us

end

function OnMapInsert()

	if (Exists("g_totalCoinsPlaced") == false) then
		LogMsg("Initting coins..");
		_G.g_totalCoinsPlaced = 0;
	end
	
	_G.g_totalCoinsPlaced = _G.g_totalCoinsPlaced + 1;
	//LogMsg("Total coins placed is " .. g_totalCoinsPlaced);
end

function OnPostInit()

end

function OnAction(enemy)

	GetTextManager:Add("It's money.", enemy);

end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	if (m_bPickedup == true) then
		//already picked up
		return;
	end
	
	m_bPickedup = true;
	//Let's disappear, we got picked up
	this:GetBrainManager():Add('FadeOutAndDelete','fade_speed_ms=50');
	GetSoundManager:Play("audio/effects/coin.ogg");
	entity:RunFunction("OnPickedUpCoin");
	return false; //don't allow collision to happen

end

