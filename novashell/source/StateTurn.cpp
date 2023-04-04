#include "AppPrecomp.h"
#include "StateTurn.h"
#include "MovingEntity.h"

StateTurn registryInstanceStateTurn(NULL); //self register ourselves i nthe brain registry

StateTurn::StateTurn(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateTurn::~StateTurn()
{
}

void StateTurn::OnAdd()
{
	m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_TURN);
}

void StateTurn::OnRemove()
{
}

void StateTurn::Update(float step)
{

}

void StateTurn::PostUpdate(float step)
{
	if (AnimIsLooping() && m_pParent->GetScriptObject()->FunctionExists("OnTurnLoop"))
	{
		m_pParent->RunFunction("OnTurnLoop");
	}
}

/*
Object: Turn
A state that causes an entity to stop moving and play his attack animation.


An attack animation is detected from the visual profile .xml when anim states named <turn_left>, <turn_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <turn_left>.

Script functions it calls:

If the script function *"OnTurnLoop"* exists in the entity's script, it will call it every time the animation loops.
*/
