#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <vector>
#include "CollisionUtils.h"

enum class ObstacleType {
    Car,        // player jumps over/lands on its sloped top, or dies on front contact
    Tire,       // rolling tire, moves left like everything else; speed follows the shared 'speed' param (same value tiles use); circle collision
    Jeep,       // downlane special-feature vehicle, 10 variants (Jeep1-Jeep10); bigger than the sedans; flat-top walk surface plus front/back death lines
    DownlaneVan,// downlane special-feature vehicle, 5 variants (DownlaneVan1-5); same treatment as Jeep; biggest of the three downlane vehicle kinds
    Drone,      // airborne, 3 variants (Drone1-3); flies toward the player (facing/moving left, opposite of the background Helicopter's left-to-right pass); no landing surface; small death zone on the nose only
    Box,        // thrown package, 2 variants; real projectile arc (launch velocity plus gravity), aimed at a fixed forehead-height point so only sliding avoids it; falls to the ground and drifts off if dodged
    ThrowingCar // the car that "throws" the Box; visually and collision-wise just a Car with trunk-open art; spawns for real (no invisibility trick) once the Box is partway through its flight
};

class Obstacle {
public:
    // heightOverride: 0 (default) uses this type's normal target
    // height. Non-zero forces a specific render height instead, used
    // only by the scripted car-jump scene, so it can make Car/Jeep/
    // DownlaneVan taller than their normal random-spawn sizes without
    // touching those normal sizes.
    //
    // groundYOverride: unused. Car/Jeep/DownlaneVan all use their own
    // per-variant ground line (carGroundYs/jeepGroundYs/vanGroundYs)
    // for both normal spawns and the scripted scene, so this parameter
    // has no effect. Kept in the signature only so boxTargetX/
    // boxTargetY/boxSpeedOverride below stay in the same position for
    // existing call sites; pass 0.0f or omit it.
    //
    // boxTargetX/boxTargetY: only read when type == Box. The fixed
    // world-space point (player's forehead height, current player X)
    // the box's projectile arc is aimed at. Computed once in Game.cpp
    // at the moment the box spawns and passed in here; the box itself
    // never re-reads the player's live position afterward. Ignored by
    // every other type.
    //
    // boxSpeedOverride: lets Game.cpp pass a growth-scaled effective
    // speed for this specific throw instead of the flat BOX_SPEED
    // constant. Sentinel -1.0f means use BOX_SPEED as normal, so this
    // stays optional for call sites that don't pass it.
    Obstacle(ObstacleType type, float x, float heightOverride = 0.0f, float groundYOverride = 0.0f, float boxTargetX = 0.0f, float boxTargetY = 0.0f, float boxSpeedOverride = -1.0f);
    void update(float deltaTime, float speed);
    void draw(sf::RenderWindow& window);
    bool isOffScreen() const;

    // For cars: true only once both visually off-screen and the
    // engine's post-disappearance fade-out tail has finished playing.
    // For every other type: same as isOffScreen(). Game.cpp uses this
    // for removal, not isOffScreen() directly, so the engine sound
    // gets to finish its tail instead of being cut off with the object.
    bool canBeRemoved() const;
    ObstacleType getType() const { return type; }

    // Generic on-screen bounds, used by Game.cpp for things that
    // aren't collision per se (e.g. checking whether the screen is
    // clear enough for a Drone to spawn).
    sf::FloatRect getBounds() const { return sprite->getGlobalBounds(); }

    // Car hitboxes: 4 rotated segments approximating the car's sloped
    // top surface (hood/windshield/roof/trunk), which the player can
    // land/walk on. Plus 1 plain vertical death line on the
    // front-facing side.
    std::vector<RotatedRect> getCarWalkFreeLines() const;
    sf::FloatRect getCarDeathLine() const;     // front (nose), right edge
    sf::FloatRect getCarBackDeathLine() const; // back (trunk), left edge, independent of the front

    // Jeep hitboxes: 3 rotated segments for the walk-free top surface
    // (rear roof/hatch, windshield slope, hood slope) and 4 independent
    // rotated death zones (rear pillar, taillight, rear bumper, front
    // bumper). Every line, walk-free or death, is fully adjustable:
    // center X/Y, width (length), height (thickness), rotationDeg
    // (tilt), plus pixel nudges on top. Edited directly inside
    // loadTextures() in Obstacle.cpp (jeepCollisionLayouts); see the
    // SegmentSpec/VehicleCollisionLayout structs below for what each
    // number does.
    std::vector<RotatedRect> getJeepWalkFreeLines() const;
    std::vector<RotatedRect> getJeepDeathZones() const;

