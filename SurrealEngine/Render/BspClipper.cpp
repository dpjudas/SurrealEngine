
#include "Precomp.h"
#include "BspClipper.h"
#include "Math/bbox.h"

#ifndef NOSSE
#include <immintrin.h>
#endif

BspClipper::BspClipper()
{
	Viewport.resize(ViewportHeight);
}

BspClipper::~BspClipper()
{
}

void BspClipper::Setup(const mat4& world_to_projection)
{
	WorldToProjection = world_to_projection;
	FrustumClip = FrustumPlanes(world_to_projection);

	ClipSpan left = { (int16_t)0x8000, 0 };
	ClipSpan right = { (int16_t)ViewportWidth, (int16_t)0x7fff };
	for (auto& line : Viewport)
	{
		line.clear();
		line.push_back(left);
		line.push_back(right);
	}
}

bool BspClipper::CheckSurface(const vec3* vertices, uint32_t count, bool solid)
{
	if (count < 3)
		return false;

	numSurfs++;

	vec4 buffer[3];
	vec4* triverts[3] = { &buffer[0], &buffer[1], &buffer[2] };

	bool result = false;
	*triverts[0] = WorldToProjection * vec4(vertices[0], 1.0f);
	*triverts[1] = WorldToProjection * vec4(vertices[1], 1.0f);
	for (uint32_t i = 2; i < count; i++)
	{
		*triverts[2] = WorldToProjection * vec4(vertices[i], 1.0f);
		result |= DrawTriangle(triverts, solid, false);
		std::swap(triverts[1], triverts[2]);
	}
	return result;
}

bool BspClipper::IsAABBVisible(const BBox& bbox)
{
	// First quickly check if we can rule it out using the frustum planes
	int bits = FrustumClip.testIntersecting(bbox);
	if (bits == -1) // outside
		return false;

	if (bits & (1 << 0)) // Near plane intersecting with bbox
		return true;

	// Perform visibility check of the bounding box itself using the spans:

	// Vertices of the AABB
	vec4 verts[8] =
	{
		WorldToProjection * vec4(bbox.min.x, bbox.min.y, bbox.min.z, 1.0f),
		WorldToProjection * vec4(bbox.max.x, bbox.min.y, bbox.min.z, 1.0f),
		WorldToProjection * vec4(bbox.min.x, bbox.max.y, bbox.min.z, 1.0f),
		WorldToProjection * vec4(bbox.max.x, bbox.max.y, bbox.min.z, 1.0f),
		WorldToProjection * vec4(bbox.min.x, bbox.min.y, bbox.max.z, 1.0f),
		WorldToProjection * vec4(bbox.max.x, bbox.min.y, bbox.max.z, 1.0f),
		WorldToProjection * vec4(bbox.min.x, bbox.max.y, bbox.max.z, 1.0f),
		WorldToProjection * vec4(bbox.max.x, bbox.max.y, bbox.max.z, 1.0f)
	};

	float viewport_x = 0.0f;
	float viewport_y = 0.0f;
	float viewport_width = (float)ViewportWidth;
	float viewport_height = (float)ViewportHeight;

	// Map to 2D viewport:
	for (int j = 0; j < 8; j++)
	{
		auto& v = verts[j];

		// Calculate normalized device coordinates:
		v.w = 1.0f / v.w;
		v.x *= v.w;
		v.y *= v.w;
		v.z *= v.w;

		// Apply viewport scale to get screen coordinates:
		v.x = viewport_x + viewport_width * (1.0f + v.x) * 0.5f;
		v.y = viewport_y + viewport_height * (1.0f - v.y) * 0.5f;
	}

	// Find the 2D bounding box

	vec2 min2d = verts[0].xy();
	vec2 max2d = verts[0].xy();
	for (int j = 1; j < 8; j++)
	{
		min2d.x = std::min(min2d.x, verts[j].x);
		min2d.y = std::min(min2d.y, verts[j].y);
		max2d.x = std::max(max2d.x, verts[j].x);
		max2d.y = std::max(max2d.y, verts[j].y);
	}

	// if we are intersecting with any of the sides then include the entire edge
	if (bits & (1 << 2)) min2d.x = 0;
	if (bits & (1 << 3)) max2d.x = (float)ViewportWidth;
	if (bits & (1 << 4)) min2d.y = 0;
	if (bits & (1 << 5)) max2d.y = (float)ViewportHeight;

	// Check if any of it can be seen:

	int topY = std::max((int)(min2d.y + 0.5f), 0);
	int bottomY = std::min((int)(max2d.y + 0.5f), ViewportHeight);
	if (topY >= bottomY)
		return false;
	int x0 = clamp((int)min2d.x, 0, ViewportWidth);
	int x1 = clamp((int)max2d.x, 0, ViewportWidth);
	if (x0 >= x1)
		return false;
	for (int y = topY; y < bottomY; y++)
	{
		if (IsVisible(y, x0, x1))
			return true;
	}
	return false;
}

