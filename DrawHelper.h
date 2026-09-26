#pragma once
#include "Structs.h"
#include <cmath>



// ---------- World-to-Screen ----------
// Возвращает true, если точка видна (перед камерой), иначе false.
// screenPos — выходные 2D-координаты на экране.
bool WorldToScreen(const Vec3& worldPos, const Camera& cam, Vec2& screenPos);
