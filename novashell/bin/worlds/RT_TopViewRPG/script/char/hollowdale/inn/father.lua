	
function OnInit()
	
	this:SetVisualProfile("char/man/man.xml", "man");
	this:LoadCollisionInfo("char/man/man.col");
	this:SetDensity(1); //control how hard he is to push.  0 would mean unpushable
	m_speechTimer = 0;
	this:SetCategory(C_CATEGORY_CHARACTER, true);
	this:SetCategory(C_CATEGORY_ENTITY, false);
	LogMsg("Initting father");
end

function OnPostInit()
	LogMsg("Father on post init..");
	AttachPointer(this, 7); //in rpg_utils.lua

	this:GetBrainManager():Add("StandardBase","");
	this:GetBrainManager():SetStateByName("Idle");
  this:SetFacingTarget(C_FACING_DOWN);
	
	ThinkOfSomethingToDo();

end

function OnKill()
end

function MaidWaiting(entMaid)

 	    this:SetVectorFacing(this:GetVectorToEntity(entMaid)); //look at the maid
 	    GetTextManager:Add("Yeah.  Go clean my bed.  NOW.", this);
	
end

function CallMaid()
		

		local maid = GetEntityByName("InnMaid"); //will locate and load if needed
		
		if (maid == nil) then
		
			//entity doesn't exist
	 	    GetTextManager:Add("Where the hell is that maid?  Did she quit?", this);
		else

			 if (maid:GetGoalManager():GetGoalCountByName("RespondToCall") > 0) then
			 	//she's currently responding to us, we shouldn't bother her
			 	this:GetGoalManager():PushSay("Hurry up, woman!", maid:GetID());
			 else
	
		 		this:GetGoalManager():PushSay("Serving wench!  'git over here!", maid:GetID());
				maid:RunFunction("ActivateIfNeeded");  //this is a function we made in her script
				maid:RunFunction("Called", this); //this is a function we made in her script
			end
		
		end
end


function ThinkOfSomethingToDo()

		this:GetGoalManager():RemoveAllSubgoals();
		
		newgoal = this:GetGoalManager():PushNewGoal("BaseThink");
 	   
		//this is what we'll do when all our goals are done
		newgoal:PushRunScriptString('ThinkOfSomethingToDo();'); 	    
 	    newgoal:PushDelay(14000);
	    newgoal:PushRunScriptString('this:SetFacingTarget(C_FACING_LEFT);');
 	    newgoal:PushDelay(14000);
 	    newgoal:PushRunScriptString('this:SetFacingTarget(C_FACING_RIGHT);');
 	    newgoal:PushDelay(14000);
 	    newgoal:PushMoveToPosition(GetTagManager:GetPosFromName("InnFatherHome"));
    	newgoal:PushRunScriptString('CallMaid();');
  	    newgoal:PushDelay(10000);
 	

end



function OnFatherDialogResponse(text, key, dialog) //dialog is passed as a SpecialEntity)

		if (key == "hi") then
	    
	        local goal = this:GetGoalManager():PushNewGoal("Talk");
			goal:AddSay("Not much, son.", g_playerID);
	    end

		if (key == "buy") then
		
			//setup a store menu
			inventory = {};
			AddItem("red_potion", inventory);
			OnStoreBuyMenu(inventory, this, "_first_");
			
		end

end

function OnStoreEnd()
	//store ended, start back up our conversation
	OpenDialogMenu("buy");
end

function OpenDialogMenu(defaultOption)
	
	if (defaultOption == "_first_") then
		PlayDialogOpenSound();
	end
	//note, the [[ and ]] is a lua way to make defining long strings easier.  You don't have to use it, msg = "crap"; works too.
		local msg = 
	[[Your father stands before you.
		
"What do you want kid?"]]	
	
	entChoice = CreateEntitySpecial("ChoiceDialog", "msg=" .. msg .. 
	";callback=OnFatherDialogResponse;title=Your Father;entity_id=" .. this:GetID()
	 .. ";selection=" .. defaultOption);
		
		entChoice:Send("add_choice|What's up, dad?|hi");
		entChoice:Send("add_choice|View items for sale|buy");
		entChoice:Send("add_choice|Nevermind|back");
		entChoice:Send("activate");
end

function OnAction(ent)

	OpenDialogMenu("_first_");

end

function OnDamage(normal, depth, enemy, damage, userVar, projectile)

	pushVec = this:GetPos() - enemy:GetPos();
	pushVec:Normalize();

	//this:AddForce(pushVec * damage);
	
	GetTextManager:AddCustom(tostring(damage), this, this:GetPos(), Vector2(0, -0.3), Color(200,0,0,255), 2000, C_FONT_DEFAULT);

   this:GetBrainManager():Add("ColorFlash", "pulse_rate=200;g=-200;b=-200;remove_brain_by_pulses=1");
  
  	if (m_speechTimer < GetApp:GetGameTick()) then
  	
  		GetTextManager:Add("Stop that.  I'm your father for gods sake!", this);
  		
  		m_speechTimer = GetApp:GetGameTick()+2000;
  	end
  
end


