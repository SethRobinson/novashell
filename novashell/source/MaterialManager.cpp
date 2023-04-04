#include "AppPrecomp.h"
#include "MaterialManager.h"


MaterialManager g_materialManager; //I should make the class a singleton. uh.. 

MaterialManager::MaterialManager()
{
	Init();
}

MaterialManager::~MaterialManager()
{
}

void MaterialManager::Init()
{
	m_vecMat.clear();
}

int MaterialManager::AddMaterial(float fCoF, float fCoR, float fCoS, float fSep, const CL_Color &col, const string &name)
{
	CMaterial m;
	m.SetFriction(fCoF);
	m.SetRestitution(fCoR);
	m.SetStaticFriction(fCoS);
	m.SetSeparation(fSep);
	m.SetName(name);
	m.SetID(m_vecMat.size());
	m.SetColor(col.color);
	m_vecMat.push_back(m);

	return m_vecMat.size()-1;
}

CMaterial * MaterialManager::GetMaterial( int idx )
{
	if (idx < int(m_vecMat.size()) && idx >= 0)
	{
		return &m_vecMat[idx];
	}

	LogError("A material of ID %d was referenced but does not exist.. missing some GetMaterialManager:AddMaterial() commands that this world needs?", idx);
	return &m_vecMat[0];
}