bool BspClipper::IsVisible(int16_t y, int16_t x0, int16_t x1)
{
	auto& line = Viewport[y];
	for (size_t pos = 0; pos < line.size(); pos++)
	{
		int16_t left = line[pos].x1;
		if (left >= x1)
			break;
		int16_t right = line[pos + 1].x0;

		left = std::max(left, x0);
		right = std::min(right, x1);
		if (left < right)
		{
			return true;
		}
	}
	return false;
}

bool BspClipper::DrawSpan(int16_t y, int16_t x0, int16_t x1, bool solid)
{
	if (x1 <= x0)
		return false;

	if (!solid)
		return IsVisible(y, x0, x1);

	numDrawSpans++;

	auto& line = Viewport[y];

	bool visible = false;
	for (size_t pos = 0; pos < line.size(); pos++)
	{
		ClipSpan& span = line[pos];
		ClipSpan& nextspan = line[pos + 1];

		int16_t left = span.x1;
		if (left >= x1)
			break;
		int16_t right = nextspan.x0;

		left = std::max(left, x0);
		right = std::min(right, x1);
		if (left < right)
		{
			visible = true;
			if (left == span.x1)
			{
				span.x1 = right;
				if (span.x1 == nextspan.x0)
				{
					span.x1 = nextspan.x1;
					line.erase(line.begin() + pos + 1);
				}
			}
			else if (right == nextspan.x0)
			{
				nextspan.x0 = left;
				if (span.x1 == nextspan.x0)
				{
					span.x1 = nextspan.x1;
					line.erase(line.begin() + pos + 1);
				}
			}
			else
			{
				pos++;
				ClipSpan span;
				span.x0 = left;
				span.x1 = right;
				line.insert(line.begin() + pos, span);
			}
		}
	}
	return visible;
}

