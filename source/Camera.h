#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
			
		}


		Vector3 origin{};
		float fovAngle{90.f};

		

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			// rightx,   righty,   rightz,   0
			// upx,      upy,      upz,      0
			// forwardx, forwardy, forwardz, 0
			// originx,  originy,  originz,  1


			return { {Vector3::Cross(up, forward).Normalized() ,0.f },
					 {Vector3::Cross(forward, right).Normalized() ,0.f},
					 {forward, 0.f }, 
					 {origin,  1.f }
				   };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
