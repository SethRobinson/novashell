function OnInit()

	//grab a random pic to use, although we won't show it in real use
	this:SetVisualProfile("system/system.xml", "ent_warp_tag");
	this:SetDensity(1);
	this:LoadCollisionInfo("~/top_damage.col");
	this:SetPersistent(false); //don't bother saving this
	//	this:SetDensity(0); //makes it immovable, speeds up processing too
  this:SetCategory(C_CATEGORY_DAMAGE, true);
  this:SetCategory(C_CATEGORY_ENTITY, false);
  this:SetCollisionCategories(C_CATEGORIES_NONE);
	m_canHit = false;
	m_parent = nil;
	m_weaponScaleMod = 1.0;
	m_distanceFromBody = 40;
	m_strength = 5; //how much we hit for
	
end

function SetStrength(damage)
	m_strength = damage;
end
function SetWeaponScaleMod(scale)

 m_weaponScaleMod = scale;
end

function SetDistanceFromBody(dist)

	m_distanceFromBody = dist;
end

function OnPostInit()

	//tell us to die soon
	Schedule(100, this:GetID(), "this:SetDeleteFlag(true);");
	
	//make it invisible.  To debug, disable this line so you can see the icon
	this:SetBaseColor(Color(255,255,255,0));
end

function SetParent(parent)

//this is an important part of initialization, it tells this weapon who swung it
	m_parent = parent;
	this:SetFacing(parent:GetFacing());
	vScale = m_parent:GetScale();
	this:SetScale( Vector2(vScale.x * m_weaponScaleMod, vScale.y *m_weaponScaleMod));

	//move it forward
	this:SetPos(this:GetPos() +  ( FacingToVector(this:GetFacing()) * (m_distanceFromBody*vScale.x)) );
	this:SetCollisionListenCategories(C_CATEGORIES_ALL);
	this:SetCollisionListenCategory(C_CATEGORY_TILEPIC, false);
	//because we might have scaled this entity, we should wait a frame so it will correctly collide
	//with everything
	
	Schedule(1, this:GetID(), "m_canHit = true;");
	
end

function OnAction(enemy)
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)
//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	if (m_canHit == false) then
	  return;
	end
	
		//LogMsg("Hit Entity " .. tostring(entity:GetID()) .. " : Depth: " .. tostring(depth) .. " normal: " .. tostring(vNormal));

	if (entity:GetID() == m_parent:GetID()) then
	
		//don't want to hurt ourself
	else
		//notify this entity we just hurt the heck out of him, if he has an OnDamage function, anyway
  		//LogMsg("hitting " .. ent:GetName() .. " for " .. m_strength);
  		entity:RunFunctionIfExists("OnDamage", vNormal, depth, m_parent, m_strength, 0, this);
	end
	
		Schedule(1, this:GetID(), "m_canHit = false;");

end
