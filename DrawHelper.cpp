#pragma once
#include "DrawHelper.h"


// ---------- World-to-Screen ----------
// Возвращает true, если точка видна (перед камерой), иначе false.
// screenPos — выходные 2D-координаты на экране.
bool DrawHelper::WorldToScreen(const Vec3& worldPos, Vec2& screenPos) {
    // 1. Смещение относительно камеры
    float dx = worldPos.x - cam.position.x;
    float dy = worldPos.y - cam.position.y;

    // 3. Поворот вокруг оси Z (yaw) — перевод в локальные координаты
    // В CS 1.6: X — восток, Y — север, Z — вверх
    float localX = dx * sinYaw - dy * cosYaw;   // вправо
    float localY = dx * cosYaw + dy * sinYaw;   // вперёд
    float localZ = worldPos.z - cam.position.z; // вверх

    // 4. Поворот вокруг оси X (pitch)
    float rotatedY = localY * cosPitch + localZ * sinPitch;
    float rotatedZ = -localY * sinPitch + localZ * cosPitch;

    // 5. Проверка: точка перед камерой?
    if (rotatedY <= 0.01f)
        return false; // Точка позади или слишком близко

    float screenX = (localX / rotatedY) * focalLength + (cam.screenW / 2.0f);
    float screenY = -(rotatedZ / rotatedY) * focalLength + (cam.screenH / 2.0f);

    screenPos.x = screenX;
    screenPos.y = screenY;
    return true;
}

void DrawHelper::UpdateCamData()
{
    yawRad = cam.yaw * PI / 180.0f;
    pitchRad = cam.pitch * PI / 180.0f;
    fovRad = cam.fov * PI / 180.0f;
    cosYaw = cosf(yawRad);
    sinYaw = sinf(yawRad);
    cosPitch = cosf(pitchRad);
    sinPitch = sinf(pitchRad);
    focalLength = (cam.screenW / 2.0f) / tanf(fovRad / 2.0f);
}