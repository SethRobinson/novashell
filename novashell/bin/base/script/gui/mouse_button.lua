m_bIsInsideRect = false;

function OnInit() //run upon initialization

	GetInputManager:AddBinding("mouse_left", "OnMouseLeft", this:GetID());
	this:SetRunUpdateEveryFrame(true);
	this:Data():SetIfNull("OnClick", 'LogMsg("Clicked!");');
end

function OnPostInit() //run during the entity's first logic update

end

//this is run every logic tick
function Update(step)

local pt = ScreenToWorld(GetInputManager:GetMousePos());
	r = this:GetWorldRect();
	if (r:IsInside(pt)) then
		
		if (m_bIsInsideRect  == false) then
			m_bIsInsideRect = true;
			OnEnteredRect();
		end
		
	else
	  if (m_bIsInsideRect ) then
		  m_bIsInsideRect = false;
		  OnLeftRect();
	  end
	end

	if (m_bIsInsideRect) then

		//how about a little rotating effect?
		local rot = math.sin(GetApp:GetGameTick()/300)/10;
		this:SetRotation(Lerp(this:GetRotation(), rot, 0.001*step));

	else
	local rot = this:GetRotation();
	
	if (rot != 0) then
	
	//slowly move it back to the normal position
	this:SetRotation(Lerp(rot, 0, 0.001*step));
	end
	
	end

end

function OnEnteredRect()
	//this:GetBrainManager():Remove("Scale"); //make sure a pending scale isn't happening
	this:GetBrainManager():Add("Scale", "scalex=1.1;scaley=1.1;scale_speed_ms=500;");
end

function OnLeftRect()
	this:GetBrainManager():Add("Scale", "scalex=1.0;scaley=1.0;scale_speed_ms=500;");
	
end

function OnMouseLeft(bKeyDown)
	
	//we only care if they pressed down the button, not if they are releasing it
	if (bKeyDown == false) then return true; end;
		if (m_bIsInsideRect) then
				ScheduleSystem(0, C_ENTITY_NONE, this:Data():Get("OnClick"));

		return false; //Don't let any other callbacks be called
		end

	return true; //Let other callbacks get called
end