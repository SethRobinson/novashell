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
**    Magic Software - www.magic-software.com
*/

inline CL_Vector2::CL_Vector2 ()
{
    // For efficiency in construction of large arrays of vectors, the
    // default constructor does not initialize the vector.
}

inline const float& CL_Vector2::operator[](int i) const
{
    return ((const float*)this)[i];
}

inline float& CL_Vector2::operator[](int i)
{
    return ((float*)this)[i];
}

inline CL_Vector2::operator float*()
{
    return (float*)this;
}

inline float CL_Vector2::squared_length() const
{
    return x*x + y*y;
}
