function OnInit()

	//any entity can be a warp target, but this script gives it a nifty icon.  You must
	//set its NAME to the target it should be.
	this:SetVisualProfile("~/system.xml", "ent_warp_tag");
	this:SetDensity(0); //makes it immovable, speeds up processing too
	
end