bool BspClipper::DrawTriangle(const vec4* const* vert, bool solid, bool ccw)
{
	// Reject triangle if degenerate
	//if (IsDegenerate(vert))
	//	return false;

	numTris++;

	// Cull, clip and generate additional vertices as needed
	vec4 clippedvert[max_additional_vertices];
	int numclipvert = ClipEdge(vert);

	// Convert barycentric weights to actual vertices
	for (int i = 0; i < numclipvert; i++)
	{
		auto& v = clippedvert[i];
		v = vec4(0.0f);
		for (int w = 0; w < 3; w++)
		{
			float weight = weights[i * 3 + w];
			v.x += vert[w]->x * weight;
			v.y += vert[w]->y * weight;
			v.z += vert[w]->z * weight;
			v.w += vert[w]->w * weight;
		}
	}

	// Make viewport cover our spans
	float viewport_x = 0.0f;
	float viewport_y = 0.0f;
	float viewport_width = (float)ViewportWidth;
	float viewport_height = (float)ViewportHeight;

#ifdef NOSSE
	// Map to 2D viewport:
	for (int j = 0; j < numclipvert; j++)
	{
		auto& v = clippedvert[j];

		// Calculate normalized device coordinates:
		v.w = 1.0f / v.w;
		v.x *= v.w;
		v.y *= v.w;
		v.z *= v.w;

		// Apply viewport scale to get screen coordinates:
		v.x = viewport_x + viewport_width * (1.0f + v.x) * 0.5f;
		v.y = viewport_y + viewport_height * (1.0f - v.y) * 0.5f;
	}
#else
	// Map to 2D viewport:
	__m128 mviewport_x = _mm_set1_ps((float)viewport_x);
	__m128 mviewport_y = _mm_set1_ps((float)viewport_y);
	__m128 mviewport_halfwidth = _mm_set1_ps(viewport_width * 0.5f);
	__m128 mviewport_halfheight = _mm_set1_ps(viewport_height * 0.5f);
	__m128 mone = _mm_set1_ps(1.0f);
	int sse_length = (numclipvert + 3) / 4 * 4;
	for (int j = 0; j < sse_length; j += 4)
	{
		__m128 vx = _mm_loadu_ps(&clippedvert[j].x);
		__m128 vy = _mm_loadu_ps(&clippedvert[j + 1].x);
		__m128 vz = _mm_loadu_ps(&clippedvert[j + 2].x);
		__m128 vw = _mm_loadu_ps(&clippedvert[j + 3].x);
		_MM_TRANSPOSE4_PS(vx, vy, vz, vw);

		// Calculate normalized device coordinates:
		vw = _mm_div_ps(mone, vw);
		vx = _mm_mul_ps(vx, vw);
		vy = _mm_mul_ps(vy, vw);
		vz = _mm_mul_ps(vz, vw);

		// Apply viewport scale to get screen coordinates:
		vx = _mm_add_ps(mviewport_x, _mm_mul_ps(mviewport_halfwidth, _mm_add_ps(mone, vx)));
		vy = _mm_add_ps(mviewport_y, _mm_mul_ps(mviewport_halfheight, _mm_sub_ps(mone, vy)));

		_MM_TRANSPOSE4_PS(vx, vy, vz, vw);
		_mm_storeu_ps(&clippedvert[j].x, vx);
		_mm_storeu_ps(&clippedvert[j + 1].x, vy);
		_mm_storeu_ps(&clippedvert[j + 2].x, vz);
		_mm_storeu_ps(&clippedvert[j + 3].x, vw);
	}
#endif

	/*if (twosided && numclipvert > 2)
	{
		ccw = !IsFrontfacing(clippedvert);
	}*/

	// Draw screen triangles
	bool result = false;
	if (ccw)
	{
		for (int i = numclipvert - 1; i > 1; i--)
		{
			vec4* args[3];
			args[0] = &clippedvert[numclipvert - 1];
			args[1] = &clippedvert[i - 1];
			args[2] = &clippedvert[i - 2];
			//if (IsFrontfacing(args) == ccw)
			{
				result |= DrawClippedTriangle(args, solid);
			}
		}
	}
	else
	{
		for (int i = 2; i < numclipvert; i++)
		{
			vec4* args[3];
			args[0] = &clippedvert[0];
			args[1] = &clippedvert[i - 1];
			args[2] = &clippedvert[i];
			//if (IsFrontfacing(args) != ccw)
			{
				result |= DrawClippedTriangle(args, solid);
			}
		}
	}
	return result;
}

