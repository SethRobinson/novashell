	
function OnInit()
	
	bInitted = false;

	this:SetVisualProfile("~/../../woman/woman.xml", "woman");
	this:LoadCollisionInfo("~/../../woman/woman.col");
	this:SetDensity(1); 
	this:SetVisibilityNotifications(true); //OnGetVisible and OnLoseVisible will get called now
	this:SetCategory(C_CATEGORY_CHARACTER, true);
	this:SetCategory(C_CATEGORY_ENTITY, false);
end

function OnPostInit()

	AttachPointer(this, -20); //in rpg_utils.lua
	ActivateIfNeeded();

end

function ActivateIfNeeded()

	//insure this entity will keep moving even if this was called while it was offscreen
	GetWatchManager:Add(this, 1000*40);
 
	if (bInitted) then return; end;
	
	bInitted = true; //don't run this again	

	m_speechTimer = 0;
		
	this:GetBrainManager():Add("StandardBase","");
	
	if (GameIsActive() == false) then
		//no profile is active, so we must be in the editor.  Let's not have them running around
		this:GetBrainManager():SetStateByName("Frozen");
		return;
	end
	
	this:GetBrainManager():SetStateByName("Idle");
	
	ThinkOfSomethingToDo();
	//Called(GetEntityByName("Father"));
	
end

function OnPolishDialogResponse(text, key, dialog) //dialog is passed as a SpecialEntity

    local goal = this:GetGoalManager():PushNewGoal("Wait");
   
if (key == "threat") then
    goal:AddSay("You're a right bastard.  If you'll excuse me...", g_playerID);
  end

if (key == "lie") then
    goal:AddSay("<blush> I'm sorry I thought.. well, I have to go now.", g_playerID);
  end
  
end

function PolishDialog()

//note, the [[ and ]] is a lua way to make defining long strings easier.  You don't have to use it, msg = "crap"; works too.
	local msg = 
[[
The maid scowls.
	
"Do you really want me to tell your dad you said that?"
]]	
	
	entChoice = CreateEntitySpecial("ChoiceDialog", "msg=" .. msg .. ";callback=OnPolishDialogResponse;title=Maid;entity_id=" .. this:GetID());
	
	entChoice:Send("add_choice|If you tell father, I'll tell him I saw you stealing coin!|threat");
	entChoice:Send("add_choice|What?  I meant the shower curtain rod, it's quite grimey indeed.|lie");
	entChoice:Send("activate");

end


function OnDialogResponse(text, key, dialog) //dialog is passed as a SpecialEntity

	if (key == "nevermind" or key == "abort") then
			return;
	end
	
	if (key == "bed") then
	    
	    if (this:GetGoalManager():GetGoalCountByName("CleanPlayerBed") > 0) then
		    local goal = this:GetGoalManager():PushNewGoal("Wait");
			goal:AddSay("I didn't forget!  Just give me a chance to get it done, ok?  I'm very busy you know.", g_playerID);
	    else
		  	
		  	local goal = this:GetGoalManager():PushNewGoal("CleanPlayerBed");
			goal:AddSay("Argh, fine.", g_playerID);
 		    local bedID = GetEntityIDByName("Hol_Inn_BedPlayer");
 		    goal:AddApproachAndSay("<cleaning>",bedID , C_DISTANCE_CLOSE); 
		   
		    local r = math.random(3);
		    if (r == 1) then
				goal:AddApproachAndSay("Still wetting his bed I see.", bedID, C_DISTANCE_CLOSE); 
		    elseif (r == 2) then
				goal:AddApproachAndSay("What the hell are all these stains from.", bedID, C_DISTANCE_CLOSE); 
		    else
				goal:AddApproachAndSay("No amount of bleach is going to get these urine stains out.", bedID, C_DISTANCE_CLOSE); 
		    end
			    

		end
	
	return;
	end
	
	if (key == "fountain") then
	    
	    if (this:GetGoalManager():GetGoalCountByName("CleanFountain") > 0) then
		    local goal = this:GetGoalManager():PushNewGoal("Wait");
			goal:AddSay("I didn't forget!  Just give me a chance to get it done, ok?  I've got a lot to do.", g_playerID);
	    else
		  	
		  	local goal = this:GetGoalManager():PushNewGoal("CleanFountain");
			goal:AddSay("Alright.", g_playerID);
 		    local fountainID = GetEntityIDByName("Hol_Fountain");
 		    //if fountainID is C_ENTITY_NONE it means we can't find it. 
 		    goal:AddApproachAndSay("<cleaning>",fountainID , C_DISTANCE_CLOSE); 

		end
	
	return;
	end

	if (key == "whats_up") then
  			local goal = this:GetGoalManager():PushNewGoal("Talk");
		    goal:AddSay("Not much, just doing my daily chores.", g_playerID);
	  end
	
	if (key == "polish") then
	  	
	   	if (GameIsActive()) then //if a real game is going, we'll set a flag to not show this again
			this:Data():Set("polish", "1"); //doesn't matter what we set it to
		end
	  	PolishDialog();
	  end

