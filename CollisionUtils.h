#ifndef COLLISION_UTILS_H
#define COLLISION_UTILS_H

#include <SFML/Graphics.hpp>
#include <array>
#include <optional>

// A rectangle that can be rotated, unlike sf::FloatRect (which is
// always axis-aligned). Used for obstacle surfaces that sit at an
// angle -- e.g. a car's sloped hood/windshield/trunk.
//
// rotationDeg: degrees, same direction as SFML's rotation (clockwise,
// since Y grows downward on screen). Negative = sloping UP to the
// right, positive = sloping DOWN to the right.
struct RotatedRect {
    sf::Vector2f center;
    float width = 0.0f;   // extent along its own (possibly tilted) length
    float height = 0.0f;  // thickness
    float rotationDeg = 0.0f;

    std::array<sf::Vector2f, 4> getCorners() const;
};

// True axis-aligned-box vs rotated-box intersection test (Separating
// Axis Theorem). The player's hitbox stays a normal sf::FloatRect --
// only obstacle surfaces need to tilt.
bool checkCollision(const sf::FloatRect& aabb, const RotatedRect& rot);

// For "walk on top of a sloped surface" logic: given a RotatedRect
// representing a walkable surface, returns the world-space Y height of
// its TOP edge at a given world X position -- or std::nullopt if that
// X falls outside the segment's horizontal span. This lets a sloped
// segment act as a continuous ramp instead of a single flat step.
std::optional<float> getTopEdgeYAtX(const RotatedRect& rot, float worldX);

// A round collision shape -- for obstacles that are naturally
// circular (e.g. a rolling tire), where a rectangle hitbox would be a
// poor fit compared to an actual circle.
struct Circle {
    sf::Vector2f center;
    float radius = 0.0f;
};

// Axis-aligned-box vs circle intersection test (finds the closest
// point on the box to the circle's center, checks if that point is
// within the radius). The player's hitbox stays a normal sf::FloatRect.
bool checkCollision(const sf::FloatRect& aabb, const Circle& circle);

#endif