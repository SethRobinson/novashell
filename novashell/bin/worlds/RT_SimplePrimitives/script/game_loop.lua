//Startup a game loop, this function gets called after all entities run their logic each frame

GetGameLogic:AddCallbackOnPostLogicUpdate("OnGameLoop", C_ENTITY_NONE);

function RandomSpotOnScreen()
	return Vector2(random(0, C_SCREEN_X), random(0, C_SCREEN_Y));
end

function RandomColor()
	return Color(random(255),random(255),random(255),random(128)+127)
end

function RandomRectOnScreen()
	local r = Rectf();
	local vA = RandomSpotOnScreen();
	local vB = RandomSpotOnScreen();
	r.left = vA.x;
	r.top = vA.y;
	r.right = vB.x;
	r.bottom = vB.y;
	
	return r;
end

//this is run every logic tick
function OnGameLoop(step)

		//draw a rect

		GetDrawManager:DrawFilledRect(RandomRectOnScreen(), RandomColor(), 5, C_DRAW_SCREEN_COORDS);
		
		GetDrawManager:DrawRect(RandomRectOnScreen(), RandomColor(), 5, C_DRAW_SCREEN_COORDS);

		for i=1, 100 do
			GetDrawManager:DrawLine(RandomSpotOnScreen(), RandomSpotOnScreen(), RandomColor(), C_LAYER_TOP, C_DRAW_SCREEN_COORDS);
		end
		

		for i=1, 100 do
			GetDrawManager:DrawPixel(RandomSpotOnScreen(), RandomColor(), C_LAYER_TOP, C_DRAW_SCREEN_COORDS);
		end
		
		
end
