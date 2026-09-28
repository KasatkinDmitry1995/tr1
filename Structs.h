#pragma once
#pragma pack(push, 1)
// ---------- Структуры ----------
struct Vec3 {
    float x, y, z;
};

struct Vec2 {
    float x, y;
};

struct Vec4 {
    float x, y, w, h;
};

// ---------- Параметры камеры ----------
struct Camera {
    Vec3  position;   // позиция игрока (X, Y, Z)
    float yaw;        // угол поворота (градусы, 0 = восток, 90 = север)
    float pitch;      // угол наклона (градусы, + вверх, - вниз)
    float fov;        // угол обзора (обычно 90 для CS 1.6)
    int   screenW;    // ширина экрана
    int   screenH;    // высота экрана
};

struct PlayerView {
    float X;
    float Y;
    float Z;
    float Ya;
    float Xa;
};

struct PlayerCoords {
    float X;
    float Y;
    float Z;
    bool operator==(const PlayerCoords& o) const {
        return X == o.X && Y == o.Y && Z == o.Z;
    }
    bool operator!=(const PlayerCoords& o) const {
        return X != o.X || Y != o.Y || Z != o.Z;
    }
};

struct PlayerInfo {
    PlayerCoords coords;
    PlayerCoords lastCoords;
    long lastTimePosChanged;
    bool isDrawable;
};

struct Offsets {
    unsigned int InCross, UserInSpect, userPV, playerAlive, 
        playerCoords, playerStructSize, playersArray;
};

#pragma pack(pop)