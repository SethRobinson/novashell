//show our items
g_activePlayerItem = nil; //the current player item we're looking at (g_activePlayerItem.item holds its db name)
g_activePlayerItemListID = nil; //the index of the current item we're looking at in the players inventory list

function OnItemMenu(defaultOption)
	
	entChoice = CreateEntitySpecial("ChoiceDialog", "callback=OnItemMenuResponse;title=Inventory;selection=" .. defaultOption);
	
	//get the table of player items
	local playerInventory = GetPlayer:RunFunction("GetInventory");
	
	//add each item we have to the dialog list, g_Item is our item database that we can get info for any item name from, makes
	//more sense than storing it per-instance
	
	local itemText;
	
	for i, v in ipairs(playerInventory) do
			if (g_Items[v.item] == nil) then
				LogError("Item type " .. v.item .. " doesn't seem to be specified in item.lua!");
		else
			local itemDB = g_Items[v.item];
			
			itemText = itemDB.name;
			if (itemDB.description) then
					itemText = itemText .. "(" .. itemDB.description .. ")";
				end
			entChoice:Send("add_choice|" .. itemText .. "|" .. tostring(i));
		end
	end
	
	entChoice:Send("add_choice|Back|back");
	entChoice:Send("activate");
	
	return false; //don't process any other callbacks for this keypress
end

function OnItemMenuResponse(text, key, dialog) //dialog is passed as a SpecialEntity
		
		if (key == "back" or key == "_escape_") then
			return OnMainMenu(); //in item_menu.lua
		end
		
		//they chose one of their items
		
		g_activePlayerItem = GetPlayer:RunFunction("GetInventory")[tonumber(key)];
		g_activePlayerItemListID = tonumber(key);
		OpenItemMenu(key);
		
end


//let's show the options for the item they chose
function OpenItemMenu(key)

	local itemDB = g_Items[g_activePlayerItem.item];
	entChoice = CreateEntitySpecial("ChoiceDialog", "msg=;callback=OnOpenItemMenuResponse;title=" .. itemDB.name);

	//so what can we do with this item?
	if (itemDB.use_action != nil) then
		//it has a use_action property, so let's add this option to the list
		entChoice:Send("add_choice|" .. itemDB.use_action .. "|action");
	end
	
	entChoice:Send("add_choice|Destroy|destroy");

	entChoice:Send("add_choice|Back|back"); //we'll send the last item they were looking at..
	entChoice:Send("activate");

end

function OnOpenItemMenuResponse(text, key, dialog)
		//done showing little message, show the store's goods again
	
	local playerInventory = GetPlayer:RunFunction("GetInventory");

local behavior = "return_to_menu";
		
		if (key == "destroy") then
		
			RemoveItemByIndex(playerInventory, g_activePlayerItemListID);
	end

	if (key == "action") then
		//call the function associated with this action, hopefully it exists somewhere (probably in items.lua)
			behavior = loadstring("return " .. g_activePlayerItem.item .. "_action(g_activePlayerItem)")();
		  //remove it?
		 	RemoveItemByIndex(playerInventory, g_activePlayerItemListID);
	end

//load the item menu back up?

if (	behavior == "return_to_menu") then
			if (#playerInventory == 0) then
				return OnItemMenu("_first_"); //no default item
			end
	
			if (g_activePlayerItemListID > #playerInventory) then
					//there is no item in the current slot, move it a bit
					g_activePlayerItemListID = g_activePlayerItemListID-1;
					end
			return OnItemMenu(g_activePlayerItemListID); //key is actually the last item they were looking at		
	end	
end