#include "CollisionUtils.h"
#include <cmath>
#include <algorithm>
#include <limits>

std::array<sf::Vector2f, 4> RotatedRect::getCorners() const {
    float rad = rotationDeg * 3.14159265f / 180.0f;
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    // Local corners before rotation: top-left, top-right, bottom-right,
    // bottom-left.
    sf::Vector2f local[4] = { {-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh} };

    std::array<sf::Vector2f, 4> world;
    for (int i = 0; i < 4; i++) {
        float x = local[i].x;
        float y = local[i].y;
        float rx = x * cosA - y * sinA;
        float ry = x * sinA + y * cosA;
        world[i] = { center.x + rx, center.y + ry };
    }
    return world;
}

namespace {
    // Projects a set of points onto an axis, returns [min, max].
    std::pair<float, float> projectOntoAxis(const std::array<sf::Vector2f, 4>& points, sf::Vector2f axis) {
        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::lowest();
        for (const auto& p : points) {
            float d = p.x * axis.x + p.y * axis.y;
            minVal = std::min(minVal, d);
            maxVal = std::max(maxVal, d);
        }
        return { minVal, maxVal };
    }

    bool overlaps(std::pair<float, float> a, std::pair<float, float> b) {
        return a.first <= b.second && b.first <= a.second;
    }
}

bool checkCollision(const sf::FloatRect& aabb, const RotatedRect& rot) {
    std::array<sf::Vector2f, 4> aabbCorners = {
        sf::Vector2f{ aabb.position.x, aabb.position.y },
        sf::Vector2f{ aabb.position.x + aabb.size.x, aabb.position.y },
        sf::Vector2f{ aabb.position.x + aabb.size.x, aabb.position.y + aabb.size.y },
        sf::Vector2f{ aabb.position.x, aabb.position.y + aabb.size.y }
    };

    std::array<sf::Vector2f, 4> rotCorners = rot.getCorners();

    float rad = rot.rotationDeg * 3.14159265f / 180.0f;
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    // 4 axes to test: AABB's two axes, plus the rotated rect's two
    // (perpendicular) local axes.
    sf::Vector2f axes[4] = {
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { cosA, sinA },
        { -sinA, cosA }
    };

    for (const auto& axis : axes) {
        auto rangeA = projectOntoAxis(aabbCorners, axis);
        auto rangeB = projectOntoAxis(rotCorners, axis);
        if (!overlaps(rangeA, rangeB)) {
            return false; // found a separating axis, no collision
        }
    }

    return true; // no separating axis found on any of the 4, so they intersect
}

std::optional<float> getTopEdgeYAtX(const RotatedRect& rot, float worldX) {
    std::array<sf::Vector2f, 4> corners = rot.getCorners();
    sf::Vector2f topLeft = corners[0];
    sf::Vector2f topRight = corners[1];

    if (topLeft.x > topRight.x) {
        std::swap(topLeft, topRight);
    }

    if (worldX < topLeft.x || worldX > topRight.x) {
        return std::nullopt;
    }

    float span = topRight.x - topLeft.x;
    if (std::abs(span) < 0.0001f) {
        return topLeft.y; // near-vertical segment, avoid divide-by-zero
    }

    float t = (worldX - topLeft.x) / span;
    return topLeft.y + t * (topRight.y - topLeft.y);
}

bool checkCollision(const sf::FloatRect& aabb, const Circle& circle) {
    float closestX = std::clamp(circle.center.x, aabb.position.x, aabb.position.x + aabb.size.x);
    float closestY = std::clamp(circle.center.y, aabb.position.y, aabb.position.y + aabb.size.y);

    float dx = circle.center.x - closestX;
    float dy = circle.center.y - closestY;

    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}