    // Downlane van hitboxes: same fully-adjustable rotated-segment
    // system as Jeep above, 3 walk-free segments (box roof, windshield
    // slope, hood slope) and 2 death zones (one long back-edge line,
    // one front bumper line). Both DownlaneVan variants share this
    // shape/layout, edited in vanCollisionLayouts in Obstacle.cpp.
    std::vector<RotatedRect> getVanWalkFreeLines() const;
    std::vector<RotatedRect> getVanDeathZones() const;

    // Tire hitbox: a single circle instead of a rectangle, a more
    // natural fit for a round rolling tire than any box shape. Touch
    // anywhere on it means death; there is no landing on a tire.
    Circle getTireCollisionCircle() const;

    // Drone hitbox: no walk-free surface at all, since it's airborne
    // and there's nothing to land on. One small adjustable death zone
    // on the nose (the left edge, since the drone faces/flies left),
    // using the same SegmentSpec system as Jeep/Van so it's fully
    // movable/resizable/rotatable. Edited via droneCollisionSpecs in
    // loadTextures() (Obstacle.cpp).
    RotatedRect getDroneDeathZone() const;

    // Drone 4-dot hitbox: replaces the single rotated rect above as the
    // collision Game.cpp actually uses. 4 small circles, one per
    // corner, using the same DotCollisionSpec system as Box below.
    // Edited via droneDotSpecs in loadTextures() (Obstacle.cpp).
    std::vector<Circle> getDroneDotZones() const;

    // Throwing car hitboxes: identical setup to Car above (same
    // 4-segment sloped walk-free top plus front/back death lines). It
    // is a Car visually and for collision, just with different art
    // (trunk popped open) and its own texture/collision tables so it
    // can be tuned independently.
    std::vector<RotatedRect> getThrowingCarWalkFreeLines() const;
    sf::FloatRect getThrowingCarDeathLine() const;
    sf::FloatRect getThrowingCarBackDeathLine() const;

    // Box hitbox: a small adjustable drop-zone, using the same
    // SegmentSpec system as Jeep/Van/Drone, deliberately not the box's
    // full sprite bounds. Edited via boxCollisionSpecs in
    // loadTextures() (Obstacle.cpp).
    RotatedRect getBoxDeathZone() const;

    // Box 4-dot hitbox: replaces the single rotated rect above as the
    // collision Game.cpp actually uses. 4 small circles, one per
    // corner (xFrac/yFrac of the box's own width/height, radius in
    // pixels). Edited via boxDotSpecs in loadTextures() (Obstacle.cpp).
    std::vector<Circle> getBoxDotZones() const;

    // Car sound volume, 0-100, same pattern as every other sound in
    // Game.cpp. These are static (one shared volume for all cars, not
    // per-instance) because there's no single Obstacle for Game.cpp to
    // call a setter on; there's a whole vector of them.
    // Call once during setup, same spot as the other sound volumes.
    // Applied live every frame to already-playing engine sounds too,
    // so a slider change takes effect immediately, not just for cars
    // spawned afterward.
    static void setCarEngineVolume(float volume0to100);
    static void setCarHornVolume(float volume0to100);
    // Drone sound uses real positional/3D audio (sf::Listener plus
    // sf::Sound position and attenuation) instead of a manual volume
    // envelope; this only sets the sound's base volume, before distance
    // attenuation is applied, same 0-100 pattern as everything else.
    static void setDroneSoundVolume(float volume0to100);

    // Same idea as the volume setters above: one shared value for every
    // car's engine/horn sound, since there's no single Obstacle for
    // Game.cpp to call this on. Called once per frame from Game.cpp
    // (alongside update()) with the current slow-motion
    // pitch/volume-multiplier, so every downlane car's engine and horn
    // play back slower/quieter exactly in sync with the world.
    // pitch mirrors worldTimeScale directly. volumeMultiplier is 1.0
    // normal, down to 0.5 at max slow-motion.
    static void setSlowMoAudio(float pitch, float volumeMultiplier);

