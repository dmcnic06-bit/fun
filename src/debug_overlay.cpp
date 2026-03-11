#include "debug_overlay.hpp"

#include <algorithm>
#include <sstream>

namespace fps::debug {

namespace {

float Radians(float degrees) {
    return degrees * 3.14159265358979323846F / 180.0F;
}

std::string BuildEnemyLabel(const Entity& enemy, float distanceMeters) {
    std::ostringstream out;
    out << enemy.name << " | HP: " << static_cast<int>(std::max(0.0F, enemy.health))
        << " | Dist: " << static_cast<int>(distanceMeters) << "m";
    return out.str();
}

} // namespace

std::vector<const Entity*> GetEntities(const std::vector<Entity>& worldEntities) {
    std::vector<const Entity*> enemies;
    enemies.reserve(worldEntities.size());

    for (const Entity& entity : worldEntities) {
        if (!entity.alive || !entity.isEnemy) {
            continue;
        }
        enemies.push_back(&entity);
    }

    return enemies;
}

bool IsVisible(const IPhysicsWorld& physics, const Vec3& from, const Vec3& to) {
    // Visibility is determined with a segment raycast between two world positions.
    // If static geometry is hit, the target is considered occluded.
    return !physics.RaycastOccluded(from, to);
}

std::optional<TargetInfo> GetClosestTarget(
    const std::vector<const Entity*>& enemies,
    const IPhysicsWorld& physics,
    const Camera& camera,
    float maxFovDegrees,
    float maxDistanceMeters) {
    std::optional<TargetInfo> bestTarget;

    const Vec3 cameraForward = Normalize(camera.forward);

    // FOV check uses dot product:
    //   dot(normalizedForward, normalizedToEnemy) = cos(theta)
    // where theta is the angle between vectors.
    // An enemy is inside FOV when theta <= maxFov/2.
    const float halfFovRadians = Radians(maxFovDegrees * 0.5F);
    const float minDot = std::cos(halfFovRadians);

    for (const Entity* enemy : enemies) {
        if (enemy == nullptr) {
            continue;
        }

        const Vec3 toEnemy = enemy->worldPosition - camera.position;
        const float distance = Length(toEnemy);
        if (distance <= 1e-3F || distance > maxDistanceMeters) {
            continue;
        }

        const Vec3 dirToEnemy = toEnemy * (1.0F / distance);
        const float alignment = Dot(cameraForward, dirToEnemy);
        if (alignment < minDot) {
            continue;
        }

        if (!IsVisible(physics, camera.position, enemy->worldPosition)) {
            continue;
        }

        if (!bestTarget.has_value() || distance < bestTarget->distanceMeters) {
            bestTarget = TargetInfo {enemy, distance};
        }
    }

    return bestTarget;
}

OverlayStats DrawOverlay(
    IRenderer& renderer,
    const IPhysicsWorld& physics,
    const Camera& camera,
    const std::vector<Entity>& worldEntities,
    const OverlayConfig& config) {
    OverlayStats stats;

    const std::vector<const Entity*> enemies = GetEntities(worldEntities);
    stats.enemiesDetected = static_cast<int>(enemies.size());

    const std::optional<TargetInfo> target = GetClosestTarget(
        enemies,
        physics,
        camera,
        config.targetFovDegrees,
        config.maxDrawDistanceMeters);

    for (const Entity* enemy : enemies) {
        if (enemy == nullptr) {
            continue;
        }

        const Vec3 toEnemy = enemy->worldPosition - camera.position;
        const float distance = Length(toEnemy);
        if (distance > config.maxDrawDistanceMeters) {
            continue;
        }

        const bool visible = IsVisible(physics, camera.position, enemy->worldPosition);
        Color drawColor = visible ? config.visibleColor : config.occludedColor;

        if (target.has_value() && target->entity == enemy) {
            drawColor = config.targetColor;
            stats.currentTarget = enemy->name;
            stats.distanceToTargetMeters = target->distanceMeters;
        }

        renderer.DrawWorldBox(enemy->bboxMin, enemy->bboxMax, drawColor);

        // Show a compact status label above the top of the enemy box.
        const Vec3 labelPos {
            enemy->worldPosition.x,
            enemy->bboxMax.y + 0.35F,
            enemy->worldPosition.z,
        };
        renderer.DrawWorldLabel(labelPos, BuildEnemyLabel(*enemy, distance), drawColor);

        // Draw a guide line only when the enemy has a clear line of sight.
        if (visible) {
            renderer.DrawWorldLine(camera.position, enemy->worldPosition, drawColor);
        }
    }

    renderer.DrawScreenText(20.0F, 20.0F, "=== DEBUG OVERLAY ===", {1.0F, 1.0F, 1.0F, 1.0F});
    renderer.DrawScreenText(20.0F, 42.0F, "Enemies detected: " + std::to_string(stats.enemiesDetected), {0.8F, 0.9F, 1.0F, 1.0F});

    if (stats.currentTarget.has_value()) {
        renderer.DrawScreenText(20.0F, 64.0F, "Current target: " + *stats.currentTarget, {1.0F, 0.9F, 0.4F, 1.0F});
        renderer.DrawScreenText(
            20.0F,
            86.0F,
            "Distance to target: " + std::to_string(static_cast<int>(stats.distanceToTargetMeters)) + "m",
            {1.0F, 0.9F, 0.4F, 1.0F});
    } else {
        renderer.DrawScreenText(20.0F, 64.0F, "Current target: none", {0.8F, 0.8F, 0.8F, 1.0F});
        renderer.DrawScreenText(20.0F, 86.0F, "Distance to target: --", {0.8F, 0.8F, 0.8F, 1.0F});
    }

    return stats;
}

} // namespace fps::debug
