function OnInit() //run upon initialization
	this:SetImage("invisible", nil); //we don't want to be shown at all
	this:SetPersistent(false); //don't save us with the map
end

function OnPostInit() //run during the entity's first logic update

	local particle = GetEffectManager:GetParticleByName("explode");
	
	if (not particle) then
		//this means we need to create it, only happens once during the game - if you
		//change these settings, you will need to restart the engine! Or, make changes
		//to the existing "simple", it will modify all instances using it on the fly
		particle = GetEffectManager:CreateParticle("explode", "effects/media/explosion.png", 500);
		//particle:SetColor(Color(255,110,60,255));
		particle:SetColoring2(Color(255,209,50,100),Color(255,92,100,10), 0);
		particle:SetSizing2( 1.0, 2.5,0 );
		particle:GetMotionController():Set1DAcceleration(-0.0003);
		
	end
	
	m_pEffectExplosion = GetEffectManager:CreateEffectExplosion(16,4,5,0.3);
	m_pEffectExplosion:AddParticle(particle, C_PROBABILITY_DEFAULT);
	m_pEffectExplosion:SetLife(300);
	m_pEffectExplosion:SetLifeDistortion(700);
	m_pEffectExplosion:SetSizeDistortion(0.8);
	m_pEffectExplosion:SetRandomRotation(true);
	m_pEffectExplosion:SetSpeedDistortion(0.1); //specific to this effect
	this:AddEffect(m_pEffectExplosion);
	
	
	//delete this entity when the explosion is done
	Schedule(3500, this:GetID(), "this:SetDeleteFlag(true);");

end