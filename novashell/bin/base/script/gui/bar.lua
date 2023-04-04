//Simple script to emulator a power bar or progress bar and handle smoothly animated the var when its values are changed.

function OnInit() //run upon initialization
		
	//load the interface BG
	if (not this:Data():Exists("_TilePicID")) then
		this:SetImage("system_images", nil); //just a junk image to use so the engine won't complain about no visual being set
	end
	this:SetAlignment(C_ORIGIN_TOP_LEFT, Vector2(0,0));
	this:SetPriorityLevel(C_PRIORITY_LEVEL_HIGH); //so it updates even while dialogs are active
end

function OnPostInit() //run during the entity's first logic update
	
	this:SetLayerByName("GUI 2"); //default GUI level
	this:SetRunUpdateEveryFrame(true);
	m_animSpeed = 0.02; //a smaller number means slower visual interpolation
	m_visualNum = 0;
end

function GetNum()
	return m_num;
end

function SetNum(value)
	m_num = value;
	if (m_num < 0) then
		m_num = 0;
	end
	m_bRequestUpdate = true;
end


function SetVisualNum(value)
	m_visualNum = value;
	m_bRequestUpdate = true;
end

function Setup(barType, imageName, value, maxValue, barAlign)

//don't save us, we'd rather be rebuilt each time
	this:SetPersistent(false);


	if (barType != "Horizontal") then
		LogError("Only support type Horizontal right now.  Add your own vertical!");
	end

	if (imageName != "") then
		this:SetImage(imageName, nil);
	end
	
	m_originalRect = this:GetImageClipRect(); //remember this for later
	m_num = value;
	m_visualNum = 0;
	m_maxNum = maxValue;
	m_bRequestUpdate = true;
	
		//check to see whether the developer has aligned the bar
		if (this:VariableExists("barAlign") == true) then		
			this:SetAlignment(barAlign, Vector2(0,0));
		else
			this:SetAlignment(C_ORIGIN_TOP_LEFT, Vector2(0,0));
		end
end


//this is run every logic tick
function Update(step)
	
	if (this:VariableExists("m_num") == false) then
		return; //we're not ready for this, haven't been setup yet.
	end
	
	if (m_num != m_visualNum or m_bRequestUpdate) then
		m_visualNum = Lerp(m_visualNum, m_num, m_animSpeed*step);
		m_bRequestUpdate = false;
		local percent = m_visualNum/m_maxNum;
		
		//LogMsg("Percent is " .. percent);
		
		//modify the image displayed
		local r = Rect(m_originalRect);
		r.right = m_originalRect.right * percent;
		//LogMsg(m_originalRect);
		this:SetImageByID(this:GetImageID(), r);

	end

end

