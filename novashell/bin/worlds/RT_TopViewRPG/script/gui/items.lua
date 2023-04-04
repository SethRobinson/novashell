//setup our item database, called from game_start.lua

//The way items work is there is a global table variable of each kind of item that is referenced by name - this is for the info
//that doesn't change.

//A players "inventory" is a lua table that contains the name of each item he owns (the specifics of which can then be referenced
//from the global g_Items table) plus it contains any 'per-instance' data that is needed, like how many blasts of the magic
//wand are left.

g_Items = {};

//add the items we need in the game to our global database, happens once at the start

//the "uses" variable should be 1 (one use) or not defined (which would means unlimited/na).

g_Items.red_potion = {name = "Red Potion", description = "Restores 20 health", uses=1, value=10, use_action = "Drink"};
g_Items.damaged_claw = {name = "Damaged Claw", value=2};
g_Items.claw = {name = "Claw", value=4};

//some general functions to make dealing with items easier

function AddItem(itemName, inventory)

	//push to the front
	table.insert(inventory, 1, {item = itemName});
	//Dump(inventory);
end

function RemoveItemByIndex(inventory, indexID)
	table.remove(inventory, indexID);
end

//special handing for items that do stuff

function red_potion_action(playerItem)

	GetSoundManager:Play("audio/effects/gulp.ogg");
	GetPlayer:RunFunction("OnHeal", 20);
	return "return_to_menu";  //defines if the item menu will take over or not after this is run
end

