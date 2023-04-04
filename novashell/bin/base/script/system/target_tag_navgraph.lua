//like a normal target tag but also registers as a navgraph node, allowing it to be a destination for
//navgraph comapatible warps/doors.

function OnInit()

	m_targetOnly = true; //other warps can query us to see if they have to worry about
	//pushing the user off it before warping them so they won't instantly warp back
	
	//any entity can be a warp target, but this script gives it a nifty icon.  You must
	//set its NAME to the target it should be.
	this:SetVisualProfile("~/system.xml", "ent_warp_tag");
	this:SetDensity(0); //makes it immovable, speeds up processing too
	
	this:SetNavNodeType(C_NODE_TYPE_WARP);
	this:SetHasPathNode(true);
end

function IsAWarp()
 return false;
end

function OnMapInsert() //run after OnInit, but before PostInit

	GetTagManager:RegisterAsWarp(this, "none");  //register us without setting a target, as we are the target of a one way warp
end


function OnGoalPreWarp(ent)
	
	  ent:PlaySoundPositioned("audio/effects/door_open.ogg");

end

function OnGoalPostWarp(ent)
		Schedule(200, ent:GetID(), 'this:PlaySoundPositioned("audio/effects/door_close.ogg");');  
end