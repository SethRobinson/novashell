//helpful global audio related functions that are game specific

//if not already playing, start up some cool fighting audio
function PlayBattleMusic()
	
		g_SmartSound:SetMusic("audio/music/Stoneworld Battle_loop.ogg", false, false);

end


function CurrentlyPlayingBattleMusic()

	if (string.find(g_SmartSound:GetActiveAudioFileName(), "Battle") != nil) then
		//well, the active song has the text "Battle" somewhere in it, assume this is some battle music
		return true;
	end
	return false; //not currently playing battle music I guess
end

function EndBattleMusicIfNoEnemiesAreClose()
	if (CurrentlyPlayingBattleMusic()) then
			g_SmartSound:SetMusic("", true, true);
  end
end