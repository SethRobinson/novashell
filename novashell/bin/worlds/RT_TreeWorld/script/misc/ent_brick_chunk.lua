//simple thing so brick chunks will fly around but through the player to not hamper his movement

function OnInit()
	this:SetVisualProfile("~/misc.xml", "ent_brick_chunk");
	this:SetDensity(0.2);
	this:LoadCollisionInfo("~/ent_brick_chunk.col");
	this:SetEnableRotationPhysics(true);
	this:SetCollisionCategory(C_CATEGORY_PLAYER, false);
end
