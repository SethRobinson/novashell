//A thingie to show a slide show, like a sequence of splash screens.


m_slideArray= {};
m_timePerSlideMS = 4000;
m_fadeOutTimeMS = 1000;
m_fadeInTimeMS = 1000;
m_curSlide = 0;

function OnInit() //before the entity is placed in the world, init visuals here, so it knows the size dimensions
	this:SetPersistent(false); //don't save us with the map
	LogMsg("Initting slideshow object");
	this:SetImage("lua_script.png", nil); //the image we'll show so we can be seen in  the editor, dunno why though
	//this:SetAttach(C_ENTITY_CAMERA, Vector2(0,0); //so we are always on screen and processed	
end

function OnPostInit() //run during the entity's first logic update
	this:SetLayerByName("Hidden Data"); //we can only see the image when using the editor
end

function AddSlide(fileName)

	local slide = {};
	slide.fileName = fileName;
	
	//I know this seems like a waste, but I set it up this way to easily add per-slide display times later, by simply
	//overriding AddSlide with a version that has more parms
	
	slide.timeToShowMS = m_timePerSlideMS;
	slide.fadeInTimeMS = m_fadeInTimeMS;
	slide.fadeOutTimeMS = m_fadeOutTimeMS;
	slide.entID = C_ENTITY_NONE;
	
	table.insert(m_slideArray, slide);
	
end

function Init(bLooping, bSkippable)
	m_bLooping = bLooping;
	m_bSkippable = bSkippable;
		if (m_bSkippable == true) then
		LogError("Slideshow script: Skippable functionality not added yet.  Need to listen for keys/joysticks and such");		
		end
		
	ShowNextSlide();	
end

function ShowNextSlide()
	
		LogMsg("Showing next slide");

	if (#m_slideArray == 0) then
		LogError("You should use AddSlide() before calling init on this SlideShow entity, otherwise we've got nothing to show.");
		return;
	end

	
	if (m_bLooping and #m_slideArray == 1 and m_curSlide == 1) then
		//how do we loop a single slide?  just show it forever I guess
		return;
	end
	
	//fade out old image?
	
	if (m_curSlide != 0 and m_slideArray[m_curSlide].entID != C_ENTITY_NONE) then
		local pOldSlide = GetEntityByID(m_slideArray[m_curSlide].entID);
		if (pOldSlide != nil) then
			pOldSlide:GetBrainManager():Add("FadeOutAndDelete", "fade_speed_ms=" .. m_slideArray[m_curSlide].fadeOutTimeMS);
		end
			m_slideArray[m_curSlide].entID = C_ENTITY_NONE;
	end

		m_curSlide = m_curSlide + 1;
	
	if (m_curSlide > #m_slideArray) then
	 
	  if (m_bLooping == false) then
	  	//I guess we're done
	  	return;
		  end		

		//wrap to the start of the slides
	  m_curSlide = 1;

	
	end	
		//show new image

	pEnt = CreateEntity(GetMapManager:GetActiveMap(), GetApp:GetScreenSize()/2, "");
	
	pEnt:GetBrainManager():Add("FadeIn", "fade_speed_ms=" .. m_slideArray[m_curSlide].fadeInTimeMS);
	pEnt:SetDensity(0); //kills physics
	pEnt:SetImage(m_slideArray[m_curSlide].fileName, nil);
	pEnt:SetPersistent(false);
	pEnt:SetLayerByName("Main");
	m_slideArray[m_curSlide].entID = pEnt:GetID(); //remember so we can delete it later
	
	Schedule(m_timePerSlideMS, this:GetID(), "ShowNextSlide();");
	
end

function OnKill()  //run when removed

		//cleanup by deleting any slides we're currently showing
		
		for i,slide in ipairs(m_slideArray) do
			
			LogMsg("Killing image", i);
			DeleteEntityByIDIfExists(m_slideArray[i].entID);
 		  m_slideArray[i].entID = C_ENTITY_NONE;
			
		end

end
