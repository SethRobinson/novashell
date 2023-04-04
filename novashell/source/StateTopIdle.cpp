#include "AppPrecomp.h"
#include "StateTopIdle.h"
#include "MovingEntity.h"

StateTopIdle registryInstanceStateTopIdle(NULL); //self register ourselves i nthe brain registry

StateTopIdle::StateTopIdle(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTopIdle::~StateTopIdle()
{
}

void StateTopIdle::OnAdd()
{
	
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_IDLE);
	
	m_bCallbackActive = m_pParent->GetScriptObject()->FunctionExists("OnIdleLoop");
}

void StateTopIdle::OnRemove()
{
}

void StateTopIdle::Update(float step)
{

}

void StateTopIdle::PostUpdate(float step)
{
	
	if (m_bCallbackActive && AnimIsLooping())
	{
		m_pParent->RunFunction("OnIdleLoop");
	}
}

/*
Object: Idle
A state that causes an entity to stop and play his idle animation.

An idle animation is detected from the visual profile .xml when anim states named <idle_left>, <idle_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <idle_left>.

Script functions it calls:

If the script function *"OnIdleLoop"* exists in the entity's script, it will call it every time the animation loops.

*/