
//easy way to create an explosion on the active map at a location
function CreateExplosion(vPos)

	local pEnt = CreateEntity(GetMapManager:GetActiveMap(), vPos, "effects/explosion.lua"); 

end