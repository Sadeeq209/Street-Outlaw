#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// Shared ground scroll speed, in pixels/second.
// Used by BOTH the ground tiles (Background) and every obstacle
// (Obstacle), so they always move in lockstep -- no drift between
// the sidewalk and the crates/chains sitting on it.
//
// Change this ONE value to make the whole game faster/harder or
// slower/easier. Do not hardcode a separate speed number in
// Background.cpp or Obstacle.cpp -- always reference GROUND_SPEED.
constexpr float GROUND_SPEED = 500.0f; // adjust this ONE number to change starting scroll speed for everything (background, obstacles, and helicopter, which is now speed*multiplier -- see Background.cpp)

// Global multiplier applied on TOP of every obstacle type's individual
// size (see the per-type "SIZE" comments in Obstacle.cpp). Useful for
// quickly testing "is everything too small/big now" against the new
// background art without touching each type's number individually.
// 1.0 = no change. Each type's own size number is still there too, for
// fine-tuning one type at a time once the overall scale feels right.
constexpr float OBSTACLE_SCALE_MULTIPLIER = 1.0f;

// VIEW BOUNDARIES -- match the zoom-out view in Game.cpp (viewWidth/
// viewHeight, centered at 400,300). Anything that needs to know "is
// this off the left/right edge of what's actually visible" should
// reference these, NOT hardcode 0 or 800 -- those were only correct
// before the zoom-out existed. If viewWidth changes in Game.cpp,
// update these two to match: LEFT = 400 - (viewWidth/2),
// RIGHT = 400 + (viewWidth/2).
constexpr float VIEW_LEFT_EDGE = -100.0f;  // matches viewWidth = 1000
constexpr float VIEW_RIGHT_EDGE = 900.0f;  // matches viewWidth = 1000

// Shared between Obstacle.cpp (the actual box-throw projectile
// physics) and Game.cpp (which spawns the ThrowingCar partway through
// the box's actual flight). MUST stay in this one shared place -- if
// the two files used their own separate copies of this number they
// could drift out of sync.
//
// This is a constant SPEED (pixels/second), not a fixed total time.
// The box always travels at this same rate no matter how far the
// throw actually is -- so changing boxSpawnEdgeMargin (how far off-
// screen it starts) just makes the box visible for a bit longer or
// shorter, without silently warping the whole arc's shape/timing the
// way a fixed-duration model did. Raise/lower this one number to make
// every throw faster/slower, distance-independent.
constexpr float BOX_SPEED = 800.0f; // pixels/second, horizontal

#endif
