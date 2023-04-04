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

//! clanCore="XML"
//! header=core.h

#ifndef header_dom_named_node_map
#define header_dom_named_node_map

#ifdef CL_API_DLL
#ifdef CL_CORE_EXPORT
#define CL_API_CORE __declspec(dllexport)
#else
#define CL_API_CORE __declspec(dllimport)
#endif
#else
#define CL_API_CORE
#endif

#if _MSC_VER > 1000
#pragma once
#endif

#include "../System/sharedptr.h"

class CL_DomNode;
class CL_DomNamedNodeMap_Generic;

//: DOM Named Node Map class.
//- !group=Core/XML!
//- !header=core.h!
//- <p>Objects implementing the NamedNodeMap interface are used to represent collections of nodes
//- that can be accessed by name. Note that NamedNodeMap does not inherit from NodeList;
//- NamedNodeMaps are not maintained in any particular order. Objects contained in an object
//- implementing NamedNodeMap may also be accessed by an ordinal index, but this is simply to
//- allow convenient enumeration of the contents of a NamedNodeMap, and does not imply that the
//- DOM specifies an order to these Nodes.</p>
class CL_API_CORE CL_DomNamedNodeMap
{
//! Construction:
public:
	//: Constructs a DOM NamedNodeMap handle.
	CL_DomNamedNodeMap();
	
	CL_DomNamedNodeMap(CL_DomNode &node);

	~CL_DomNamedNodeMap();
	
//! Attributes:
public:
	//: The number of nodes in the map.
	int get_length() const;
	
//! Operations:
public:
	//: Retrieves a node specified by name.
	CL_DomNode get_named_item(const std::string &name) const;

	//: Adds a node using its node name attribute.
	//- <p>As the node name attribute is used to derive the name which the node must be stored
	//- under, multiple nodes of certain types (those that have a "special" string value) cannot
	//- be stored as the names would clash. This is seen as preferable to allowing nodes to be
	//- aliased.</p>
	//param arg: A node to store in a named node map. The node will later be accessible using the value of the node name attribute of the node. If a node with that name is already present in the map, it is replaced by the new one.
	//retval: If the new Node replaces an existing node with the same name the previously existing Node is returned, otherwise null is returned.
	CL_DomNode set_named_item(const CL_DomNode &node);

	//: Removes a node specified by name.
	//- <p>If the removed node is an Attr with a default value it is immediately replaced.</p>
	//param name: The name of a node to remove.
	//retval: The node removed from the map or null if no node with such a name exists.
	CL_DomNode remove_named_item(const std::string &name);

	//: Returns the indexth item in the map.
	//- <p>If index is greater than or equal to the number of nodes in the map, this returns null.</p>
	CL_DomNode item(unsigned long index) const;
	
//! Implementation:
private:
	CL_SharedPtr<CL_DomNamedNodeMap_Generic> impl;
};

#endif
