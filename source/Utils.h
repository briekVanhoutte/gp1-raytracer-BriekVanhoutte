#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <math.h>

#include <iostream>
namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS


		inline bool hitTestSphereAnalytical(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord)
		{
			Vector3 sphereToRay = ray.origin - sphere.origin;
			float a = Vector3::Dot(ray.direction, ray.direction);
			float b = 2.0f * Vector3::Dot(ray.direction, sphereToRay);
			float c = Vector3::Dot(sphereToRay, sphereToRay) - (sphere.radius * sphere.radius);

			float discriminant = b * b - 4 * a * c;

			if (discriminant < 0) {
				return false;  // No intersection.
			}

			float sqrtDiscriminant = sqrtf(discriminant);
			float invA = 1.0f / (2.0f * a);

			float t0 = (-b - sqrtDiscriminant) * invA;
			float t1 = (-b + sqrtDiscriminant) * invA;

			if (t0 < ray.min || t0 > ray.max) {
				t0 = t1;  // Swap t0 and t1 if t0 is outside the valid range.
			}

			if (t0 >= ray.min && t0 <= ray.max && t0 < hitRecord.t) {
				hitRecord.t = t0;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin + ray.direction * t0;
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
				return true;
			}

			return false;  // No valid intersection found.
		}



		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			return hitTestSphereAnalytical(sphere, ray, hitRecord);
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}

		inline bool TestIfRayHitSphere(const Sphere& sphere, const Ray& ray) {
			Vector3 sphereToRay = ray.origin - sphere.origin;
			float a = Vector3::Dot(ray.direction, ray.direction);
			float b = 2.0f * Vector3::Dot(ray.direction, sphereToRay);
			float c = Vector3::Dot(sphereToRay, sphereToRay) - (sphere.radius * sphere.radius);

			float discriminant = b * b - 4 * a * c;

			if (discriminant < 0) {
				return false; 
			}

			float sqrtDiscriminant = sqrtf(discriminant);
			float invA = 1.0f / (2.0f * a);

			float t0 = (-b - sqrtDiscriminant) * invA;
			float t1 = (-b + sqrtDiscriminant) * invA;

			if ((t0 >= ray.min && t0 <= ray.max) || (t1 >= ray.min && t1 <= ray.max)) {
				return true; 
			}

			return false; 
		}




#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool TestIfRayHitPlane(const Plane& plane, const Ray& ray) {
			float denominator = Vector3::Dot(ray.direction, plane.normal);

			// Check if the ray is not parallel to the plane.
			if (denominator != 0.0f) {
				float t = Vector3::Dot(plane.origin - ray.origin, plane.normal) / denominator;

				// Check if the intersection point is within the valid range.
				if (t >= ray.min && t <= ray.max) {
					return true;
				}
			}

			return false;
		}


		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float dotDirectionNormal = Vector3::Dot(ray.direction, plane.normal);

			// Check if the ray and plane are not parallel
			if (dotDirectionNormal != 0)
			{
				const float t = Vector3::Dot((plane.origin - ray.origin), plane.normal) / dotDirectionNormal;

				// Check if t is within the ray's valid range
				if (t > ray.min && t < ray.max)
				{
					if (!ignoreHitRecord && t >= hitRecord.t)
					{
						return false;
					}

					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = plane.normal;

					return true;
				}
			}

			return false;
		}


		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false) {
			float dirDotNormal = Vector3::Dot(ray.direction, triangle.normal);

			// Early exit for parallel ray and triangle normal or back/front face culling
			if (dirDotNormal == 0 ||
				(!ignoreHitRecord && (
					(triangle.cullMode == TriangleCullMode::BackFaceCulling && dirDotNormal > 0) ||
					(triangle.cullMode == TriangleCullMode::FrontFaceCulling && dirDotNormal < 0)
					))) {
				return false;
			}

			Vector3 toVertex0 = triangle.v0 - ray.origin;
			float t = Vector3::Dot(toVertex0, triangle.normal) / dirDotNormal;

			// Check if the intersection is within the ray's bounds
			if (t < ray.min || t > ray.max || (!ignoreHitRecord && t >= hitRecord.t)) {
				return false;
			}

			// Calculate intersection point
			Vector3 P = ray.origin + (ray.direction * t);

			// Edge vectors
			Vector3 EdgeA = triangle.v1 - triangle.v0;
			Vector3 EdgeB = triangle.v2 - triangle.v1;
			Vector3 EdgeC = triangle.v0 - triangle.v2;

			// Vectors from point to vertices
			Vector3 C0 = P - triangle.v0;
			Vector3 C1 = P - triangle.v1;
			Vector3 C2 = P - triangle.v2;

			// Cross products
			Vector3 CrossA = Vector3::Cross(EdgeA, C0);
			Vector3 CrossB = Vector3::Cross(EdgeB, C1);
			Vector3 CrossC = Vector3::Cross(EdgeC, C2);

			// Perform the same-side test to determine if P is inside the triangle
			if ((Vector3::Dot(triangle.normal, CrossA) < 0) ||
				(Vector3::Dot(triangle.normal, CrossB) < 0) ||
				(Vector3::Dot(triangle.normal, CrossC) < 0)) {
				return false;
			}

			// At this point, the ray intersects the triangle
			// If ignoreHitRecord is true, no need to record the hit
			if (!ignoreHitRecord) {
				hitRecord.didHit = true;
				hitRecord.normal = triangle.normal;
				hitRecord.origin = P;
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
			}

			return true;
		}


		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			bool hitOccurred = false;
			for (int i{ 0 }; i < mesh.indices.size() / 3; ++i) {
				Triangle t = { mesh.transformedPositions[mesh.indices[i * 3]], mesh.transformedPositions[mesh.indices[i * 3 + 1 ]], mesh.transformedPositions[mesh.indices[i * 3 + 2]], mesh.transformedNormals[i]};
				t.cullMode = mesh.cullMode;
				t.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(t, ray, hitRecord)) {
					hitOccurred = true;

					if (!ignoreHitRecord) break;
				}
			}

			return hitOccurred;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			return Vector3(origin,light.origin);
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target, const Vector3& surfaceNormal)
		{

			Vector3 direction = light.origin - target;

			return light.color * (light.intensity/(direction.SqrMagnitude()));
		}

	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}