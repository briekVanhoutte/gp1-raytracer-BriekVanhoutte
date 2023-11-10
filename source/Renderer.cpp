//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <algorithm>

#include <execution>

#define PARALLEL_EXECUTION
using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();
	const float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	const float fovAngle = TO_RADIANS * camera.fovAngle;
	const float fov = tanf(fovAngle / 2);

#if defined(PARALLEL_EXECUTION)
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);

	for (uint32_t index{}; index < amountOfPixels; ++index) pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
			RenderPixel(pScene, i, fov, aspect, cameraToWorld, camera.origin);
			});
	
#else
	for (uint32_t i{}; i < m_Width * m_Height; ++i) {
		RenderPixel(pScene, i, fov, aspect, cameraToWorld, camera.origin);
	}

#endif

	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRation, const  Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	auto materials{ pScene->GetMaterials() };
	auto& lights = pScene->GetLights();

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	float rx{ px + 0.5f }, ry{ py + 0.5f };
	float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRation * fov };
	float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };

	float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	Vector3 rayDirectionApplydCameraMovement;

	Ray viewRay;
	viewRay.origin = cameraOrigin;
	ColorRGB finalColor;

	Vector3 rayDirection(cx, cy, 0.7f);
	rayDirection.Normalize();
	rayDirectionApplydCameraMovement = cameraToWorld.TransformVector(rayDirection);

	viewRay.direction = rayDirectionApplydCameraMovement;
	finalColor = {};

	HitRecord closestHit;
	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		bool shadow = false;
		int amountShadow = 0;
		const float shadowIncrease = 0.1f;
		ColorRGB totalLightColor = {};

		for (const Light& l : lights)
		{
			float angleCos = 1.f;
			ColorRGB irradiance = {};

			switch (l.type)
			{
			case LightType::Point:
				angleCos = Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(l, closestHit.origin).Normalized());
				irradiance = LightUtils::GetRadiance(l, closestHit.origin, closestHit.normal);
				break;
			case LightType::Directional:
				angleCos = Vector3::Dot(closestHit.normal, l.direction);
				irradiance = l.color * l.intensity;
				break;
			default:

				break;
			}

			Vector3 directionToHit = (cameraOrigin - closestHit.origin).Normalized();
			ColorRGB shading = {};

			switch (m_CurrentLightingMode)
			{
			case dae::Renderer::LightingMode::ObservedArea:
				if (angleCos > 0) {
					totalLightColor += ColorRGB{ 1.f,1.f,1.f } *angleCos;
				}
				break;
			case dae::Renderer::LightingMode::Radiance:
				if (!m_ShadowsEnabled)
				{
					totalLightColor += irradiance;
				}
				else
				{
					if (angleCos > 0) {
						totalLightColor += irradiance;
					}
				}
				break;
			case dae::Renderer::LightingMode::BRDF:
				if (angleCos > 0) {
					shading = materials[closestHit.materialIndex]->Shade(closestHit, LightUtils::GetDirectionToLight(l, closestHit.origin).Normalized(), directionToHit);
					totalLightColor += shading;
				}
				break;
			case dae::Renderer::LightingMode::Combined:
				if (angleCos > 0) {
					shading = materials[closestHit.materialIndex]->Shade(closestHit, LightUtils::GetDirectionToLight(l, closestHit.origin).Normalized(), directionToHit);
					totalLightColor += irradiance * shading * angleCos;
				}
				break;
			default:
				break;
			}

			if (m_ShadowsEnabled)
			{
				if (angleCos > 0) {
					Vector3 originPointRay = closestHit.origin + closestHit.normal * 0.001f; 
					Vector3 raydir = LightUtils::GetDirectionToLight(l, originPointRay);
					float rayMagnitude = raydir.Magnitude();
					raydir.Normalize();

					Ray raytoLight(originPointRay, raydir);
					raytoLight.max = rayMagnitude - 0.001f; 

					if (pScene->DoesHit(raytoLight))
					{
						amountShadow++;
						shadow = true;
					}
				}
			}
		}

		totalLightColor.MaxToOne();
		finalColor += totalLightColor;

		if (shadow)
		{
			finalColor *= 0.9f - amountShadow * shadowIncrease;
		}
	}

	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void  Renderer::RenderGradient(int px, int py) const
{
	float gradient = px / static_cast<float>(m_Width);
	gradient += py / static_cast<float>(m_Width);
	gradient /= 2.0f;

	ColorRGB finalColor{ gradient, gradient, gradient };

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}


void Renderer::Update()
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	if (pKeyboardState[SDL_SCANCODE_F2])
	{
		m_F2Pressed = true;
	}
	else
	{
		if (m_F2Pressed) ToggleShadows();
		m_F2Pressed = false;
	}
	if (pKeyboardState[SDL_SCANCODE_F3])
	{
		m_F3Pressed = true;
	}
	else
	{
		if (m_F3Pressed) CycleLightingMode();
		m_F3Pressed = false;
	}
}

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode) {
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		break;
	case LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	}
}

void Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}