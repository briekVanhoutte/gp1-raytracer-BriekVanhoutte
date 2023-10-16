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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float fovValue = tanf((TO_RADIANS * camera.fovAngle) / 2);
	const Matrix matrixToWorld = camera.CalculateCameraToWorld();

	float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	Vector3 rayDirectionApplydCameraMovement;

	Ray viewRay;
	viewRay.origin = camera.origin;
	ColorRGB finalColor;

	for (int px = 0; px < m_Width; ++px)
	{
		for (int py = 0; py < m_Height; ++py)
		{
			float ndcX = (2.0f * (px + 0.5f) / m_Width - 1.0f) * aspect * fovValue;
			float ndcY = (1.0f - 2.0f * (py + 0.5f) / m_Height) * fovValue;

			Vector3 rayDirection(ndcX, ndcY, 0.7f);
			rayDirection.Normalize();
			rayDirectionApplydCameraMovement = matrixToWorld.TransformVector(rayDirection);

			viewRay.direction = rayDirectionApplydCameraMovement;
			finalColor = {};

			HitRecord closestHit;
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				bool shadow = false;

				ColorRGB totalLightColor = {};

				

				for (const Light& l : lights)
				{
					float angleCos = 1.f;
					
					switch (l.type)
					{
					case LightType::Point:
						angleCos = Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(l, closestHit.origin));
						break;
					case LightType::Directional:
						angleCos = Vector3::Dot(closestHit.normal, l.direction);
						break;
					default:
						
						break;
					}
					if (angleCos >= 0) {
						totalLightColor += LightUtils::GetRadiance(l, closestHit.origin, closestHit.normal) * 
										   materials[closestHit.materialIndex]->Shade(closestHit, -LightUtils::GetDirectionToLight(l, closestHit.origin) ,  camera.forward) *
										   angleCos;

						if (m_ShadowsEnabled) {
							Vector3 originPointRay = closestHit.origin + closestHit.normal * 0.001f;
							Vector3 raydir = LightUtils::GetDirectionToLight(l, originPointRay);
							float rayMagnitude = raydir.Magnitude();
							raydir.Normalize();

							Ray raytoLight(originPointRay, raydir);
							raytoLight.max = rayMagnitude;


							if (pScene->DoesHit(raytoLight))
							{
								shadow = true;

							}
						}

					}
					

					
				}
				totalLightColor.MaxToOne();
				//finalColor = materials[closestHit.materialIndex]->Shade();
				finalColor += totalLightColor;

				if (shadow)
				{
					finalColor *= 0.5f;
				}
			}

			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	SDL_UpdateWindowSurface(m_pWindow);
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