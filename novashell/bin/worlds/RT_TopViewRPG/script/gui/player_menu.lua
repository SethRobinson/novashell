RunScript("gui/item_menu.lua");

//when escape is hit, it brings up the players menu.

function OnMainMenu()
	
	
	entChoice = CreateEntitySpecial("ChoiceDialog", "callback=OnPlayerMenuResponse;title=Main Menu");
	
	//get the table of player items
	local playerInventory = GetPlayer:RunFunction("GetInventory");
	
	//the # in front of a table returns its item count, it's a lua thing
	entChoice:Send("add_choice|Inventory (" .. #playerInventory .. ")|inventory");
	entChoice:Send("add_choice|Quit and save|quit_and_save");
	entChoice:Send("add_choice|Back|return");

	entChoice:Send("activate");
	
	return false; //don't process any other callbacks for this keypress
end

function OnPlayerMenuResponse(text, key, dialog) //dialog is passed as a SpecialEntity

	if (key == "quit_and_save") then
			return DefaultEscapeHandler(); //really let them quit
		end;
		
		if (key == "inventory") then
			return OnItemMenu("_first_"); //in item_menu.lua
		end

end

