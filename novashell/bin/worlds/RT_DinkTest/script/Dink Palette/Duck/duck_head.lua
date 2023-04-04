//just the head part

function OnInit()

	this:SetVisualProfile("~/duck_headless.xml", "duck_head");
	this:LoadCollisionInfo("~/head.col");
	//we need collision data for movement to work, and we want the head to stop when it
	//hits buildings, but we want it to fly through entities and not be pushable by them
	this:SetCollisionCategories(C_CATEGORIES_NONE);
	this:SetCollisionCategory(C_CATEGORY_TILEPIC, true); //only bump against walls
	this:SetDampening(0.04); //so we don't slide everywhere

	m_doFlyAnim = false;
	
end

function OnPostInit()

	this:SetSpriteByVisualStateAndFacing();
	//by default, let's put it on the last frame, assuming we're just sitting on the ground
	if (m_doFlyAnim == false) then
		this:SetAnimFrame(C_ANIM_FRAME_LAST);	
	end

end

function DoFlyAnim()

	m_doFlyAnim = true;
	
end