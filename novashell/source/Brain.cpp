#include "AppPrecomp.h"
#include "Brain.h"
#include "BrainManager.h"

#define C_DEFAULT_SORT 10

Brain::Brain(MovingEntity * pParent): m_pParent(pParent)
{
	m_bDeleteFlag = false;
	SetSort(C_DEFAULT_SORT);
}

Brain::~Brain()
{
}

void Brain::RegisterClass()
{
	assert(!m_pParent);

		//no parent class given, assume we want to register this instance in the
		//brain registry
		BrainRegistry::GetInstance()->Add(this);
}
