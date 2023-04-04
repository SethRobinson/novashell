function OnInit()
	
	this:SetVisualProfile("misc/misc.xml", "ent_brick");
	this:LoadCollisionInfo("~/ent_brick.col");
	this:SetDensity(0); //makes it immovable, speeds up processing too
	this:SetEnableRotationPhysics(false);
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true);
	
end

function OnPostInit()
end

function AddDebris(vPos, vForce)

	ent = CreateEntity(this:GetMap(), this:GetPos()+vPos, "misc/ent_brick_chunk.lua");
	ent:AddForce(vForce);
	ent:SetPersistent(false);
 	Schedule(800, ent:GetID(), "this:GetBrainManager():Add(\"FadeOutAndDelete\",'');");

end

function OnDamage(normal, depth, enemy, damage, userVar, projectile)
	Crumble();
 end

function Crumble()

	//Bust up this brick
	this:SetDeleteFlag(true);
	GetSoundManager:Play("audio/mario/smash.wav");
	
	//create the particles that fly off
	AddDebris(Vector2(0,0), Vector2(-2,-2));
	AddDebris(Vector2(20,0), Vector2(2,-2));
	AddDebris(Vector2(0,20), Vector2(-1,-2));
	AddDebris(Vector2(20,20), Vector2(1,-2));

end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)

//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)


	//LogMsg("Brick: Hit Static: Depth: " .. tostring(depth) .. " normal: " .. tostring(vNormal));
	
	if (vNormal.y < -0.1 and depth > 0) then
		Crumble();
	end

end

