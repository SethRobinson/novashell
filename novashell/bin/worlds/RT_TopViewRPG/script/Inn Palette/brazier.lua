
function OnInit() //run upon initialization
	this:Data():SetIfNull("OnActionText", "I don't know what this is.");
	this:Data():SetIfNull("OnDamageText", "");
end

function OnPostInit() //run during the entity's first logic update


	local particle = GetEffectManager:GetParticleByName("smoke_brazier");
	
	if (not particle) then
		//this means we need to create it, only happens once during the game - if you
		//change these settings, you will need to restart the engine! Or, make changes
		//to the existing "simple", it will modify all instances using it on the fly
		particle = GetEffectManager:CreateParticle("smoke_brazier", "effects/media/small.png", 1000);
		particle:SetColoring2(Color(255,209,50,100),Color(255,92,100,10), 0);

		//particle:SetColor(Color(255,110,60,255));
		//particle:SetSizing2( 1.0, 2.5,0 );
		//particle:GetMotionController():Set1DAcceleration(-0.003);
		//particle:GetMotionController():Set1DAcceleration(0.001);
	end
	
	m_pEffectExplosion = GetEffectManager:CreateEffectExplosion(200,10,10,0.002);
	
	m_pEffectExplosion:SetOffset(Vector2(-1,-26));
	m_pEffectExplosion:SetAdditionalVector(Vector2(0,-0.01));
	m_pEffectExplosion:AddParticle(particle, C_PROBABILITY_DEFAULT);
	//m_pEffectExplosion:SetLife(300);
	//m_pEffectExplosion:SetLifeDistortion(700);
	//m_pEffectExplosion:SetSizeDistortion(0.01);
	m_pEffectExplosion:SetRandomRotation(true);
	m_pEffectExplosion:SetSpeedDistortion(0.01); //specific to this effect
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