#include "AppPrecomp.h"
#include "PointList.h"
#include "MaterialManager.h"
#include "AppUtils.h"
#include "PhysicsManager.h"

string PointList::ErrorToString(PointList::eVertCheckStatus error)
{
	switch(error)
	{
	case STATUS_OK:
		return "OK"; break;
	
	case STATUS_NOT_ENOUGH_VERTS:
		return "Not enough verts"; break;

	case STATUS_TOO_MANY_VERTS:
		return "Too many vertexes"; break;
		
		
		case STATUS_TOO_SKINNY:
			return "Polygon too skinny"; break;
		case STATUS_BAD_EDGES:
			return "Bad edges, try adjusting the angles a bit."; break;
		case STATUS_RADIUS_LESS:
			return "Your shape has a radius/extent less than b2_toiSlop.  Adjust!"; break;
	}

	assert(0);
	return "Unknown";
	
}



//functions from inside Box2d...

static b2Vec2 ComputeCentroid(const b2Vec2* vs, int32 count)
{
	b2Assert(count >= 3);

	b2Vec2 c; c.Set(0.0f, 0.0f);
	float32 area = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);
#if 0
	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < count; ++i)
	{
		pRef += vs[i];
	}
	pRef *= 1.0f / count;
#endif

	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	// Centroid
	b2Assert(area > B2_FLT_EPSILON);
	c *= 1.0f / area;
	return c;
}

