#pragma once

#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fps::debug {

struct Vec3 {
    float x {0.0F};
    float y {0.0F};
    float z {0.0F};

    Vec3 operator+(const Vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(const Vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator*(float scale) const { return {x * scale, y * scale, z * scale}; }
};

inline float Dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float Length(const Vec3& value) {
    return std::sqrt(Dot(value, value));
}

inline Vec3 Normalize(const Vec3& value) {
    const float len = Length(value);
    if (len <= 1e-6F) {
        return {0.0F, 0.0F, 0.0F};
    }
    return value * (1.0F / len);
}

struct Color {
    float r {1.0F};
    float g {1.0F};
    float b {1.0F};
    float a {1.0F};
};

struct Entity {
    std::string name;
    Vec3 worldPosition;
    Vec3 bboxMin;
    Vec3 bboxMax;
    float health {100.0F};
    bool alive {true};
    bool isEnemy {true};
};

struct Camera {
    Vec3 position;
    Vec3 forward;
};

struct OverlayConfig {
    float maxDrawDistanceMeters {250.0F};
    float targetFovDegrees {45.0F};
    Color visibleColor {0.0F, 1.0F, 0.0F, 1.0F};
    Color occludedColor {1.0F, 0.3F, 0.3F, 1.0F};
    Color targetColor {1.0F, 0.85F, 0.2F, 1.0F};
};

struct OverlayStats {
    int enemiesDetected {0};
    std::optional<std::string> currentTarget;
    float distanceToTargetMeters {0.0F};
};

struct TargetInfo {
    const Entity* entity {nullptr};
    float distanceMeters {std::numeric_limits<float>::infinity()};
};

class IPhysicsWorld {
public:
    virtual ~IPhysicsWorld() = default;

    // Returns true if any static geometry blocks the segment [from, to].
    virtual bool RaycastOccluded(const Vec3& from, const Vec3& to) const = 0;
};

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void DrawWorldBox(const Vec3& worldMin, const Vec3& worldMax, const Color& color) = 0;
    virtual void DrawWorldLine(const Vec3& from, const Vec3& to, const Color& color) = 0;
    virtual void DrawWorldLabel(const Vec3& worldPos, std::string_view text, const Color& color) = 0;
    virtual void DrawScreenText(float x, float y, std::string_view text, const Color& color) = 0;
};

std::vector<const Entity*> GetEntities(const std::vector<Entity>& worldEntities);

bool IsVisible(const IPhysicsWorld& physics, const Vec3& from, const Vec3& to);

std::optional<TargetInfo> GetClosestTarget(
    const std::vector<const Entity*>& enemies,
    const IPhysicsWorld& physics,
    const Camera& camera,
    float maxFovDegrees,
    float maxDistanceMeters);

OverlayStats DrawOverlay(
    IRenderer& renderer,
    const IPhysicsWorld& physics,
    const Camera& camera,
    const std::vector<Entity>& worldEntities,
    const OverlayConfig& config);

} // namespace fps::debug
