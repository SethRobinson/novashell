//misc utility functions


//Ian's cool class wrapper to make faking a 'class' easier. 

do
    local function call(cls, ...)
        instance = setmetatable({}, cls)
        if cls.__init then
            cls.__init(instance, ...)
        end
        return instance
    end

    function class()
        local cls = {}
        cls.__index = cls
        return setmetatable(cls, {__call = call})
    end
end

//here's an example/test using the class function
--[[ start comment
Monster = class();  //create the class.

//define the class function protypes
function Monster:__init()
	//if a function named __init exists, it will be called when an object is initialized (like a constructor)
	LogMsg("A monster initted!");
	self.hitpoints = 10;
end

function Monster:Hurt(damage)
	self.hitpoints = self.hitpoints - damage;
end

//ok, now let's actually make some objects and use them.

dave = Monster();
tom = Monster();

LogMsg("Dave has " .. dave.hitpoints .. " hitpoints.");
LogMsg("Tom has " .. tom.hitpoints .. " hitpoints.");

dave:Hurt(2); 
tom:Hurt(8);

//show hitpoints again

LogMsg("Dave has " .. dave.hitpoints .. " hitpoints.");
LogMsg(" Tom has " .. tom.hitpoints .. " hitpoints.");

]]--


//utility to show what's in a table/structure
function Dump(a)
	local varType = type(a);
	LogMsg("Dumping " .. varType); //we really hope it has a tostring() function!
	LogMsg("------------------");
	
	if (varType == "table") then
		LogMsg(serialize(a));
		return;
	end


	//default
	LogMsg(a);
end

//returns the previously serialized as a table or whatever
function unserialize(o)
	loadstring( ("_G.temp = " .. o))();
	//isn't there a better way to do this without leaving "temp" floating around?
	return temp;
end

function unserialize_from_db(dbName, db)
	local s = db:Get(dbName);
	if (s == "") then return {}; end;
	return unserialize(s);
end

function serialize_to_db(inTable, dbName, db)
	if (inTable == nil) then return; end;
	db:Set(dbName, serialize(inTable));
end

//Author: Matt Campbell, based on an example from Programming in Lua

function serialize(o)
 local t = type(o)
 if t == "number" then
   return tostring(o)
 elseif t == "string" then
   return string.format("%q", o)
 elseif t == "table" then
   local result = "{"
   local nextIndex = 1
   local first = true
   for k, v in pairs(o) do
     if first then
       first = false
     else
       result = result .. ", "
     end
     if type(k) == "number" and k == nextIndex then
       nextIndex = nextIndex + 1
     else
       if type(k) == "string" and string.find(k, "^[_%a][_%w]*$") then
         result = result .. k
       else
         result = result .. "[" .. serialize(k) .. "]"
       end
       result = result .. " = "
     end
     result = result .. serialize(v)
   end
   result = result .. "}"
   return result
 else
   return tostring(o)
 end
end

do
    local scripts = {}

    function require(filename)
        if not scripts[filename] then
            scripts[filename] = true
            RunScript(filename)
        end
    end
end


function toboolean(X)
	if (tonumber(X) == 0) then return false; end;
	if (X == "false") then return false; end;
	if (X == false) then return false; end;
	if (X == "") then return false; end;
	return true;
end

function GameIsActive()
	if (GetGameLogic:UserProfileActive() != false) then return true; end;
	return false;
end


function DefaultEscapeHandler()

	
	if (GetGameLogic:UserProfileActive() and GetMapManager:GetActiveMap():GetName() != "Intro" ) then
	 	
		//they are currently playing, let's quit back to the main menu
		GetGameLogic:SetRestartEngineFlag(true);
	  
	   
	else

		//they are sitting at the main menu, or inside a game that doesn't use profiles
	
	if (g_isRetail) then
			
			GetGameLogic:Quit(); //this will just quit the whole game
	    return;
		end
			
		//go back to the worlds menu
		GetGameLogic:ClearModPaths(); //so this world won't load again
		GetGameLogic:SetRestartEngineFlag(true);
		  //if the screenres is weird, let's put it back to normal
	  GetApp:SetScreenSize(1024, 768);
	
	end

			return false; //don't process any other callbacks for this keypress
end

//my editor keeps changing Random to random, so uh..
Random = math.random;
random = math.random;

//util to make case insensitive searches

function string.findnocase(a, b)
	return string.find(a:lower(), b:lower());
end

function CenterCameraOnThisEntityIfAvailable(entName)

	local ent = GetEntityByName("Player");
	
	if (ent == nil) then
		return false;
	end
	
	GetCamera:SetPosCentered(ent:GetPos());

end

function DeleteEntityByNameIfExists(name)

	local pEnt = GetEntityByName(name);
	if (pEnt != nil) then
		pEnt:SetDeleteFlag(true);
		return true;
	end

	return false; //nothing deleted
end

function DeleteEntityByIDIfExists(id)

	if (id == C_ENTITY_NONE) then
		return false;
		end;

	pEnt = GetEntityByID(id);
	if (pEnt != nil) then
		pEnt:SetDeleteFlag(true);
		return true;
	end

	return false; //nothing deleted
end

//simple function to enlarge a rectangle on all sides by a certain amount

function PadRect(rect, padAmount)
	
	rect.right = rect.right + padAmount;
	rect.left = rect.left - padAmount;
	rect.top = rect.top - padAmount;
	rect.bottom = rect.bottom + padAmount;
	return rect;

end

function ScaleEntToScreen(ent, vSrcScreenSize)
	local vScale =Vector2(C_SCREEN_X/vSrcScreenSize.x, C_SCREEN_Y/vSrcScreenSize.y);
	//LogMsg("setting scale to", vScale);
	ent:SetScale( vScale);
end

