#include "DrawHelper.h"


// ---------- World-to-Screen ----------
// Возвращает true, если точка видна (перед камерой), иначе false.
// screenPos — выходные 2D-координаты на экране.
bool WorldToScreen(const Vec3& worldPos, const Camera& cam, Vec2& screenPos) {
    // 1. Смещение относительно камеры
    float dx = worldPos.x - cam.position.x;
    float dy = worldPos.y - cam.position.y;
    float dz = worldPos.z - cam.position.z;

    // 2. Перевод углов в радианы
    const float PI = 3.14159265358979323846f;
    float yawRad = cam.yaw * PI / 180.0f;
    float pitchRad = cam.pitch * PI / 180.0f;

    float cosYaw = cosf(yawRad);
    float sinYaw = sinf(yawRad);
    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);

    // 3. Поворот вокруг оси Z (yaw) — перевод в локальные координаты
    // В CS 1.6: X — восток, Y — север, Z — вверх
    float localX = dx * sinYaw - dy * cosYaw;   // вправо
    float localY = dx * cosYaw + dy * sinYaw;   // вперёд
    float localZ = dz;                           // вверх

    // 4. Поворот вокруг оси X (pitch)
    float rotatedY = localY * cosPitch + localZ * sinPitch;
    float rotatedZ = -localY * sinPitch + localZ * cosPitch;

    // 5. Проверка: точка перед камерой?
    if (rotatedY <= 0.01f) {
        return false; // Точка позади или слишком близко
    }

    // 6. Перспективная проекция
    float fovRad = cam.fov * PI / 180.0f;
    float focalLength = (cam.screenW / 2.0f) / tanf(fovRad / 2.0f);

    float screenX = (localX / rotatedY) * focalLength + (cam.screenW / 2.0f);
    float screenY = -(rotatedZ / rotatedY) * focalLength + (cam.screenH / 2.0f);

    screenPos.x = screenX;
    screenPos.y = screenY;
    return true;
}