    // Pauses this car's engine and horn (if playing); call on every car
    // at game over, same idea as bgMusic.pause()/heliSound.pause().
    void pauseSounds();

private:
    // Per-car-variant wheel layout: where the front/rear wheels sit
    // relative to the body sprite, as fractions of the body's own
    // width/height (not fixed pixels), so it stays correct no matter
    // what size the car renders at. wheelDiameterFrac is also a
    // fraction of the body's width. Measured from the M4 body art's
    // wheel-well cutouts for variant 0; new variants need their own
    // measured values here, since wheel well position/size differs car
    // to car.
    struct CarWheelLayout {
        float frontXFrac, frontYFrac;
        float rearXFrac, rearYFrac;
        float wheelDiameterFrac;

        // Fine-tune nudges, in pixels, added after the fraction-based
        // position above, used to nudge one wheel slightly if it
        // doesn't sit perfectly in the well. Positive X is right,
        // positive Y is down. All start at 0 (no nudge).
        float frontOffsetX = 0.0f, frontOffsetY = 0.0f;
        float rearOffsetX = 0.0f, rearOffsetY = 0.0f;
    };
    static std::vector<CarWheelLayout> carWheelLayouts;

public:
    // Read-only access to a downlane Car's own wheel layout, keyed by
    // variant index (0-39). Background.cpp uses this to build the
    // up-lane Car table as a computed mirror (1 - x on the front/rear
    // X fractions and X offsets) instead of hand-typed values, so
    // up-lane can't drift out of sync or have a copy mistake. Jeep gets
    // the same treatment (see getJeepWheelLayout below); Van/Truck are
    // still hand-typed.
    //
    // Must be declared here, after struct CarWheelLayout and
    // carWheelLayouts above: a member function's return type (unlike
    // its body) is looked up immediately at the point it's written, not
    // deferred to end-of-class, so this can't sit any higher in the
    // file than the type/data it references.
    //
    // Must be public and must be called by anything that depends on
    // carWheelLayouts/etc. (like Background's mirror loop) before using
    // it. Safe to call repeatedly from multiple places; texturesLoaded
    // guards it into a no-op after the first real call.
    static void loadTextures();

    static const CarWheelLayout& getCarWheelLayout(int variant) { return carWheelLayouts[variant]; }

private:

    // Jeep and DownlaneVan reuse the exact same CarWheelLayout shape
    // (frontXFrac/frontYFrac/rearXFrac/rearYFrac/wheelDiameterFrac plus
    // pixel nudges) since it's already size-independent; no need for a
    // separate struct type, just separate tables per vehicle kind.
    static std::vector<CarWheelLayout> jeepWheelLayouts;
    static std::vector<CarWheelLayout> vanWheelLayouts;

public:
    // Read-only access to a downlane Jeep's own wheel layout, keyed by
    // variant index (0-9), same purpose/pattern as getCarWheelLayout
    // above (Van/Truck are not mirrored). Same ordering requirement
    // applies: this must sit after jeepWheelLayouts is declared.
    static const CarWheelLayout& getJeepWheelLayout(int variant) { return jeepWheelLayouts[variant]; }

private:

    // ThrowingCar reuses the exact same CarWheelLayout shape as Car:
    // it's visually a sedan with the trunk art popped open, so the same
    // wheel-fraction model fits. Own separate table so it can be tuned
    // independently of the normal sedans.
    static std::vector<CarWheelLayout> throwingCarWheelLayouts;

    // Per-car-variant collision lines: hood/windshield/roof/trunk (the
    // walk-free surface, 4 sloped segments the player lands on) plus
    // the death line (front of the car). Each segment: X/Y is center
    // position as a fraction of the body's width/height, widthFrac is
    // segment length as a fraction of body width, height is thickness
    // in pixels, rotationDeg is tilt angle. The death line uses pixel
    // offsets from the right edge (xOffsetFromRight) since the car's
    // nose faces right, the side the player dies touching.
    struct CarCollisionLayout {
        float hoodXFrac, hoodYFrac, hoodWidthFrac, hoodHeight, hoodRotationDeg;
        float windshieldXFrac, windshieldYFrac, windshieldWidthFrac, windshieldHeight, windshieldRotationDeg;
        float roofXFrac, roofYFrac, roofWidthFrac, roofHeight, roofRotationDeg;
        float trunkXFrac, trunkYFrac, trunkWidthFrac, trunkHeight, trunkRotationDeg;
        float deathLineXOffsetFromRight, deathLineYOffset, deathLineThickness, deathLineHeightReduction; // front (nose)
        float backDeathLineXOffsetFromLeft, backDeathLineYOffset, backDeathLineThickness, backDeathLineHeightReduction; // back (trunk), independent of the front
    };
    static std::vector<CarCollisionLayout> carCollisionLayouts;

