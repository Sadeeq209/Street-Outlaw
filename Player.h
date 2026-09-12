#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

enum class PlayerState {
    Running,
    Jumping,
    Rolling,  // fast mid-air fall, triggered by pressing Down while Jumping
    Sliding,
    Dead
};

class Player {
public:
    Player();
    void update(float deltaTime);
    void draw(sf::RenderWindow& window, float sizeMultiplier = 1.0f);

    void beginRun();  // called by Game::startGame() -- resets run animation to frame 0
    void jump();
    void slide();
    void roll();
    void onDownPressed(); // decides slide() vs roll() based on current state
    void die(float stopX = -100000.0f, bool knockForward = false);
    void reset();

    void setGroundLevel(float y) { currentGroundLevel = y; }
    float getGroundY() const { return groundY; } // so Game.cpp never needs its own separate copy of this number

    // Called every frame by Game.cpp's updateLandableObstacles() --
    // true only on the exact frame(s) the player is actually resting
    // on a DownlaneVan's walk-free surface (not just any elevated
    // obstacle). jump() reads this at the instant of liftoff to decide
    // whether THIS jump is eligible for the van-flip slow-motion roll.
    void setStandingOnVan(bool value) { standingOnVan = value; }
    void setStandingOnCar(bool value) { standingOnCar = value; }
    void setStandingOnJeep(bool value) { standingOnJeep = value; }

    // Called from Game each frame with a value that grows slightly
    // over survival time -- makes the run-cycle animation play back
    // faster as the game speeds up, matching the increasing pace
    // visually. 1.0 = normal speed, higher = faster leg-cycle.
    void setRunAnimSpeedMultiplier(float m) { runAnimSpeedMultiplier = m; }
    bool isDeathAnimationFinished() const { return deathLanded; }
    sf::FloatRect getBounds() const;
    PlayerState getState() const { return state; }

    // Consumable one-shot event: true exactly once, on the frame a
    // normal jump (front-flip OR classic) lands -- NOT roll's landing.
    // Used by Game.cpp to play a landing sound only for real jumps.
    bool consumeJumpLandedEvent();

    // Same one-shot pattern: true exactly once, the instant jump()
    // decides to launch a front/back flip off a Van AND the 50% roll
    // for the slow-motion effect succeeds. Game.cpp checks this once
    // per frame and, when true, kicks off the world slow-motion
    // timeline + its sound effect.
    bool consumeVanFlipSlowMoEvent();

    // Same one-shot pattern: true exactly once, on the Running-state
    // frame the left/right foot plants (currentFrame == 2 and == 6 of
    // the 8-frame run cycle). Only fires while actually Running, so
    // never during Jumping/Rolling/Sliding.
    bool consumeFootstepLeftEvent();
    bool consumeFootstepRightEvent();

    // True while standing on ANY vehicle (Car/Van/Jeep walk-free
    // surface) -- Game.cpp uses this to pick Car-footstep vs
    // Ground-footstep sounds.
    bool isStandingOnVehicle() const { return standingOnCar || standingOnVan || standingOnJeep; }

    // Same one-shot pattern: true exactly once, the instant the death
    // animation reaches its final frame -- used to trigger the
    // Death_bodyfall.mp3 sound at exactly that moment.
    bool consumeDeathFinalFrameEvent();

private:
    // ==================== TEXTURES ====================
    std::vector<sf::Texture> runTextures;     // 8 frames
    std::vector<sf::Texture> jumpTextures;    // 6 frames (front flip)
    std::vector<sf::Texture> oldJumpTextures; // 4 frames (classic jump, velocity-based)
    std::vector<sf::Texture> rollTextures;    // 5 frames (mid-air fast-fall)
    std::vector<sf::Texture> slideTextures;   // 4 frames
    std::vector<sf::Texture> deathTextures;   // 4 frames
    std::optional<sf::Sprite> sprite;

    // Shadow -- see the loading comment in Player.cpp for how the
    // shrink-with-jump-height numbers were derived.
    sf::Texture shadeTexture;
    std::optional<sf::Sprite> shadowSprite;

    // ==================== PHYSICS ====================
    sf::Vector2f position;
    sf::Vector2f velocity;

    // Kept as-is, used only by death physics (updateDeathPhysics) --
    // not touched by jumpPhysicsSpeedMultiplier below.
    float gravity;

    // JUMP PHYSICS SPEED -- ONE knob for the ENTIRE jump, launch to
    // landing. Scales both how hard he launches AND how hard gravity
    // pulls him back down, together, by the same amount -- so the jump
    // stays the SAME HEIGHT no matter what this is set to, it just
    // takes more or less real time start to finish. 1.0 = original
    // feel. Higher = whole jump (up AND down) happens faster; lower =
    // slower/floatier. This only affects the FRONT-FLIP jump's
    // physics, not classic jump.
    float jumpPhysicsSpeedMultiplier;

    float groundY;
    float currentGroundLevel;

    bool isOnGround;
    bool isSliding;

    PlayerState state;
    int currentFrame;
    int totalFrames;
    float animTimer;
    float animSpeed;
    float runAnimSpeedMultiplier; // set by Game each frame; grows slightly over time

    // RUN FEET PADDING: same idea as jumpFeetPadding/rollFeetPadding/
    // etc -- corrects for each Run1-8.png frame not having its feet
    // at exactly the same pixel height (very common in a hand-drawn
    // run cycle). Without this, the anchor point silently shifts up/
    // down every frame, which showed up as the hitbox "dividing the
    // head" while running, especially visible on top of obstacles.
    // All start at 0.0f -- tune each one by eye per frame.
    float runFeetPadding[8];