end

function OnAction(ent)

	PlayDialogOpenSound();
	
	local entChoice = CreateEntitySpecial("ChoiceDialog", "title=Maid;entity_id=" .. this:GetID());
	entChoice:Send("add_choice|What's going on?|whats_up");
	entChoice:Send("add_choice|Clean my bed!|bed");
	entChoice:Send("add_choice|Go clean the fountain outside!|fountain");

	if (this:Data():Exists("polish") == false) then
		entChoice:Send("add_choice|I have a rod I'd like you to polish.|polish");
	end
	
	entChoice:Send("add_choice|Nevermind|nevermind");
	entChoice:Send("activate");
    
end


function OnGetVisible() //we are on the screen

	ActivateIfNeeded();
	
end

//someone wants us
function Called(entCaller)
	
	 if (this:GetGoalManager():GetGoalCountByName("RespondToCall") > 0) then
	 	//was already called
	 	  if (this:IsCloseToEntity(entCaller, 1000)) then
	 	  	this:GetGoalManager():PushSay("I said I was coming!", entCaller:GetID()); //say it right now
	 	  end
	 	return;
	 end
		
	
	lastCaller = entCaller;
  	local goal = this:GetGoalManager():PushNewGoal("RespondToCall");
 
    goal:AddDelay(1400); //wait for him to stop talking

 	if (this:IsCloseToEntity(entCaller, 1000)) then

		//very close to him
	    goal:AddSay("I hear ya, I'm coming.", entCaller:GetID());
	 else
	    goal:AddSay("I think I heard someone calling me.", entCaller:GetID());
	 end
	 
    goal:AddApproachAndSay("You called?", entCaller:GetID(), C_DISTANCE_TALK); 
		goal:AddRunScriptString('lastCaller:RunFunction("MaidWaiting", this);'); //let him do his thing
		goal:AddDelay(3000); //wait for him to talk
	//actually do the cleaning	
    goal:AddApproachAndSay("<cleaning>", GetEntityIDByName("Hol_Inn_BedParents"), C_DISTANCE_CLOSE); 

end

function OnLoseVisible() //we left the screen

    //let the guy keep walking around for a while, even though he's offscreen
	GetWatchManager:Add(this, 1000*40);
end


function OnWatchTimeout(bIsOnScreen)
end

function OnKill()
end

function CleanupPlace(goal, name)

       goal:PushApproachAndSay("<cleaning>", GetEntityIDByName(name), C_DISTANCE_CLOSE); 

end

function ThinkOfSomethingToDo()
		
		this:GetGoalManager():RemoveAllSubgoals();
		
		local newgoal = this:GetGoalManager():PushNewGoal("BaseThink");
 	   
		//this is what we'll do when all our goals are done
		newgoal:PushRunScriptString('ThinkOfSomethingToDo();'); 	    
 	    newgoal:PushDelay(1000);
 	    newgoal:PushRunScriptString('this:SetFacingTarget(C_FACING_LEFT);');
 		
 		
 		local r = math.random(3);
 		
 		if (r == 1) then newgoal:PushSay("<wipes brow>", C_FACING_NONE); end;
 		if (r == 2) then newgoal:PushSay("I hate my life.", C_FACING_NONE);  end;
 		if (r == 3) then newgoal:PushSay("Cleaning and more cleaning.  Will I ever amount to anything?", C_FACING_NONE);  end;

	   //actually do something

 	    newgoal = this:GetGoalManager():PushNewGoal("CleaningUp");

		//chose a random place or two to clean
		
 	    CleanupPlace(newgoal, "Hol_Inn_Table" .. math.random(3));
 	    CleanupPlace(newgoal, "Hol_Inn_Bed" .. math.random(5));

end


function OnDamage(normal, depth, enemy, damage, userVar, projectile)


	pushVec = this:GetPos() - enemy:GetPos();
	pushVec:Normalize();
	this:AddForce(pushVec * damage);
	
	GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);

   this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");
  
  	if (m_speechTimer < GetApp:GetGameTick()) then
  	
  		GetTextManager:Add("Ouch!", this);
  		
  		m_speechTimer = GetApp:GetGameTick()+2000;
  	end
  
end