    // Same reasoning as throwingCarWheelLayouts above: reuses the exact
    // CarCollisionLayout shape (4 sloped segments plus front/back death
    // line), own table, tunable independently.
    static std::vector<CarCollisionLayout> throwingCarCollisionLayouts;

    // Fully adjustable rotated-segment collision system, used by both
    // Jeep and DownlaneVan (walk-free surface and death zones alike),
    // since their boxier shapes need multiple independent rotated
    // pieces instead of the sedan's fixed hood/windshield/roof/trunk
    // layout.
    //
    // Every line, whether part of the walkable top surface or a death
    // zone, is one of these specs, and every number is independently
    // editable:
    //   xFrac, yFrac    : center position, as a fraction of the body's
    //                     own width/height
    //   widthFrac       : length of the segment along its own tilted
    //                     axis, as a fraction of body width
    //   height          : thickness in pixels
    //   rotationDeg     : tilt angle (0 = flat, positive slopes down to
    //                     the right, negative slopes up to the right),
    //                     including near-vertical (90) for death zones
    //                     like the taillight/bumper marks
    //   xOffset,yOffset : extra pixel nudge added after the fraction
    //                     position, same idea as the wheel nudges above
    struct SegmentSpec {
        float xFrac, yFrac;
        float widthFrac;
        float height;
        float rotationDeg;
        float xOffset = 0.0f, yOffset = 0.0f;
    };

    // Small circular hitbox: position as a fraction of the sprite's
    // own width/height (same fraction convention as everything else),
    // radius in raw pixels. Used for the 4-dot collision system
    // (Drone, Box), 4 of these per variant, one per corner.
    struct DotCollisionSpec {
        float xFrac, yFrac;
        float radius;
    };

    // One vehicle variant's full collision picture: however many
    // walk-free segments and death zones it needs, not a fixed count,
    // so Jeep's 3 walk-free/4 death-zone layout and DownlaneVan's 3
    // walk-free/2 death-zone layout both fit the same struct.
    struct VehicleCollisionLayout {
        std::vector<SegmentSpec> walkFreeSegments;
        std::vector<SegmentSpec> deathZones;
    };
    static std::vector<VehicleCollisionLayout> jeepCollisionLayouts;
    static std::vector<VehicleCollisionLayout> vanCollisionLayouts;

    // Shared helper: turns a list of SegmentSpecs plus a body's current
    // on-screen bounds into world-space RotatedRects. Used by all 4
    // Jeep/Van getters below so the fraction-to-world-space math only
    // lives in one place.
    static std::vector<RotatedRect> buildSegments(const std::vector<SegmentSpec>& specs, const sf::FloatRect& body);

    // 40 car variants. Add matching entries to carBodyTextures/
    // carWheelTextures/carWheelLayouts in loadTextures() (Obstacle.cpp)
    // for any added later.
    static const int NUM_CAR_VARIANTS = 40;
    static int lastCarVariant; // avoids the same variant spawning twice in a row; static since it's shared across all Obstacle instances
    static std::vector<sf::Texture> carBodyTextures;
    static std::vector<float> carTargetHeights;
    static std::vector<float> carGroundYs; // per-variant ground line, same pattern as carTargetHeights; used for both normal spawns and the scripted scene
    static std::vector<sf::Texture> carWheelTextures;

    // Jeep: 10 variants (Jeep1-Jeep10), same "never repeat last"
    // pattern as Car above.
    static const int NUM_JEEP_VARIANTS = 10;
    static int lastJeepVariant;
    static std::vector<sf::Texture> jeepBodyTextures;
    static std::vector<float> jeepTargetHeights;
    static std::vector<float> jeepGroundYs; // per-variant ground line, same pattern as jeepTargetHeights; used for both normal spawns and the scripted scene
    static std::vector<sf::Texture> jeepWheelTextures;

    // Downlane van: 5 variants (DownlaneVan1-5). Separate from the
    // up-lane Van in Background.cpp: different lane, different
    // textures/collision, just a similar naming idea.
    static const int NUM_DOWNLANE_VAN_VARIANTS = 5;
    static int lastVanVariant;
    static std::vector<sf::Texture> vanBodyTextures;
    static std::vector<float> vanTargetHeights;
    static std::vector<float> vanGroundYs; // per-variant ground line, same pattern as vanTargetHeights; used for both normal spawns and the scripted scene
    static std::vector<sf::Texture> vanWheelTextures;

