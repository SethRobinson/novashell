function OnInit()
	
	this:SetVisualProfile("~/top_coin.xml", "top_coin");
	this:LoadCollisionInfo("~/top_coin.col");
	
	this:SetCollisionCategory(C_CATEGORY_PLAYER, false); //let the player walk through us
 this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
 	m_bPickedup = false;
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
  
	money = entity:Data():ModNum("Money", 1);
	entity:RunFunction("UpdateGUI");
	//GetTextManager:Add("I picked up a coin. (" .. tostring(money) .. ")", entity );
	
end

