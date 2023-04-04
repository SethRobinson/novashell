function OnInit()
	
	this:SetVisualProfile("~/slash.xml", "slash");
	this:LoadCollisionInfo("~/slash.col");
	this:SetPersistent(false); //don't bother saving this
	m_parent = nil;
	m_canHit = false;
	this:SetDensity(1); //One of the objects must have density for collisions to occur, so we'll use dampening to stop us from moving instead
	this:SetBlendMode(C_BLEND_MODE_ADDITIVE);
	this:SetBaseColor(Color(255,255,255,150)); //add some transparency
	this:SetCollisionCategories(C_CATEGORIES_NONE); //never touch anything
	this:SetGravityOverride(0); //so we don't move
	
end

function OnPostInit()

	//tell us to die soon
	Schedule(100, this:GetID(), "this:SetDeleteFlag(true);");
	if (GetPlayer != nil) then
		this:SetLayerID(GetPlayer:GetLayerID()+1);
	 end

end

function OnKill()

	
end

function SetParent(parent)

//this is an important part of initialization, it tells this weapon who swung it
	m_parent = parent;
	this:SetFacing(parent:GetFacing());
	GetSoundManager:Play("audio/effects/swing.ogg");
	this:SetSpriteByVisualStateAndFacing();	
	vScale = m_parent:GetScale();
	this:SetScale(vScale);

	//move it forward
	  this:SetPos(this:GetPos() +  ( FacingToVector(this:GetFacing()) * (50*vScale.x)) );
		this:SetCollisionListenCategory(C_CATEGORY_ENTITY, true);
		
		//because we might have scaled this entity, we should wait a frame so it will correctly collide
		//with everything
		Schedule(1, this:GetID(), "m_canHit = true;");
end

function OnAction(enemy)

	GetTextManager:Add("It's a slash.", enemy);
   
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	if (m_canHit == false or entity == nil) then
	  return;
	end
	
	if (entity:GetID() == m_parent:GetID()) then
		//don't want to hurt ourself
	else
		//LogMsg("Got hit by " .. tostring(ent:GetID()) .. " Depth: " .. tostring(depth) .. " normal: " .. tostring(normal));
	//hurt this entity
  		entity:RunFunctionIfExists("OnDamage", vNormal, depth, m_parent, 5, 0, this);
	end
	
		//by saying 1, we say run "on the next tick" as 1 MS is the smallest amount that can pass.
		Schedule(1, this:GetID(), "m_canHit = false;");

	return; //don't allow collision to happen

end