    // Throwing car: 2 variants, own "never repeat last" pattern. Reuses
    // the same engine/horn sound as Car/Jeep/DownlaneVan; see update().
    static const int NUM_THROWING_CAR_VARIANTS = 2;
    static int lastThrowingCarVariant;
    static std::vector<sf::Texture> throwingCarBodyTextures;
    static std::vector<sf::Texture> throwingCarWheelTextures;

    // Box: 2 variants, single sprite each, no wheels, no collision
    // layout table (uses plain getBounds() as its own death zone).
    static const int NUM_BOX_VARIANTS = 2;
    static int lastBoxVariant;
    static std::vector<sf::Texture> boxTextures;

    // Small adjustable drop-zone, one per box variant, using the same
    // SegmentSpec system as Jeep/Van/Drone rather than the full sprite
    // bounds.
    static std::vector<SegmentSpec> boxCollisionSpecs;

    // 4-dot hitbox: flat vector, 4 entries per variant (index =
    // carVariant*4 + 0..3, one per corner). This is the collision
    // Game.cpp actually uses, not boxCollisionSpecs above.
    static std::vector<DotCollisionSpec> boxDotSpecs;

    // Drone: 3 variants (Drone1-3), single sprite each, no wheels, no
    // wheel layout table needed. Own "never repeat last" pattern, same
    // as everything else.
    static const int NUM_DRONE_VARIANTS = 3;
    static int lastDroneVariant;
    static std::vector<sf::Texture> droneBodyTextures;

    // Per-variant target height, same pattern as carTargetHeights/
    // jeepTargetHeights/vanTargetHeights above, so each drone variant
    // can be sized independently. Populated in loadTextures()
    // (Obstacle.cpp).
    static std::vector<float> droneTargetHeights;
    static sf::SoundBuffer droneSoundBuffer;
    static bool droneSoundBufferLoaded;
    static float droneSoundVolume; // 0-100, same pattern as every other sound
    static void loadDroneSound();

    // One SegmentSpec per drone variant: just the single nose death
    // zone (no walk-free segments, no other death zones). Reuses the
    // exact same SegmentSpec type Jeep/Van use, so it gets the same
    // full move/resize/rotate adjustment power.
    static std::vector<SegmentSpec> droneCollisionSpecs;

    // 4-dot hitbox, same flat-vector convention as boxDotSpecs above
    // (index = carVariant*4 + 0..3). This is the collision Game.cpp
    // actually uses, not droneCollisionSpecs above.
    static std::vector<DotCollisionSpec> droneDotSpecs;

    // Shared across every wheeled vehicle type (Car/Jeep/DownlaneVan/
    // ThrowingCar): one texture, reused for all of them.
    static sf::Texture shadeTexture; // loads Shade.png
    static sf::Texture tireTexture;      // loads Tire.png
    static bool texturesLoaded;

    // Car sounds: 2 engine variants (each car picks one 50/50 at spawn,
    // so two cars on screen can sound different from each other) and 3
    // horn variants (see the horn trigger in update()). Buffers are
    // static/shared (loaded once), but each car gets its own sf::Sound
    // instance referencing them, which lets multiple cars' engines and
    // horns overlap and play simultaneously instead of one shared
    // sf::Sound cutting itself off every time a new car plays.
    static const int NUM_CAR_HORN_VARIANTS = 3;
    static sf::SoundBuffer carHornBuffers[NUM_CAR_HORN_VARIANTS];
    static bool carHornBuffersLoaded[NUM_CAR_HORN_VARIANTS];
    static sf::SoundBuffer carEngineBuffer;   // single shared engine sound, all downlane cars
    static bool carEngineBufferLoaded;
    static bool carSoundsLoaded;
    static void loadCarSounds();
    static float carEngineVolume; // 0-100, shared by all cars
    static float carHornVolume;   // 0-100, shared by all cars
    static float slowMoAudioPitch;      // 1.0 = normal; set by Game.cpp each frame
    static float slowMoAudioVolumeMult; // 1.0 = normal, down to 0.5 at max slow-motion; set by Game.cpp each frame

    ObstacleType type;
    std::optional<sf::Sprite> sprite;
    bool active;

    // Per-instance: each car's own looping engine sound, started at
    // spawn, stops automatically when the car object is destroyed.
    std::optional<sf::Sound> engineSound;

