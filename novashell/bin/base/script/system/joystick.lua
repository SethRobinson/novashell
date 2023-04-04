
//detect different joysticks and fix its settings if applicable.  For some reason every joystick driver likes to
//use different axis mapping for the second stick.  Default is 2 and 7.

local joyInfo = {}

	//autodetect and configure joysticks by a name search.  "match" is a piece of text that must be in the joystick drivers
	//name to be detected, so you don't want to use something so generic it will be detected for the wrong joysticks too.
	//The axisX/axisY are settings for the right stick.
	
	//add more auto detection entries by following the same format.  Send to Seth (seth@rtsoft.com) if you add a new one!
	
	//XBOX 360 controller, both wired and wireless
	table.insert(joyInfo, 1, {match="xbox", axisX=5, axisY=6});

	//Sony DualsShock hooked up through a Radio Shack USB Dongle (illspirit)
	table.insert(joyInfo, 1, {match="USB Dual Vibration Joystick", axisX=7, axisY=2});	
	

for i=0, GetInputManager:GetJoystickCount()-1, 1 do
	
		local joy = GetInputManager:GetJoystick(i);
		
		//scan through out joystick DB and see if there are any string matches
			for j, v in ipairs(joyInfo) do
	
				if (string.findnocase(joy:GetName(), v.match) != nil) then
					LogMsg("Configured right stick for device " .. i .. " - '" .. joy:GetName() .. "'");
					joy:SetRightStickAxis(v.axisX,v.axisY); 
				end
			end
	
	//if you want to force which axis are used for the right 'stick' for testing,
	// you could do it here, uncomment below:
	
	//joy:SetRightStickAxis(5,6); //force axis
	end
 