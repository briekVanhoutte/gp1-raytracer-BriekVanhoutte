#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>
#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{

		}


		Vector3 origin{};
		float fovAngle{ 90.f };



		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			// rightx,   righty,   rightz,   0
			// upx,      upy,      upz,      0
			// forwardx, forwardy, forwardz, 0
			// originx,  originy,  originz,  1

			right = Vector3::Cross(up, forward).Normalized();
			right.y = 0; // force now roll but is this correct idk??
			up = Vector3::Cross(forward, right).Normalized();

			return { {right ,0.f },
					 { up,0.f},
					 {forward, 0.f },
					 {origin,  1.f }
			};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const float rotationSpeed = 10.f;
			const float movementSpeed = 10.f;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float movementSpeedFinal = movementSpeed * deltaTime;
			float rotationspeedFinal = rotationSpeed * deltaTime;

			bool lmb = false;
			bool rmb = false;
			bool mmb = false;

			if (mouseState & SDL_BUTTON_LMASK) 
			{
				//std::cout << "LMB pressed!" << std::endl; 
				lmb = true; 
			}
			if (mouseState & SDL_BUTTON_RMASK) 
			{
				//std::cout << "RMB pressed!" << std::endl; 
				rmb = true;
			}
			if (mouseState & SDL_BUTTON_MMASK) 
			{ 
				//std::cout << "MMB pressed!" << std::endl;
				mmb = true;
			}

			if (pKeyboardState[SDL_SCANCODE_W]) { origin += forward * movementSpeedFinal; }
			if (pKeyboardState[SDL_SCANCODE_S]) { origin -= forward * movementSpeedFinal; }
			if (pKeyboardState[SDL_SCANCODE_A]) { origin -= right * movementSpeedFinal; }
			if (pKeyboardState[SDL_SCANCODE_D]) { origin += right * movementSpeedFinal; }

			if (lmb && rmb) {
				origin -= Vector3::UnitY * mouseY * deltaTime;
			}

			else if (lmb) {
				//std::cout << totalPitch << std::endl;
				totalYaw -= mouseX * rotationspeedFinal;
				origin -= forward  * mouseY * deltaTime;
			}
			else if (rmb) {
				totalYaw -= mouseX * rotationspeedFinal;

				float newPitch = totalPitch - (mouseY * rotationspeedFinal);
				if (newPitch > -90 && newPitch < 90)
				{
					totalPitch = newPitch;
				}
			}
			Matrix finalRotation = Matrix::CreateRotation(totalPitch * TO_RADIANS, totalYaw * TO_RADIANS, 0.f);

			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
		}
	};
}
