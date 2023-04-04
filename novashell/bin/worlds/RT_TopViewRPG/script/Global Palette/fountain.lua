
function OnInit() //run upon initialization
	this:Data():SetIfNull("OnActionText", "It's a refreshing fountain.");
	this:Data():SetIfNull("OnDamageText", "");
end

function OnPostInit() //run during the entity's first logic update

	local particle = GetEffectManager:GetParticleByName("waterfall_fountain");
	
	if (not particle) then
		//this means we need to create it, only happens once during the game - if you
		//change these settings, you will need to restart the engine! Or, make changes
		//to the existing "simple", it will modify all instances using it on the fly
		particle = GetEffectManager:CreateParticle("waterfall_fountain", "effects/media/small.png", 1800);
		//particle:SetColor(Color(255,110,60,255));
		//particle:GetMotionController():Set1DAcceleration(-0.0001);
		particle:GetMotionController():Set2DAcceleration(Vector2(0, 0.0001));
		//particle:SetRotationEveryCycle(0.0001,0);
		particle:SetBlendMode(C_BLEND_MODE_NORMAL);
		particle:SetColor(Color(255,255,255,80)); //set the starting color
		particle:SetColoring2(Color(255,255,255,80),Color(255,255,255,0), 0.97); //setup a fade out to happen near the end
	  particle:SetSizing2( 1.0, 1.0, 0.97 ); //get big right at the end
	
	end

	m_pEffectExplosion = GetEffectManager:CreateEffectExplosion(10,2,3,0.002);
	
	m_pEffectExplosion:SetOffset(Vector2(3,-30));
	m_pEffectExplosion:SetAdditionalVector(Vector2(0,-0.07));
	m_pEffectExplosion:AddParticle(particle, C_PROBABILITY_DEFAULT);
	//m_pEffectExplosion:SetLife(500); //this would limit this effect to the beginning only
	m_pEffectExplosion:SetLifeDistortion(0.01);
	m_pEffectExplosion:SetSizeDistortion(0.1);
	m_pEffectExplosion:SetRandomRotation(false);
	m_pEffectExplosion:SetSpeedDistortion(0.017); //specific to this effect
	this:AddEffect(m_pEffectExplosion);
	
end

function OnAction(Ent)
	GetTextManager:Add(this:Data():Get("OnActionText"), Ent);
end

//entParent is whoever sent the damage, entObject is the actual damaging object/projectile if not nil
function OnDamage(normal, depth, entParent, strength, extra, entObject)

	if (entParent:GetID() != g_PlayerID) then
		//it ain't the players weapon/spell, just ignore it
		return;
	end

	if ( string.len(this:Data():Get("OnDamageText")) > 0) then
		//we have some text to show
		GetTextManager:Add(this:Data():Get("OnDamageText"), entParent);
	end
end