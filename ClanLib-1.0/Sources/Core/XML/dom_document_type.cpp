/*
**  ClanLib SDK
**  Copyright (c) 1997-2005 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    (if your name is missing here, please add it)
*/

#include "Core/precomp.h"
#include "API/Core/XML/dom_document_type.h"
#include "API/Core/XML/dom_named_node_map.h"
#include "dom_node_generic.h"

/////////////////////////////////////////////////////////////////////////////
// CL_DomDocumentType construction:

CL_DomDocumentType::CL_DomDocumentType()
{
}

CL_DomDocumentType::CL_DomDocumentType(CL_DomDocument &doc)
: CL_DomNode(doc, DOCUMENT_TYPE_NODE)
{
}

CL_DomDocumentType::CL_DomDocumentType(const CL_SharedPtr<CL_DomNode_Generic> &impl) : CL_DomNode(impl)
{
}
	
CL_DomDocumentType::~CL_DomDocumentType()
{
}

/////////////////////////////////////////////////////////////////////////////
// CL_DomDocumentType attributes:

std::string CL_DomDocumentType::get_name() const
{
	if (impl) return impl->node_name;
	return std::string();
}
	
CL_DomNamedNodeMap CL_DomDocumentType::get_entities() const
{
	return CL_DomNamedNodeMap();
}
	
CL_DomNamedNodeMap CL_DomDocumentType::get_notations() const
{
	return CL_DomNamedNodeMap();
}

/////////////////////////////////////////////////////////////////////////////
// CL_DomDocumentType operations:

/////////////////////////////////////////////////////////////////////////////
// CL_DomDocumentType implementation:
