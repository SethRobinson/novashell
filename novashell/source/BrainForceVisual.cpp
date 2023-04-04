#include "AppPrecomp.h"
#include "BrainForceVisual.h"
#include "MovingEntity.h"
#include "VisualProfile.h"

BrainForceVisual registryInstanceBrainForceVisual(NULL); //self register ourselves i nthe brain registry

BrainForceVisual::BrainForceVisual(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	m_forceSetID = VisualProfile::VISUAL_STATE_NONE;
}


void BrainForceVisual::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "force_anim")
		{
			m_forceAnim = words[1];
			m_pParent->SetAnimByName(m_forceAnim);

		}  else
			if (words[0] == "force_set")
			{
				m_forceSetID = CL_String::to_int(words[1]);
				m_pParent->SetVisualStateOverride(m_forceSetID);
			} 
		else
		{
			LogMsg("Brain %s doesn't understand keyword '%s'", GetName(), words[0].c_str());
		}
	}

}


BrainForceVisual::~BrainForceVisual()
{
	if (m_forceSetID != VisualProfile::VISUAL_STATE_NONE)
	{
		
		if (m_pParent) m_pParent->SetVisualStateOverride(VisualProfile::VISUAL_STATE_NONE);
	} 	
}

void BrainForceVisual::Update(float step)
{
	if (m_forceSetID != VisualProfile::VISUAL_STATE_NONE)
	{
	} else
	{
		m_pParent->GetBrainManager()->SendToBrainBaseSimple(C_BRAIN_MESSAGE_DONT_SET_VISUAL,0,0);
	}
	
}

/*
Object: ForceVisual
A brain for use with the <BrainManager> that overrides its owner <Entity>'s visual settings.

About:

Usually an entity plays the animation associated with its state, if available.  For instance, while in the <Walk> state, the walk animation in the <Entity>'s visual profile would be played.

What this does:

This Brain allows you to force a specific animation to be played, regardless of what is going on with the AI.

Currently this only supports setting a single animation by name, but later it should allow you to set animation "sets" (to handle different directions) and animations from different visual profiles.

Parameters it understands:

force_anim - Forces this animation to be played and causes the AI system to stop changing it.

force_set - Forces this visual set to be played.  One of the C_VISUAL_STATE defines from setup_constants.lua.

Usage:ual
(code)
this:GetBrainManager():Add("ForceVisual", "force_anim=climb"); //look like we're climbing, using the "climb" anim in our visual profile.
this:GetBrainManager():Add("ForceVisual", "force_set=" .. C_VISUAL_STATE_JUMP ); //look like we're jumping, supporting different anims for each direction
(end)

*/