// http://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf
static void ComputeOBB(b2OBB* obb, const b2Vec2* vs, int32 count)
{
	b2Assert(count <= b2_maxPolygonVertices);
	b2Vec2 p[b2_maxPolygonVertices + 1];
	for (int32 i = 0; i < count; ++i)
	{
		p[i] = vs[i];
	}
	p[count] = p[0];

	float32 minArea = B2_FLT_MAX;

	for (int32 i = 1; i <= count; ++i)
	{
		b2Vec2 root = p[i-1];
		b2Vec2 ux = p[i] - root;
		float32 length = ux.Normalize();
		b2Assert(length > B2_FLT_EPSILON);
		b2Vec2 uy(-ux.y, ux.x);
		b2Vec2 lower(B2_FLT_MAX, B2_FLT_MAX);
		b2Vec2 upper(-B2_FLT_MAX, -B2_FLT_MAX);

		for (int32 j = 0; j < count; ++j)
		{
			b2Vec2 d = p[j] - root;
			b2Vec2 r;
			r.x = b2Dot(ux, d);
			r.y = b2Dot(uy, d);
			lower = b2Min(lower, r);
			upper = b2Max(upper, r);
		}

		float32 area = (upper.x - lower.x) * (upper.y - lower.y);
		if (area < 0.95f * minArea)
		{
			minArea = area;
			obb->R.col1 = ux;
			obb->R.col2 = uy;
			b2Vec2 center = 0.5f * (lower + upper);
			obb->center = root + b2Mul(obb->R, center);
			obb->extents = 0.5f * (upper - lower);
		}
	}

	b2Assert(minArea < B2_FLT_MAX);
}
//code originally by JamesTan - http://www.box2d.org/forum/viewtopic.php?f=3&t=720
PointList::eVertCheckStatus PointList::IsValidBox2DPolygon()
{
	// No vertices, return false
	if(m_points.empty() || m_points.size() < 2)
		return STATUS_NOT_ENOUGH_VERTS;
	if ( m_points.size() > b2_maxPolygonVertices)
	{
		return STATUS_TOO_MANY_VERTS;
	}

	if (GetRect().get_height() == 1 && GetRect().get_width() == 1)
	{
		//a special pass...
		//it's not ok, but a 1X1 box is used on things scaled up, which is ok in the end
		return STATUS_OK;
	}

	// Create our vertices
	std::vector<b2Vec2> vertices;
	std::vector<b2Vec2> normals;

	// Clear our vectors
	vertices.clear();
	normals.clear();

	// Assign the vertices
	point_list::iterator vertex_idx = m_points.begin();

	while (vertex_idx != m_points.end())
	{
		vertices.push_back(ToPhysicsSpace(*vertex_idx));
		vertex_idx++;
	}

	// Compute the normals
	for (unsigned int i = 0; i < (unsigned int)vertices.size(); i++)
	{
		unsigned int j = i + 1;

		int32 i1 = i;
		int32 i2 = i + 1 < vertices.size() ? i + 1 : 0;
		b2Vec2 edge = vertices[i2] - vertices[i1];

		if (edge.LengthSquared() < FLT_EPSILON * FLT_EPSILON)
			return STATUS_BAD_EDGES;

		b2Vec2 normal = b2Cross(edge, 1.f);
		normal.Normalize();
		normals.push_back(normal);
	}

	// Ensure the polygon is convex.
	for (unsigned int i = 0; i < (unsigned int)vertices.size(); ++i)
	{
		for (unsigned int j = 0; j < (unsigned int)vertices.size(); ++j)
		{
			// Don't check vertices on the current edge.
			if (j == i || j == (i + 1) % (unsigned int)vertices.size())
				continue;

			// Your polygon is non-convex (it has an indentation).
			// Or your polygon is too skinny.
			float s = b2Dot(normals[i], vertices[j] - vertices[i]);

			if (s > -b2_linearSlop)
				return STATUS_TOO_SKINNY;
		}
	}

	// Ensure the polygon is counter-clockwise.
	for (unsigned int i = 1; i < (unsigned int)vertices.size(); ++i)
	{
		float cross = b2Cross(normals[i-1], normals[i]);

		// Keep asinf happy.
		cross = b2Clamp(cross, -1.0f, 1.0f);

		// You have consecutive edges that are almost parallel on your polygon.
		float angle = asinf(cross);

		if (angle <= b2_angularSlop)
			return STATUS_BAD_EDGES;
	}

	// Compute the polygon centroid.
	b2Vec2 centroid = ComputeCentroid(&vertices[0], vertices.size());

	// Compute the oriented bounding box.
	b2OBB obb;
	ComputeOBB(&obb, &vertices[0], vertices.size());



	// Create core polygon shape by shifting edges inward.
	// Also compute the min/max radius for CCD.
	for (int32 i = 0; i < int(vertices.size()); ++i)
	{
		int32 i1 = i - 1 >= 0 ? i - 1 : vertices.size() - 1;
		int32 i2 = i;

		b2Vec2 n1 = normals[i1];
		b2Vec2 n2 = normals[i2];
		b2Vec2 v = vertices[i] - centroid;;

		b2Vec2 d;
		d.x = b2Dot(n1, v) - b2_toiSlop;
		d.y = b2Dot(n2, v) - b2_toiSlop;

		// Shifting the edge inward by b2_toiSlop should
		// not cause the plane to pass the centroid.

		// Your shape has a radius/extent less than b2_toiSlop.
		if (d.x < 0.0f || d.y <= 0.0f)
		{
			return STATUS_RADIUS_LESS;
		}
	
		b2Mat22 A;
		A.col1.x = n1.x; A.col2.x = n1.y;
		A.col1.y = n2.x; A.col2.y = n2.y;
		A.Solve(d) + centroid;
	}
	return STATUS_OK;
}




//for SimpleHull2D function:
//Copyright 2002, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.


// Assume that a class is already given for the object:
//    Point with coordinates {float x, y;}
//===================================================================


// isLeft(): test if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
inline float
isLeft( CL_Vector2 P0, CL_Vector2 P1, CL_Vector2 P2 )
{
	return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
}