int BspClipper::ClipEdge(const vec4* const* verts)
{
	bool DepthClamp = false;

	// use barycentric weights for clipped vertices
	weights = weightsbuffer;
	for (int i = 0; i < 3; i++)
	{
		weights[i * 3 + 0] = 0.0f;
		weights[i * 3 + 1] = 0.0f;
		weights[i * 3 + 2] = 0.0f;
		weights[i * 3 + i] = 1.0f;
	}

	// Clip and cull so that the following is true for all vertices:
	// -v.w <= v.x <= v.w
	// -v.w <= v.y <= v.w
	// -v.w <= v.z <= v.w

	// halfspace clip distances
	static const int numclipdistances = 6; // 9;
#ifdef NOSSE
	float clipdistance[numclipdistances * 3];
	bool needsclipping = false;
	float* clipd = clipdistance;
	for (int i = 0; i < 3; i++)
	{
		const auto& v = *verts[i];
		clipd[0] = v.x + v.w;
		clipd[1] = v.w - v.x;
		clipd[2] = v.y + v.w;
		clipd[3] = v.w - v.y;
		if (DepthClamp)
		{
			clipd[4] = 1.0f;
			clipd[5] = 1.0f;
		}
		else
		{
			clipd[4] = v.z + v.w;
			clipd[5] = v.w - v.z;
		}
		//clipd[6] = v.gl_ClipDistance[0];
		//clipd[7] = v.gl_ClipDistance[1];
		//clipd[8] = v.gl_ClipDistance[2];
		for (int j = 0; j < numclipdistances; j++)
			needsclipping = needsclipping || clipd[i];
		clipd += numclipdistances;
	}

	// If all halfspace clip distances are positive then the entire triangle is visible. Skip the expensive clipping step.
	if (!needsclipping)
	{
		return 3;
	}
#else
	__m128 mx = _mm_loadu_ps(&verts[0]->x);
	__m128 my = _mm_loadu_ps(&verts[1]->x);
	__m128 mz = _mm_loadu_ps(&verts[2]->x);
	__m128 mw = _mm_setzero_ps();
	_MM_TRANSPOSE4_PS(mx, my, mz, mw);
	__m128 clipd0 = _mm_add_ps(mx, mw);
	__m128 clipd1 = _mm_sub_ps(mw, mx);
	__m128 clipd2 = _mm_add_ps(my, mw);
	__m128 clipd3 = _mm_sub_ps(mw, my);
	__m128 clipd4 = DepthClamp ? _mm_set1_ps(1.0f) : _mm_add_ps(mz, mw);
	__m128 clipd5 = DepthClamp ? _mm_set1_ps(1.0f) : _mm_sub_ps(mw, mz);
	//__m128 clipd6 = _mm_setr_ps(verts[0]->gl_ClipDistance[0], verts[1]->gl_ClipDistance[0], verts[2]->gl_ClipDistance[0], 0.0f);
	//__m128 clipd7 = _mm_setr_ps(verts[0]->gl_ClipDistance[1], verts[1]->gl_ClipDistance[1], verts[2]->gl_ClipDistance[1], 0.0f);
	//__m128 clipd8 = _mm_setr_ps(verts[0]->gl_ClipDistance[2], verts[1]->gl_ClipDistance[2], verts[2]->gl_ClipDistance[2], 0.0f);
	__m128 mneedsclipping = _mm_cmplt_ps(clipd0, _mm_setzero_ps());
	mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd1, _mm_setzero_ps()));
	mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd2, _mm_setzero_ps()));
	mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd3, _mm_setzero_ps()));
	mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd4, _mm_setzero_ps()));
	mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd5, _mm_setzero_ps()));
	//mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd6, _mm_setzero_ps()));
	//mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd7, _mm_setzero_ps()));
	//mneedsclipping = _mm_or_ps(mneedsclipping, _mm_cmplt_ps(clipd8, _mm_setzero_ps()));
	if (_mm_movemask_ps(mneedsclipping) == 0)
	{
		return 3;
	}
	float clipdistance[numclipdistances * 4];
	_mm_storeu_ps(clipdistance, clipd0);
	_mm_storeu_ps(clipdistance + 4, clipd1);
	_mm_storeu_ps(clipdistance + 8, clipd2);
	_mm_storeu_ps(clipdistance + 12, clipd3);
	_mm_storeu_ps(clipdistance + 16, clipd4);
	_mm_storeu_ps(clipdistance + 20, clipd5);
	//_mm_storeu_ps(clipdistance + 24, clipd6);
	//_mm_storeu_ps(clipdistance + 28, clipd7);
	//_mm_storeu_ps(clipdistance + 32, clipd8);
