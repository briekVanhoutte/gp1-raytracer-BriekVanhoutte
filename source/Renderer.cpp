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

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// calc aspect ratio width over height
			float aspect =float( m_Width) / float(m_Height);

			/*
				calc direction of ray for every pixel from camera location to pixel pixel location
				pixel location calculated based on center of pixel +0.5f, and camera needs to look at center of the screen so some shenanigans to make that work
			*/ 

			Vector3 rayDirection({0,0,0}, { (2 * (((px + 0.5f)) / m_Width) - 1) * aspect * fovValue ,
												(1 - 2 * ( ( py + 0.5f ) ) / m_Height) * fovValue,
												 0.7f });
			
			rayDirection.Normalize();

			Ray viewRay(camera.origin, rayDirection);

			ColorRGB finalColor{};

			//HitRecord containing more information about a potential hit
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);
			//Sphere testSphere{ {0.f,0.f,100.f},50.f,0 };

			//GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if (closestHit.didHit)
			{
				{
					finalColor = materials[closestHit.materialIndex]->Shade();
				}
			}

			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
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

void Renderer::ShootRayEachPixel(int px, int py, Scene* pScene) const
{


}