// simpleHull_2D():
//    Input:  V[] = polyline array of 2D vertex points 
//            n   = the number of points in V[]
//    Output: H[] = output convex hull array of vertices (max is n)
//    Return: h   = the number of points in H[]
int simpleHull_2D( CL_Vector2* V, int n, CL_Vector2* H )
{
	// initialize a deque D[] from bottom to top so that the
	// 1st three vertices of V[] are a counterclockwise triangle
	CL_Vector2* D = new CL_Vector2[2*n+1];
	int bot = n-2, top = bot+3;   // initial bottom and top deque indices
	D[bot] = D[top] = V[2];       // 3rd vertex is at both bot and top
	if (isLeft(V[0], V[1], V[2]) > 0) {
		D[bot+1] = V[0];
		D[bot+2] = V[1];          // ccw vertices are: 2,0,1,2
	}
	else {
		D[bot+1] = V[1];
		D[bot+2] = V[0];          // ccw vertices are: 2,1,0,2
	}

	// compute the hull on the deque D[]
	for (int i=3; i < n; i++) {   // process the rest of vertices
		// test if next vertex is inside the deque hull
		if ((isLeft(D[bot], D[bot+1], V[i]) > 0) &&
			(isLeft(D[top-1], D[top], V[i]) > 0) )
			continue;         // skip an interior vertex

		// incrementally add an exterior vertex to the deque hull
		// get the rightmost tangent at the deque bot
		while (isLeft(D[bot], D[bot+1], V[i]) <= 0)
			++bot;                // remove bot of deque
		D[--bot] = V[i];          // insert V[i] at bot of deque

		// get the leftmost tangent at the deque top
		while (isLeft(D[top-1], D[top], V[i]) <= 0)
			--top;                // pop top of deque
		D[++top] = V[i];          // push V[i] onto top of deque
	}

	// transcribe deque D[] to the output hull array H[]
	int h;        // hull vertex counter
	for (h=0; h <= (top-bot); h++)
		H[h] = D[bot + h];

	delete D;
	return h-1;
}


void PointList::RemoveDuplicateVerts()
{
	point_list::iterator itor = m_points.begin();

	while (itor != m_points.end())
	{

		point_list::iterator itorFound = std::find(itor+1, m_points.end(), *itor);
	
		if (itorFound != m_points.end())
		{
			//uh oh, it has a dupe.  Erase this one
			itor = m_points.erase(itor);
			continue;
		}

		itor++;
	}
}

void PointList::ApplyScale(const CL_Vector2 &vScale)
{

	RemoveOffsets();
	
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i].x *= vScale.x;
		m_points[i].y *= vScale.y;
	}

	CalculateOffsets();

	m_bNeedsToRecalculateRect = true;

}

void PointList::ApplyOffset(const CL_Vector2 &vOffset)
{
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] += vOffset;
	}
	m_bNeedsToRecalculateRect = true;
}

bool PointList::ComputeConvexHull()
{
	m_bNeedsToRecalculateRect = true;

	if (m_points.size() < 2)
	{
		LogMsg("Not enough points, aborting computing convex hull.");
		return false;
	}

	if (m_points.size() > 3)
	{
		point_list ptsVec;
		ptsVec.resize(m_points.size()+1, CL_Vector2(0,0));
		simpleHull_2D(&m_points[0], m_points.size(), &ptsVec[0]);

		//copy the new points over
		for (unsigned int i=0; i < m_points.size(); i++)
		{
			m_points[i] = ptsVec[i];
		}
	}

	RemoveDuplicateVerts();
	return true;
}


PointList::PointList()
{
	m_bNeedsToRecalculateRect = true;
	m_type = C_POINT_LIST_HARD;	
    m_vecOffset = CL_Vector2(0,0);
}

PointList::~PointList()
{
}

const CL_Rectf & PointList::GetRect()
{
	if (m_bNeedsToRecalculateRect)
	{
		BuildBoundingRect();
	}

	return m_boundingRect;
}

void PointList::SetType(int myType)
{
	m_type = myType;
	if (m_type >= g_materialManager.GetCount())
	{
		LogMsg("Warning: Material index %d not initted, changing it to 0", m_type);
		m_type = 0;
	}
}

bool PointList::BuildBoundingRect()
{
	if (!HasData()) 
	{
		m_boundingRect.left = m_boundingRect.right = m_boundingRect.top = m_boundingRect.bottom = 0;	
		return false;
	}

	m_bNeedsToRecalculateRect = false;

	m_boundingRect.left = FLT_MAX;
	m_boundingRect.top = FLT_MAX;
	m_boundingRect.bottom = -FLT_MAX;
	m_boundingRect.right = -FLT_MAX;

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_boundingRect.left = min(m_boundingRect.left, m_points[i].x + m_vecOffset.x);
		m_boundingRect.right = max(m_boundingRect.right, m_points[i].x + m_vecOffset.x);

		m_boundingRect.top = min(m_boundingRect.top, m_points[i].y + m_vecOffset.y);
		m_boundingRect.bottom = max(m_boundingRect.bottom, m_points[i].y + m_vecOffset.y);
	}
	return true;
}