#endif

	// Clip against each halfspace
	float* input = weights;
	float* output = weights + max_additional_vertices * 3;
	int inputverts = 3;
	for (int p = 0; p < numclipdistances; p++)
	{
		// Clip each edge
		int outputverts = 0;
		for (int i = 0; i < inputverts; i++)
		{
			int j = (i + 1) % inputverts;
#ifdef NOSSE
			float clipdistance1 =
				clipdistance[0 * numclipdistances + p] * input[i * 3 + 0] +
				clipdistance[1 * numclipdistances + p] * input[i * 3 + 1] +
				clipdistance[2 * numclipdistances + p] * input[i * 3 + 2];

			float clipdistance2 =
				clipdistance[0 * numclipdistances + p] * input[j * 3 + 0] +
				clipdistance[1 * numclipdistances + p] * input[j * 3 + 1] +
				clipdistance[2 * numclipdistances + p] * input[j * 3 + 2];
#else
			float clipdistance1 =
				clipdistance[0 + p * 4] * input[i * 3 + 0] +
				clipdistance[1 + p * 4] * input[i * 3 + 1] +
				clipdistance[2 + p * 4] * input[i * 3 + 2];

			float clipdistance2 =
				clipdistance[0 + p * 4] * input[j * 3 + 0] +
				clipdistance[1 + p * 4] * input[j * 3 + 1] +
				clipdistance[2 + p * 4] * input[j * 3 + 2];
#endif

			// Clip halfspace
			if ((clipdistance1 >= 0.0f || clipdistance2 >= 0.0f) && outputverts + 1 < max_additional_vertices)
			{
				float t1 = (clipdistance1 < 0.0f) ? std::max(-clipdistance1 / (clipdistance2 - clipdistance1), 0.0f) : 0.0f;
				float t2 = (clipdistance2 < 0.0f) ? std::min(1.0f + clipdistance2 / (clipdistance1 - clipdistance2), 1.0f) : 1.0f;

				// add t1 vertex
				for (int k = 0; k < 3; k++)
					output[outputverts * 3 + k] = input[i * 3 + k] * (1.0f - t1) + input[j * 3 + k] * t1;
				outputverts++;

				if (t2 != 1.0f && t2 > t1)
				{
					// add t2 vertex
					for (int k = 0; k < 3; k++)
						output[outputverts * 3 + k] = input[i * 3 + k] * (1.0f - t2) + input[j * 3 + k] * t2;
					outputverts++;
				}
			}
		}
		std::swap(input, output);
		inputverts = outputverts;
		if (inputverts == 0)
			break;
	}

	weights = input;
	return inputverts;
}

