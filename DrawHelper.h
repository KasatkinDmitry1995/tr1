#pragma once
#include "Structs.h"
#include <cmath>



class DrawHelper
{

	private:
		const float PI = 3.14159265358979323846f;
		float yawRad = .0f;
		float pitchRad = .0f;

		float cosYaw = .0f;
		float sinYaw = .0f;
		float cosPitch = .0f;
		float sinPitch = .0f;
		float fovRad = .0f;
		float focalLength = .0f;

	public:

		Camera cam;

		bool WorldToScreen(const Vec3& worldPos, Vec2& screenPos);
		void UpdateCamData();

};