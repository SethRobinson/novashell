function OnMapInsert() //run as the entity is placed on an actual map, but before OnPostInit()
	GetWatchManager:Add(this, C_TIME_FOREVER); //always function offscreen
	this:SetDensity(1.8); //make smaller for more floaty
end

function OnInit() //run upon initialization

	//hint to the path-finding system that it can ignore this while computing paths if it needs to
	this:SetIsCreature(true); 
end

function OnPostInit() //run after being placed on a map

	this:SetCollisionCategory(C_CATEGORY_TILEPIC, false); //allow this to move through static tilepic things
	this:GetBrainManager():Add("StandardBase","");
	AddPatrolGoals();
	
end

function OnKill() //run when removed

end

function AddPatrolGoals()

	//let the platform move between the checkpoints
	
	this:GetGoalManager():AddMoveToPosition(GetTagManager:GetPosFromName("Ele1A"));
	this:GetGoalManager():AddMoveToPosition(GetTagManager:GetPosFromName("Ele1B"));
	//when complete, do it again
	this:GetGoalManager():AddRunScriptString("AddPatrolGoals()");

end
