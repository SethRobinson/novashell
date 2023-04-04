//show our items
g_entStoreOwner = nil;
g_buyStoreInventory = nil;

function OnStoreBuyMenu(inventory, ent, defaultOption)
	
	entChoice = CreateEntitySpecial("ChoiceDialog", "callback=OnBuyMenuResponse;title=Buy Menu;selection=" ..
		defaultOption);
	g_entStoreOwner = ent;
	g_buyStoreInventory = inventory;
	
	//add each item we have to the dialog list, g_Item is our item database that we can get info for any item name from, makes
	//more sense than storing it per-instance
	local itemText;
	for i, v in ipairs(inventory) do
		
		local itemDB = g_Items[v.item];
		
		itemText = itemDB.name;
		if (itemDB.description != nil) then
			itemText = itemText .. " (" .. itemDB.description .. ") ";
		end
		itemText = itemText .. " Cost: " .. itemDB.value;
		
		entChoice:Send("add_choice|" .. itemText .. "|" .. v.item);
	end
	
	entChoice:Send("add_choice|Back|back");
	entChoice:Send("activate");
	
	return false; //don't process any other callbacks for this keypress
end

//message we want to show, as well as the item they were last looking at
function ShowBuyMessage(message, defaultOption)

	entChoice = CreateEntitySpecial("ChoiceDialog", "msg=" .. message .. ";callback=OnStoreBuyMessageResponse;title=Buy Menu;selection=" ..
		defaultOption);
		entChoice:Send("add_choice|Continue|" .. defaultOption); //we'll send the last item they were looking at..
	entChoice:Send("activate");

end

function OnStoreBuyMessageResponse(text, key, dialog)
		//done showing little message, show the store's goods again
		OnStoreBuyMenu(g_buyStoreInventory, g_entStoreOwner, key); //key is actually the last item they were looking at
end

function OnBuyMenuResponse(text, key, dialog) //dialog is passed as a SpecialEntity

		g_dialogOpenSound = false; //don't play the open dialog sound
		
		if (key == "back" or key == "_escape_") then
			g_buyStoreInventory = nil; //hint for lua's garbage collection.. uh..I guess...
			return g_entStoreOwner:RunFunctionIfExists("OnStoreEnd");
		end
		
		//they must have chosen an item to buy.
		local itemDB = g_Items[key];
		
		if (GetPlayer:Data():GetNum("Money") < itemDB.value) then
			//can't afford it
			ShowBuyMessage("You don't have enough gold!", key);		
			return;
		end

		//remove the money
		GetPlayer:Data():ModNum("Money", -itemDB.value);
		GetPlayer:RunFunction("GiveItem", key); //actually give the player the item
		GetPlayer:RunFunction("UpdateGUI"); //register the change visually
		GetSoundManager:Play("audio/effects/coins.ogg");
		ShowBuyMessage("You have purchased a " .. itemDB.name .. "!", key);		
		
end