bool BspClipper::DrawClippedTriangle(const vec4* const* vertices, bool solid)
{
	// Sort vertices by Y position

	const vec4* sortedVertices[3];
	SortVertices(vertices, sortedVertices);

	int16_t clipleft = 0;
	int16_t clipright = ViewportWidth;
	int16_t cliptop = 0;
	int16_t clipbottom = ViewportHeight;

	int16_t topY = (int16_t)(sortedVertices[0]->y + 0.5f);
	int16_t midY = (int16_t)(sortedVertices[1]->y + 0.5f);
	int16_t bottomY = (int16_t)(sortedVertices[2]->y + 0.5f);

	topY = std::max(topY, cliptop);
	midY = std::min(midY, clipbottom);
	bottomY = std::min(bottomY, clipbottom);

	if (topY >= bottomY)
		return false;

	// Find start/end X positions for each line covered by the triangle:

	int16_t y = topY;

	float longDX = sortedVertices[2]->x - sortedVertices[0]->x;
	float longDY = sortedVertices[2]->y - sortedVertices[0]->y;
	float longStep = longDX / longDY;
	float longPos = sortedVertices[0]->x + longStep * (y + 0.5f - sortedVertices[0]->y) + 0.5f;

	bool result = false;

	if (y < midY)
	{
		float shortDX = sortedVertices[1]->x - sortedVertices[0]->x;
		float shortDY = sortedVertices[1]->y - sortedVertices[0]->y;
		float shortStep = shortDX / shortDY;
		float shortPos = sortedVertices[0]->x + shortStep * (y + 0.5f - sortedVertices[0]->y) + 0.5f;

		while (y < midY)
		{
			int16_t x0 = (int16_t)shortPos;
			int16_t x1 = (int16_t)longPos;
			if (x1 < x0) std::swap(x0, x1);
			x0 = clamp(x0, clipleft, clipright);
			x1 = clamp(x1, clipleft, clipright);

			result |= DrawSpan(y, x0, x1, solid);

			shortPos += shortStep;
			longPos += longStep;
			y++;
		}
	}

	if (y < bottomY)
	{
		float shortDX = sortedVertices[2]->x - sortedVertices[1]->x;
		float shortDY = sortedVertices[2]->y - sortedVertices[1]->y;
		float shortStep = shortDX / shortDY;
		float shortPos = sortedVertices[1]->x + shortStep * (y + 0.5f - sortedVertices[1]->y) + 0.5f;

		while (y < bottomY)
		{
			int16_t x0 = (int16_t)shortPos;
			int16_t x1 = (int16_t)longPos;
			if (x1 < x0) std::swap(x0, x1);
			x0 = clamp(x0, clipleft, clipright);
			x1 = clamp(x1, clipleft, clipright);

			result |= DrawSpan(y, x0, x1, solid);

			shortPos += shortStep;
			longPos += longStep;
			y++;
		}
	}

	return result;
}

void BspClipper::SortVertices(const vec4* const* vertices, const vec4** sortedVertices)
{
	sortedVertices[0] = vertices[0];
	sortedVertices[1] = vertices[1];
	sortedVertices[2] = vertices[2];

	if (sortedVertices[1]->y < sortedVertices[0]->y)
		std::swap(sortedVertices[0], sortedVertices[1]);
	if (sortedVertices[2]->y < sortedVertices[0]->y)
		std::swap(sortedVertices[0], sortedVertices[2]);
	if (sortedVertices[2]->y < sortedVertices[1]->y)
		std::swap(sortedVertices[1], sortedVertices[2]);
}

bool BspClipper::IsDegenerate(const vec4* const* vert)
{
	// A degenerate triangle has a zero cross product for two of its sides.
	float ax = vert[1]->x - vert[0]->x;
	float ay = vert[1]->y - vert[0]->y;
	float az = vert[1]->w - vert[0]->w;
	float bx = vert[2]->x - vert[0]->x;
	float by = vert[2]->y - vert[0]->y;
	float bz = vert[2]->w - vert[0]->w;
	float crossx = ay * bz - az * by;
	float crossy = az * bx - ax * bz;
	float crossz = ax * by - ay * bx;
	float crosslengthsqr = crossx * crossx + crossy * crossy + crossz * crossz;
	return crosslengthsqr <= 1.e-8f;
}

bool BspClipper::IsFrontfacing(const vec4* const* vert)
{
	float a =
		vert[0]->x * vert[1]->y - vert[1]->x * vert[0]->y +
		vert[1]->x * vert[2]->y - vert[2]->x * vert[1]->y +
		vert[2]->x * vert[0]->y - vert[0]->x * vert[2]->y;
	return a <= 0.0f;
}
