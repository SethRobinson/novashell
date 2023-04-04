#include "AppPrecomp.h"
#include "StateRun.h"
#include "MovingEntity.h"

StateRun registryInstanceStateRun(NULL); //self register ourselves in the brain registry

StateRun::StateRun(MovingEntity * pParent):State(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

}

StateRun::~StateRun()
{
}

void StateRun::OnAdd()
{
	if (m_pParent->GetVisualProfile())
	{
		m_pParent->SetVisualState(VisualProfile::VISUAL_STATE_RUN);
	}

	m_bCallbackActive = m_pParent->GetScriptObject()->FunctionExists("OnRunLoop");
}

void StateRun::OnRemove()
{
}

void StateRun::Update(float step)
{
	if (m_pParent->IsFacingTarget(0.5))
	{
		m_pParent->GetBrainManager()->GetBrainBase()->AddWeightedForce( m_pParent->GetVectorFacing() * m_pParent->GetDesiredSpeed() );
	}
}

void StateRun::PostUpdate(float step)
{
	if (m_bCallbackActive && AnimIsLooping())
	{
		m_pParent->RunFunction("OnRunLoop");
	}
}


/*
Object: Run
A state that causes an entity to play his run animation and move in the direction he's current facing at the speed set in <Entity::SetDesiredSpeed>.

A run animation is detected from the visual profile .xml when anim states named <run_left>, <run_down_left>, and so on are found.  The best match for the <Entity>'s current direction is used.

If you want only one animation for all directions, only set an animation for <run_left>.

Script functions it calls:

If the script function *"OnRunLoop"* exists in the entity's script, it will call it every time the animation loops.
*/
