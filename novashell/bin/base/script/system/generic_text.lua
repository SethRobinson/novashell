//any entity can have a text label.  This script sets one up based on options in it's custom
//data.  (look at the entities properties to see them)
//It also resizes the entity image to match the text, so if you assign this to anything, it should
//have a 1,1 pixel size.

//To change a value by script later, call SetText by script, ie, entText:RunFunction("SetText", "New text");

function OnInit() //run upon initialization
  this:Data():SetIfNull("text", "Text!");
  this:Data():SetIfNull("color", "255 255 255 255");
  this:Data():SetIfNull("scale", "1.0 1.0");

  this:SetText(this:Data():Get("text"));
  this:SetTextColor(StringToColor(this:Data():Get("color")));
  this:SetTextScale(StringToVector(this:Data():Get("scale")));
  
  this:SetTextAlignment(C_ORIGIN_CENTER, Vector2(0,0));
  
  ScaleEntityToMatchText();
  
end

function ScaleEntityToMatchText()
	
	local vecSize = this:GetTextBounds(); //see how big this text is
	  
	  //match it with our size, assuming our entity pic is 1,1 in size, this will scale
	  //correctly
	  
	  if (vecSize:Length() > 2) then
	  	this:SetScale(vecSize);
	  else
	  //well, the text is so tiny (or blank) I'd rather not scale it, might be hard to click
	  end
  
end

function SetText(text)
	
	this:Data():Set("text", tostring(text)); //store it for serializing to disk
    this:SetText(tostring(text));
 
  	ScaleEntityToMatchText();
  	
end

function SetTextScale(v)
	this:SetTextScale(v);
	
	//make this change persistant if it's moved/saved
	 this:Data():Set("scale", VectorToString(v));

	ScaleEntityToMatchText();
end

function OnPostInit() //run during the entity's first logic update
end

function OnKill() //run when removed

end