    // Per-instance: fires once per car, only if the 20% roll passes,
    // when the car has traveled ~90% of the way toward the left edge.
    // hornRolled marks that this car's honk decision has already been
    // made, so the 20% chance isn't re-rolled every frame once the 90%
    // mark is crossed.
    std::optional<sf::Sound> hornSound;
    std::optional<sf::Sound> droneSound; // positional; follows this drone's live position every frame
    bool hornRolled;

    // X position where this car was spawned, needed to compute what
    // fraction of the trip to the left edge this car is at, for the
    // horn trigger, since the car's closing speed isn't constant.
    float spawnXPos;

    // Per-instance (not static/shared): each rolling tire needs its own
    // persistent spin angle, since multiple tires can exist on screen
    // at once, each at a different point in its roll.
    float tireRotationDeg;

    // Per-instance: same idea as tireRotationDeg, but for a car's 2
    // wheels (both spin at the same rate, so one shared angle covers
    // both). Also stores which variant this specific car instance is,
    // chosen randomly at construction. Reused as-is for Jeep and
    // DownlaneVan too (indexing into jeepXxx/vanXxx tables instead of
    // carXxx when type is Jeep/DownlaneVan), since the per-instance
    // shape needed (variant index, wheel spin angle, 2 wheel sprites,
    // engine/horn sound) is the same across vehicle kinds.
    int carVariant;
    float carWheelRotationDeg;
    std::optional<sf::Sprite> frontWheelSprite;
    std::optional<sf::Sprite> rearWheelSprite;

    // Per-instance, only meaningful for Car/Jeep/DownlaneVan/
    // ThrowingCar. Width is based on the wheelbase (distance between
    // the front and rear wheel positions, from the same CarWheelLayout
    // data the wheels themselves use), not a guessed fraction of the
    // whole body: a car's mirrors/bumpers stick out past where its
    // tires actually touch the ground, so sizing off the full body
    // width would look chunkier than a real ground shadow. Both
    // fractions are computed once at construction; the actual pixel
    // position/size is recalculated fresh from the body's current
    // bounds every frame in draw(), so the shadow tracks the vehicle
    // with zero extra per-frame movement code, the same trick the
    // wheels use.
    std::optional<sf::Sprite> shadowSprite;
    float shadowWidthFrac;
    float shadowCenterXFrac;

    // Per-instance, only meaningful for type == Box. The arc itself
    // (launch to aimed target) is driven directly by elapsed time and a
    // fixed BOX_ARC_HEIGHT rather than solved backward from gravity and
    // duration, since that indirect approach makes "how high does it
    // arc" hard to predict from BOX_GRAVITY and duration alone.
    // BOX_ARC_HEIGHT is a direct pixel value: at 150, the box rises
    // exactly ~150px above the straight line between launch and target.
    float boxElapsedTime;      // time since launch, drives the arc while boxArcComplete is false
    float boxFlightDuration;   // this throw's actual flight time, derived once at spawn as distance/BOX_SPEED; varies per-throw based on actual launch/target distance
    float boxLaunchX, boxLaunchY;      // stored launch position (arc math needs the original point, not the current one)
    float boxArcTargetX, boxArcTargetY; // stored aim point (same values passed into the constructor)
    bool boxArcComplete;       // true once elapsed time has reached boxFlightDuration; switches from the arc formula to real falling

    // Per-instance, only meaningful for type == Box. Real projectile
    // motion (velocity plus gravity), but only for the phase after the
    // arc completes (dodged, or just past the aimed moment): gravity
    // keeps pulling it down from wherever the arc left off, continuing
    // seamlessly into a real fall rather than the arc's fixed curve.
    float boxVelocityX;
    float boxVelocityY;

    // Once the box's falling arc reaches the downlane ground line, it
    // stops falling and switches to simple horizontal drift (the same
    // -speed*deltaTime movement everything else uses) for the rest of
    // its time on screen.
    bool boxGrounded;

    // Shared between the constructor and update(); see the comment on
    // boxElapsedTime above for why gravity only applies after the arc.
    static constexpr float BOX_GRAVITY = 2150.0f; // pixels/second^2

    // The arc-height knob: direct pixels, always exactly this value
    // regardless of flight duration or how far the box travels.
    static constexpr float BOX_ARC_HEIGHT = 135.0f; // pixels above the straight launch->target line
};

#endif