    // ==================== SLIDE ====================
    float slideTimer;
    float slideDuration;
    float slideCooldownTimer;    // must reach 0 before slide() can retrigger
    float slideCooldownDuration; // how long after a slide ends before another can start

    // ==================== JUMP (FRONT FLIP + BACK FLIP + CLASSIC) ====================
    // Each jump() call randomly picks ONE of three animation sets to
    // play for that jump (useFrontFlip / useBackFlip -- both false
    // means classic; see jump() in Player.cpp for the odds). Physics
    // is scaled by jumpPhysicsSpeedMultiplier for BOTH flip types;
    // classic jump is untouched by that.
    bool useFrontFlip;
    bool useBackFlip;
    bool useSideFlip;

    float jumpFrameDurations[6]; // how quickly each of the 6 frames is
                                  // shown, in seconds -- simple sequential
                                  // slideshow, sped up/slowed as one group
                                  // by jumpSpeedMultiplier.
    float jumpFeetPadding[6]; // one per frame -- no dedicated landing
                               // frame anymore, the animation just holds
                               // on frame 5 (the last one) through touchdown.
    float jumpSpeedMultiplier; // speeds up/slows the ANIMATION (all 6 frames)

    int jumpFrameIndex;
    float jumpFrameTimer;

    // ==================== JUMP: BACK FLIP ====================
    // Structurally identical to the front-flip fields above (see
    // jumpFrameDurations/jumpFeetPadding/jumpSpeedMultiplier comments)
    // -- 6 real animated frames, no dedicated landing frame, own
    // texture set, own timing so the two can be tuned independently.
    // Assets are Jump001.png-Jump006.png (Jump007.png no longer
    // loaded/used).
    std::vector<sf::Texture> backFlipTextures; // 6 frames
    float backFlipFrameDurations[6];
    float backFlipFeetPadding[6];
    float backFlipSpeedMultiplier;
    int backFlipFrameIndex;
    float backFlipFrameTimer;

    std::vector<sf::Texture> sideFlipTextures; // 7 frames (6 in-air + 1 dedicated landing)
    float sideFlipFrameDurations[6]; // only [0..4] actually read (5 transitions across the 6 in-air frames) -- [5] is a dead slot, kept for size parity, same convention front/back flip originally used
    float sideFlipFeetPadding[7]; // [0-5] = the 6 in-air frames, [6] = the dedicated post-landing frame (SideFlip7)
    float sideFlipSpeedMultiplier;
    int sideFlipFrameIndex;
    float sideFlipFrameTimer;

    float oldJumpFeetPadding[4]; // classic jump picks its frame from
                                  // velocity.y thresholds (see
                                  // updateAnimation), not a timer, so it
                                  // only needs feetPadding, not durations.

    float landingTimer;   // brief hold after a real front-flip/back-flip
                           // touchdown before handing control back to
                           // Running -- purely a timing gate now (no
                           // dedicated landing texture swap happens
                           // anymore, it just keeps showing whichever
                           // frame the flip was already on). ONLY
                           // starts on real ground contact, never
                           // mid-air.
    float landingDuration; // how long that post-touchdown hold lasts, in seconds -- adjust freely
    bool jumpLandedEvent; // set true for one frame when a real jump lands
    bool footstepLeftEvent;  // set true for one frame when the left foot plants (Running frame 2)
    bool footstepRightEvent; // set true for one frame when the right foot plants (Running frame 6)

    // ==================== VAN FLIP SLOW-MOTION ====================
    bool standingOnVan;      // set every frame by Game.cpp -- true only while actually resting on a Van's walk-free surface
    bool standingOnCar;      // same idea, for Car's hood/windshield/roof/trunk
    bool standingOnJeep;     // same idea, for Jeep's walk-free segments
    bool vanFlipSlowMoEvent; // one-shot: true the instant jump() launches a flip off a Van AND the 50% roll succeeds

    // ==================== ROLL (mid-air fast-fall) ====================
    // Two phases, both fully scripted (not gravity-driven) so timing is
    // 100% consistent no matter when in the jump you press Down:
    //   Phase 1 (frames 0-1): "bending knees" -- position smoothly
    //     descends from wherever the roll started down to ground level,
    //     over rollFrameDurations[0]+[1] seconds.
    //   Phase 2 (frames 2-4): locked at ground level, playing out the
    //     remaining 3 frames over rollFrameDurations[2]+[3]+[4] seconds.
    // Total roll length = sum of all 5 durations. Edit any of the 5
    // individually, or scale rollSpeedMultiplier for overall pace.
    float rollFrameDurations[5];
    float rollFeetPadding[5];
    float rollSpeedMultiplier;
    int rollFrameIndex;
    float rollFrameTimer;
    float rollElapsed;   // total time since roll() was called
    float rollStartY;    // position.y at the moment roll() fired

    // ==================== DEATH ====================
    float deathFrameDurations[4];
    float deathFeetPadding[4];
    float deathSpeedMultiplier;
    int deathFrameIndex;
    float deathFrameTimer;
    bool deathFinished;
    bool deathLanded;
    bool deathFinalFrameEvent; // one-shot: true the instant deathFinished first becomes true

    float deathKnockbackSpeed;
    float deathBounceSpeed;
    float deathStopX; // hard limit for how far the knockback can carry position.x, in whichever direction it's travelling -- set fresh each die() call, see Game::triggerDeath()

    // ==================== INTERNAL HELPERS ====================
    void updateAnimation(float deltaTime);
    void applyGravity(float deltaTime);
    void updateDeathPhysics(float deltaTime);
    void updateRollPhysics(float deltaTime);
    void setFrameTexture(sf::Texture& tex, float feetPadding = 0.0f);
};

#endif