void PointList::RemoveOffsets()
{
	assert(m_vecOffset == CL_Vector2(0,0));
return;
	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] += m_vecOffset;
	}

	m_vecOffset = CL_Vector2(0,0);
	m_bNeedsToRecalculateRect = true;
}

void PointList::CalculateOffsets()
{
		return;
	if (m_points.size() == 0) return;

	m_bNeedsToRecalculateRect = true; //force recalulation to happen now	

	CL_Vector2 upperLeftBounds = CL_Vector2(100000,100000);

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		upperLeftBounds.x = min(upperLeftBounds.x, m_points[i].x);
		upperLeftBounds.y = min(upperLeftBounds.y, m_points[i].y);
	}

	m_vecOffset = upperLeftBounds + CL_Vector2(GetRect().get_width()/2, GetRect().get_height()/2);

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		m_points[i] -= m_vecOffset;
	}
	
}

void PointList::PrintPoints()
{
	LogMsg("Offset is %.2f, %.2f", m_vecOffset.x, m_vecOffset.y);

	string s;

	for (unsigned int i=0; i < m_points.size(); i++)
	{
		s = "Vert #" + CL_String::from_int(i) + ": "+ PrintVector(m_points[i]);
		LogMsg(s.c_str());
	}

}


//stolen from jyk's snippet from GDNet: http://www.gamedev.net/community/forums/topic.asp?topic_id=304578

bool IntersectCircleSegment(const CL_Vector2& c,        // center
							float r,                            // radius
							const CL_Vector2& p1,     // segment start
							const CL_Vector2& p2)     // segment end
{
	CL_Vector2 dir = p2 - p1;
	CL_Vector2 diff = c - p1;
	float t = diff.dot(dir) / dir.dot(dir);
	if (t < 0.0f)
		t = 0.0f;
	if (t > 1.0f)
		t = 1.0f;
	CL_Vector2 closest = p1 + (dir*t);
	CL_Vector2 d = c - closest;
	float distsqr = d.dot(d);
	return distsqr <= r * r;
}

bool PointList::GetCircleIntersection(const CL_Vector2 &c, float radius)
{
	for (unsigned int i=0; i < GetPointList()->size();)
	{
		int endLineIndex;

		if (i == GetPointList()->size()-1)
		{
			endLineIndex=0;
		} else
		{
			endLineIndex = i+1;
		}

		if (IntersectCircleSegment(c, radius, GetPointList()->at(i), GetPointList()->at(endLineIndex)))
		{
			return true; 
		}
		
		i++;
	}

	return false;
}

void PointList::GetAsPolygonDef(b2PolygonDef *shapeDef )
{
	for (unsigned int i=0; i < GetPointList()->size(); i++)
	{
		shapeDef->vertices[i].x = m_points[i].x / C_PHYSICS_PIXEL_SIZE;
		shapeDef->vertices[i].y = m_points[i].y / C_PHYSICS_PIXEL_SIZE;
	}

	shapeDef->vertexCount = GetPointList()->size();

	shapeDef->restitution = g_materialManager.GetMaterial(m_type)->GetRestitution();
	shapeDef->friction = g_materialManager.GetMaterial(m_type)->GetFriction();
}

bool PointList::GetLineIntersection(const CL_Vector2 &a, const CL_Vector2 &b)
{
	float lineA[4], lineB[4];
	lineA[0] = a.x;
	lineA[1] = a.y;

	lineA[2] = b.x;
	lineA[3] = b.y;

		for (unsigned int i=0; i < GetPointList()->size();)
			{
	
					lineB[0] = GetPointList()->at(i).x;
					lineB[1] = GetPointList()->at(i).y;

					int endLineIndex;

					if (i == GetPointList()->size()-1)
					{
						endLineIndex=0;
					} else
					{
						endLineIndex = i+1;
					}

					lineB[2] = GetPointList()->at(endLineIndex).x;
					lineB[3] = GetPointList()->at(endLineIndex).y;

				
				//LogMsg("Line B is %.2f, %.2f - %.2f, %.2f", lineB[0],lineB[1],lineB[2],lineB[3] );
				if (CL_LineMath::intersects(lineA, lineB))
				{
					
					//LogMsg("Got intersection");
					//ptCol = CL_LineMath::get_intersection(lineA, lineB);
					return true;
				}
				i++;
			}
		

	return false;
}
