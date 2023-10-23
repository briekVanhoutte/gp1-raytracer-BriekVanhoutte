#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update();

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void RenderGradient(int px, int py) const;

		void ToggleShadows();
		void CycleLightingMode();

	private:
		enum class LightingMode {
			ObservedArea, //lambert cosine law
			Radiance, // incident Radiance
			BRDF, // scatering of the light
			Combined // ObservedArea * Radiance * BRDF
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ false };

		bool m_F2Pressed{ false };
		bool m_F3Pressed{ false };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
	};
}
