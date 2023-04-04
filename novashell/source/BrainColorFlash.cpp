#include "AppPrecomp.h"
#include "BrainColorFlash.h"
#include "MovingEntity.h"

BrainColorFlash registryInstanceColorFlash(NULL); //self register ourselves i nthe brain registry

BrainColorFlash::BrainColorFlash(MovingEntity * pParent):Brain(pParent)
{
	if (!pParent)
	{
		//this is a dummy for our blind factory to get info from
		RegisterClass();
		return;
	}

	SetPulseRate(0);
	m_bOn = true;

	m_r = 0;
	m_g = 0;
	m_b = 0;
	m_a = 0;

	m_remove_brain_by_pulses = 0; //disable this
}

void BrainColorFlash::SetPulseRate(int pulseRate)
{
	m_pulseRate = pulseRate;
	m_flashTimer.SetInterval(m_pulseRate);
}

void BrainColorFlash::HandleMsg(const string &msg)
{
	vector<string> messages = CL_String::tokenize(msg, ";",true);

	for (unsigned int i=0; i < messages.size(); i++)
	{
		vector<string> words = CL_String::tokenize(messages[i], "=",true);

		if (words[0] == "r")
		{
			m_r = CL_String::to_int(words[1]);
		} else
			if (words[0] == "g")
			{
				m_g = CL_String::to_int(words[1]);
			} else
				if (words[0] == "b")
				{
					m_b = CL_String::to_int(words[1]);
				} else
					if (words[0] == "a")
					{
						m_a = CL_String::to_int(words[1]);
					} else
			if (words[0] == "pulse_rate")
			{
				SetPulseRate(CL_String::to_int(words[1]));
			} else
				if (words[0] == "remove_brain_by_pulses")
				{
					m_remove_brain_by_pulses = CL_String::to_int(words[1]);
				} else
				{
				LogMsg("Brain %s doesn't understand keyword %s", GetName(), words[0].c_str());
			}
	}

}

BrainColorFlash::~BrainColorFlash()
{
}

void BrainColorFlash::Update(float step)
{
	if (m_bOn)
	{
		m_pParent->AddColorModRed(m_r);
		m_pParent->AddColorModGreen(m_g);
		m_pParent->AddColorModBlue(m_b);
		m_pParent->AddColorModAlpha(m_a);
	}

	if (m_pulseRate != 0)
	if (m_flashTimer.IntervalReached())
	{
		m_bOn = !m_bOn;
		if (m_remove_brain_by_pulses != 0)
		{
			m_remove_brain_by_pulses--;
			
			if (m_remove_brain_by_pulses == 0)
			{
				//remove ourself
				SetDeleteFlag(true);
			}
		}
	}

}


/*
Object: ColorFlash
A brain for use with the <BrainManager> that causes its <Entity>'s to flash different colors/alpha values.

The coloring temporarily overrides the <Entity>'s base color without really changing it.  To permenently change it, use <ColorFade> or <Entity::SetBaseColor>.
Parameters it understands:

pulse_rate - How fast each flash lasts.
remove_brain_by_pulses - If set, after this number of pulses occur, the brain will be removed automatically.
r - The pulse red color, 0 to 255.
g - The pulse green color, 0 to 255.
b - The pulse blue color, 0 to 255.
a - The pulse alpha color, 0 to 255.

Usage:
(code)
//flash red for 10 pulses
this:GetBrainManager():Add("ColorFlash", "pulse_rate=50;g=-200;b=-200;remove_brain_by_pulses=10");
(end)

*/
