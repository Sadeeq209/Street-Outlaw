#include "Player.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

Player::Player() :
    position(150.0f, 672.0f),
    velocity(0.0f, 0.0f),
    gravity(1500.0f),
    // Scales both launch velocity and gravity for the front-flip jump
    // equally, so jump height stays the same regardless of this value;
    // only the time to complete the jump changes. 1.0 = baseline speed
    // and height.
    jumpPhysicsSpeedMultiplier(1.0f),
    groundY(672.0f),
    currentGroundLevel(675.0f),
    isOnGround(true),
    isSliding(false),
    state(PlayerState::Running),
    currentFrame(0),
    totalFrames(8),
    animTimer(0.0f),
    animSpeed(0.080f),
    runAnimSpeedMultiplier(2.2f),
    slideTimer(0.0f),
    slideDuration(0.6f),
    slideCooldownTimer(0.0f),
    slideCooldownDuration(0.15f),
    useFrontFlip(true),
    useBackFlip(false),
    useSideFlip(false),
    jumpSpeedMultiplier(1.7f),
    jumpFrameIndex(0),
    jumpFrameTimer(0.0f),
    backFlipSpeedMultiplier(1.4f),
    backFlipFrameIndex(0),
    backFlipFrameTimer(0.0f),
    sideFlipSpeedMultiplier(1.4f),
    sideFlipFrameIndex(0),
    sideFlipFrameTimer(0.0f),
    landingTimer(0.0f),
    landingDuration(0.13f),
    jumpLandedEvent(false),
    footstepLeftEvent(false),
    footstepRightEvent(false),
    standingOnVan(false),
    standingOnCar(false),
    standingOnJeep(false),
    vanFlipSlowMoEvent(false),
    rollSpeedMultiplier(0.5f),
    rollFrameIndex(0),
    rollFrameTimer(0.0f),
    rollElapsed(0.0f),
    rollStartY(0.0f),
    deathSpeedMultiplier(1.5f), // death animation plays at 1.5x speed
    deathFrameIndex(0),
    deathFrameTimer(0.0f),
    deathFinished(false),
    deathFinalFrameEvent(false),
    deathLanded(false),
    deathKnockbackSpeed(-300.0f),
    deathBounceSpeed(-400.0f),
    deathStopX(-100000.0f)
{
    // Shadow. Uses the same Shade.png texture as the vehicles. Width is a
    // flat fraction of the player's body width (no wheelbase concept for
    // a person); shrinks and fades while airborne based on current height
    // off the ground. 120px is the max jump height (launch velocity -600,
    // gravity 1500: v^2/(2g) = 120), used as the fully-shrunk reference
    // point in draw() below.
    shadeTexture.loadFromFile("assets/Shade.png");
    if (shadeTexture.getSize().x == 0) {
        std::cerr << "Warning: could not load assets/Shade.png -- player shadow will be invisible/broken until this file exists.\n";
    }
    shadowSprite.emplace(shadeTexture);

    // Run
    runTextures.resize(8);
    runTextures[0].loadFromFile("assets/Run1.png");
    runTextures[1].loadFromFile("assets/Run2.png");
    runTextures[2].loadFromFile("assets/Run3.png");
    runTextures[3].loadFromFile("assets/Run4.png");
    runTextures[4].loadFromFile("assets/Run5.png");
    runTextures[5].loadFromFile("assets/Run6.png");
    runTextures[6].loadFromFile("assets/Run7.png");
    runTextures[7].loadFromFile("assets/Run8.png");

    // All start at 0.0f; corrects each Run frame so the feet appear
    // planted at the same spot across the cycle. Same mechanism as
    // jumpFeetPadding/rollFeetPadding below.
    runFeetPadding[0] = 0.0f;
    runFeetPadding[1] = 0.0f;
    runFeetPadding[2] = 0.0f;
    runFeetPadding[3] = 0.0f;
    runFeetPadding[4] = 0.0f;
    runFeetPadding[5] = 0.0f;
    runFeetPadding[6] = 0.0f;
    runFeetPadding[7] = 0.0f;

    // Jump: front flip (6 animated frames, no dedicated landing frame)
    jumpTextures.resize(6);
    jumpTextures[0].loadFromFile("assets/Jump1.png");
    jumpTextures[1].loadFromFile("assets/Jump2.png");
    jumpTextures[2].loadFromFile("assets/Jump3.png");
    jumpTextures[3].loadFromFile("assets/Jump4.png");
    jumpTextures[4].loadFromFile("assets/Jump5.png");
    jumpTextures[5].loadFromFile("assets/Jump6.png");

    // Seconds each frame is shown; sped up/slowed by jumpSpeedMultiplier
    // as one group. Index 5's duration is never read (there's no frame
    // after the last one to advance to; it just holds once reached),
    // kept only so the array size matches the frame count.
    jumpFrameDurations[0] = 0.37f;
    jumpFrameDurations[1] = 0.30f;
    jumpFrameDurations[2] = 0.11f;
    jumpFrameDurations[3] = 0.10f;
    jumpFrameDurations[4] = 0.24f;
    jumpFrameDurations[5] = 0.10f; // unused

    jumpFeetPadding[0] = 0.0f;
    jumpFeetPadding[1] = 0.0f;
    jumpFeetPadding[2] = 0.0f;
    jumpFeetPadding[3] = 0.0f;
    jumpFeetPadding[4] = 0.0f;
    jumpFeetPadding[5] = 50.0f;

    // Jump: back flip (6 animated frames, no dedicated landing frame).
    // 6 animated frames loaded (Jump001.png-Jump006.png); Jump007.png is
    // not used.
    backFlipTextures.resize(6);
    backFlipTextures[0].loadFromFile("assets/Jump001.png");
    backFlipTextures[1].loadFromFile("assets/Jump002.png");
    backFlipTextures[2].loadFromFile("assets/Jump003.png");
    backFlipTextures[3].loadFromFile("assets/Jump004.png");
    backFlipTextures[4].loadFromFile("assets/Jump005.png");
    backFlipTextures[5].loadFromFile("assets/Jump006.png");

    sideFlipTextures.resize(7);
    sideFlipTextures[0].loadFromFile("assets/SideFlip1.png");
    sideFlipTextures[1].loadFromFile("assets/SideFlip2.png");
    sideFlipTextures[2].loadFromFile("assets/SideFlip3.png");
    sideFlipTextures[3].loadFromFile("assets/SideFlip4.png");
    sideFlipTextures[4].loadFromFile("assets/SideFlip5.png");
    sideFlipTextures[5].loadFromFile("assets/SideFlip6.png");
    sideFlipTextures[6].loadFromFile("assets/SideFlip7.png"); // only shown after real touchdown

    // Index 5's duration is never read (there's no frame after the last
    // one to advance to; it just holds once reached), kept only so the
    // array size matches the frame count.
    backFlipFrameDurations[0] = 0.20f;
    backFlipFrameDurations[1] = 0.35f;
    backFlipFrameDurations[2] = 0.20f;
    backFlipFrameDurations[3] = 0.10f;
    backFlipFrameDurations[4] = 0.80f;
    backFlipFrameDurations[5] = 0.05f; // unused

    // Starts with the same timing as back flip; independent from it.
    sideFlipFrameDurations[0] = 0.25f;
    sideFlipFrameDurations[1] = 0.28f;
    sideFlipFrameDurations[2] = 0.15f;
    sideFlipFrameDurations[3] = 0.15f;
    sideFlipFrameDurations[4] = 0.15f;
    sideFlipFrameDurations[5] = 0.10f; // unused

    backFlipFeetPadding[0] = 0.0f;
    backFlipFeetPadding[1] = 0.0f;
    backFlipFeetPadding[2] = 0.0f;
    backFlipFeetPadding[3] = 0.0f;
    backFlipFeetPadding[4] = 0.0f;
    backFlipFeetPadding[5] = 30.0f;

    sideFlipFeetPadding[0] = 0.0f;
    sideFlipFeetPadding[1] = 0.0f;
    sideFlipFeetPadding[2] = 0.0f;
    sideFlipFeetPadding[3] = 0.0f;
    sideFlipFeetPadding[4] = 0.0f;
    sideFlipFeetPadding[5] = 0.0f;
    sideFlipFeetPadding[6] = 0.0f; // SideFlip7, the post-landing frame

    // Jump: classic (4 frames, velocity-based)
    oldJumpTextures.resize(4);
    oldJumpTextures[0].loadFromFile("assets/Jump01.png");
    oldJumpTextures[1].loadFromFile("assets/Jump02.png");
    oldJumpTextures[2].loadFromFile("assets/Jump03.png");
    oldJumpTextures[3].loadFromFile("assets/Jump04.png");

    oldJumpFeetPadding[0] = 0.0f;
    oldJumpFeetPadding[1] = 0.0f;
    oldJumpFeetPadding[2] = 0.0f;
    oldJumpFeetPadding[3] = 0.0f;

    // Roll (5 frames, mid-air fast-fall)
    rollTextures.resize(5);
    rollTextures[0].loadFromFile("assets/Roll1.png");
    rollTextures[1].loadFromFile("assets/Roll2.png");
    rollTextures[2].loadFromFile("assets/Roll3.png");
    rollTextures[3].loadFromFile("assets/Roll4.png");
    rollTextures[4].loadFromFile("assets/Roll5.png");

    // Fast by default since roll is meant to feel quick and punchy.
    // rollSpeedMultiplier scales overall pace; individual frame durations
    // can be set separately.
    rollFrameDurations[0] = 0.06f;
    rollFrameDurations[1] = 0.06f;
    rollFrameDurations[2] = 0.06f;
    rollFrameDurations[3] = 0.06f;
    rollFrameDurations[4] = 0.06f;

    rollFeetPadding[0] = 30.0f;
    rollFeetPadding[1] = 50.0f;
    rollFeetPadding[2] = 80.0f;
    rollFeetPadding[3] = 80.0f;
    rollFeetPadding[4] = 70.0f;

    // Slide (4 frames)
    slideTextures.resize(4);
    slideTextures[0].loadFromFile("assets/Slide1.png");
    slideTextures[1].loadFromFile("assets/Slide2.png");
    slideTextures[2].loadFromFile("assets/Slide3.png");
    slideTextures[3].loadFromFile("assets/Slide4.png");

    // Death (4 frames)
    deathTextures.resize(4);
    deathTextures[0].loadFromFile("assets/Death1.png");
    deathTextures[1].loadFromFile("assets/Death2.png");
    deathTextures[2].loadFromFile("assets/Death3.png");
    deathTextures[3].loadFromFile("assets/Death4.png");

    deathFrameDurations[0] = 0.15f;
    deathFrameDurations[1] = 0.15f;
    deathFrameDurations[2] = 0.15f;
    deathFrameDurations[3] = 0.20f;

    deathFeetPadding[0] = 10.0f;
    deathFeetPadding[1] = 60.0f;
    deathFeetPadding[2] = 110.0f;
    deathFeetPadding[3] = 140.0f;

    // Initial sprite setup: starts on Run frame 0
    sprite.emplace(runTextures[0]);

    float texH = runTextures[0].getSize().y;
    float scale = 110.0f / texH;
    sprite->setScale({ scale, scale });

    sf::Vector2u size0 = runTextures[0].getSize();
    sprite->setOrigin({ size0.x / 2.0f, static_cast<float>(size0.y) });

    sprite->setPosition(position);
}

void Player::setFrameTexture(sf::Texture& tex, float feetPadding)
{
    sprite->setTexture(tex, true);
    sf::Vector2u size = tex.getSize();
    sprite->setOrigin({ size.x / 2.0f, static_cast<float>(size.y) - feetPadding });
}

void Player::updateAnimation(float deltaTime)
{
    // Death
    if (state == PlayerState::Dead)
    {
        if (deathFinished)
        {
            return;
        }

        deathFrameTimer += deltaTime * deathSpeedMultiplier;

        if (deathFrameTimer >= deathFrameDurations[deathFrameIndex])
        {
            deathFrameTimer = 0.0f;
            deathFrameIndex++;

            if (deathFrameIndex >= 4)
            {
                deathFrameIndex = 3;
                deathFinished = true;
                deathFinalFrameEvent = true;
            }

            setFrameTexture(deathTextures[deathFrameIndex], deathFeetPadding[deathFrameIndex]);
        }

        return;
    }

    // Sliding
    if (isSliding)
    {
        slideTimer -= deltaTime;

        if (slideTimer <= 0.0f)
        {
            isSliding = false;
            state = PlayerState::Running;
            currentFrame = 0;
            slideTimer = 0.0f;
            slideCooldownTimer = slideCooldownDuration; // must get up briefly before sliding again
        }
        else
        {
            float progress = 1.0f - (slideTimer / slideDuration);

            if (progress < 0.10f)
            {
                setFrameTexture(slideTextures[0], 60.0f);
            }
            else if (progress < 0.75f)
            {
                setFrameTexture(slideTextures[1], 80.0f);
            }
            else if (progress < 0.85f)
            {
                setFrameTexture(slideTextures[2], 50.0f);
            }
            else
            {
                setFrameTexture(slideTextures[3], 40.0f);
            }
        }

        return;
    }

    // Roll (mid-air fast-fall): frame cycling only. Position/phase logic
    // lives in updateRollPhysics(), called separately.
    if (state == PlayerState::Rolling)
    {
        if (rollFrameIndex < 4)
        {
            rollFrameTimer += deltaTime * rollSpeedMultiplier;

            if (rollFrameTimer >= rollFrameDurations[rollFrameIndex])
            {
                rollFrameTimer = 0.0f;
                rollFrameIndex++;
                setFrameTexture(rollTextures[rollFrameIndex], rollFeetPadding[rollFrameIndex]);
            }
        }

        return;
    }

    // Front-flip jump landing pause
    if (landingTimer > 0.0f)
    {
        landingTimer -= deltaTime;
        if (landingTimer <= 0.0f)
        {
            state = PlayerState::Running;
            currentFrame = 0;
            setFrameTexture(runTextures[0], runFeetPadding[0]);
        }
        return;
    }

    // Running
    // Threshold divided by runAnimSpeedMultiplier (set by Game each
    // frame, grows slightly over survival time); higher multiplier means
    // a smaller threshold, so frames advance faster for a quicker
    // leg-cycle.
    animTimer += deltaTime;

    if (animTimer >= (animSpeed / runAnimSpeedMultiplier))
    {
        animTimer = 0.0f;

        if (state == PlayerState::Running)
        {
            currentFrame = (currentFrame + 1) % 8;
            setFrameTexture(runTextures[currentFrame], runFeetPadding[currentFrame]);

            // Footstep events: frame 2 (3rd frame) is left foot plant,
            // frame 6 (7th frame) is right foot plant. Only reachable
            // here since this whole block is gated on Running.
            if (currentFrame == 2) footstepLeftEvent = true;
            if (currentFrame == 6) footstepRightEvent = true;
        }
    }

    // Jumping (front flip, back flip, or classic; picked randomly in jump())
    if (state == PlayerState::Jumping)
    {
        if (useFrontFlip)
        {
            // Sequential slideshow through all 6 frames, sped up or
            // slowed by jumpSpeedMultiplier as one dial. Doesn't check
            // position; it cycles on its own schedule based on each
            // frame's duration. If it finishes before landing, it holds
            // on frame 6 (Jump6) until touchdown.
            if (jumpFrameIndex < 5)
            {
                jumpFrameTimer += deltaTime * jumpSpeedMultiplier;

                if (jumpFrameTimer >= jumpFrameDurations[jumpFrameIndex])
                {
                    jumpFrameTimer = 0.0f;
                    jumpFrameIndex++;
                    setFrameTexture(jumpTextures[jumpFrameIndex], jumpFeetPadding[jumpFrameIndex]);
                }
            }
        }
        else if (useBackFlip)
        {
            // Same sequential-slideshow approach as front flip, with its
            // own texture set and timers; see the comment above.
            if (backFlipFrameIndex < 5)
            {
                backFlipFrameTimer += deltaTime * backFlipSpeedMultiplier;

                if (backFlipFrameTimer >= backFlipFrameDurations[backFlipFrameIndex])
                {
                    backFlipFrameTimer = 0.0f;
                    backFlipFrameIndex++;
                    setFrameTexture(backFlipTextures[backFlipFrameIndex], backFlipFeetPadding[backFlipFrameIndex]);
                }
            }
        }
        else if (useSideFlip)
        {
            // Same sequential-slideshow approach as front/back flip.
            if (sideFlipFrameIndex < 5)
            {
                sideFlipFrameTimer += deltaTime * sideFlipSpeedMultiplier;

                if (sideFlipFrameTimer >= sideFlipFrameDurations[sideFlipFrameIndex])
                {
                    sideFlipFrameTimer = 0.0f;
                    sideFlipFrameIndex++;
                    setFrameTexture(sideFlipTextures[sideFlipFrameIndex], sideFlipFeetPadding[sideFlipFrameIndex]);
                }
            }
        }
        else
        {
            // Classic jump: frame picked from current vertical speed,
            // no timer needed.
            if (velocity.y < -500)
            {
                setFrameTexture(oldJumpTextures[0], oldJumpFeetPadding[0]);
            }
            else if (velocity.y < -300)
            {
                setFrameTexture(oldJumpTextures[1], oldJumpFeetPadding[1]);
            }
            else if (velocity.y < 300)
            {
                setFrameTexture(oldJumpTextures[2], oldJumpFeetPadding[2]);
            }
            else
            {
                setFrameTexture(oldJumpTextures[3], oldJumpFeetPadding[3]);
            }
        }
    }
}

void Player::applyGravity(float deltaTime)
{
    if (isOnGround)
    {
        float diff = currentGroundLevel - position.y; // negative = new ground is above the player

        if (std::abs(diff) > 0.5f)
        {
            if (diff < 0.0f)
            {
                // Ground rose (for example, stepping from a lower car
                // segment onto a higher one, like hood to windshield).
                // Climbs smoothly instead of falling then snapping,
                // avoiding a freeze-then-teleport look when walking up
                // the car.
                float climbSpeed = 800.0f; // pixels/sec climbing speed for a step up
                float maxStep = climbSpeed * deltaTime;

                if (std::abs(diff) <= maxStep)
                {
                    position.y = currentGroundLevel;
                }
                else
                {
                    position.y -= maxStep;
                }
                // Stays isOnGround == true the whole time; this is a
                // climb, not a fall, so no jump/roll state should fire.
            }
            else
            {
                // Small downward changes are still part of a continuous
                // walkable surface (for example, moving from one sloped
                // car segment to the next). Follow the surface instead of
                // briefly entering falling physics, which causes scraping/
                // moonwalking at segment joins.
                float dropSpeed = 800.0f; // pixels/sec
                float maxStep = dropSpeed * deltaTime;

                if (diff <= maxStep)
                {
                    // Small drop: stay grounded and follow the new surface.
                    position.y = currentGroundLevel;
                    velocity.y = 0.0f;
                }
                else
                {
                    // Large drop: this is actually walking off the obstacle.
                    isOnGround = false;
                }
            }
        }
    }

    if (!isOnGround)
    {
        // jumpPhysicsSpeedMultiplier affects both flip types; classic
        // jump always uses the plain gravity value, unaffected by this.
        // gravity * multiplier^2 keeps jump height constant while
        // scaling how fast the whole up-and-down trip happens (matches
        // launch velocity also being scaled by multiplier in jump()
        // below).
        float activeGravity = (useFrontFlip || useBackFlip || useSideFlip)
            ? gravity * jumpPhysicsSpeedMultiplier * jumpPhysicsSpeedMultiplier
            : gravity;
        velocity.y += activeGravity * deltaTime;
        position.y += velocity.y * deltaTime;

        if (position.y >= currentGroundLevel)
        {
            position.y = currentGroundLevel;
            velocity.y = 0.0f;
            isOnGround = true;

            bool wasJumping = (state == PlayerState::Jumping);

            if (wasJumping && useFrontFlip)
            {
                // Forces the front-flip animation to its last frame on
                // touchdown, regardless of which frame it was on,
                // guaranteeing the player lands visually upright
                // independent of animation-vs-airtime timing.
                jumpFrameIndex = 5;
                setFrameTexture(jumpTextures[5], jumpFeetPadding[5]);
                landingTimer = landingDuration;
                jumpLandedEvent = true;
            }
            else if (wasJumping && useBackFlip)
            {
                // Same as front flip, using back flip's own frame array/index.
                backFlipFrameIndex = 5;
                setFrameTexture(backFlipTextures[5], backFlipFeetPadding[5]);
                landingTimer = landingDuration;
                jumpLandedEvent = true;
            }
            else if (wasJumping && useSideFlip)
            {
                // Points at the dedicated 7th frame (SideFlip7, index 6)
                // on touchdown; this is the only place that sets it, so
                // it can't show mid-air.
                sideFlipFrameIndex = 6;
                setFrameTexture(sideFlipTextures[6], sideFlipFeetPadding[6]);
                landingTimer = landingDuration;
                jumpLandedEvent = true;
            }
            else
            {
                // Classic jump, roll, or any other airborne state (e.g.
                // falling off the edge of a crate while already
                // Running): snap straight back to Running, no dedicated
                // landing pose needed.
                state = PlayerState::Running;
                currentFrame = 0;
                setFrameTexture(runTextures[0], runFeetPadding[0]);

                if (wasJumping)
                {
                    // Classic (non-flip) jump landing: still a real jump
                    // landing, just no special pose held.
                    jumpLandedEvent = true;
                }
            }
        }
    }
}

void Player::updateDeathPhysics(float deltaTime)
{
    velocity.y += gravity * deltaTime;
    position.y += velocity.y * deltaTime;
    position.x += velocity.x * deltaTime;

    // Knockback can't cross the stop limit set in die(), in whichever
    // direction it's travelling (nearest obstacle edge in that direction
    // at the moment of death, +/-100000 = no limit if nothing was
    // nearby). Stops the player sliding off a car's back/front edge or
    // straight through a second car sitting close by on the ground; same
    // idea for backward (default) and forward (front-hit) knockback.
    if (velocity.x < 0.0f && position.x < deathStopX)
    {
        position.x = deathStopX;
        velocity.x = 0.0f;
    }
    else if (velocity.x > 0.0f && position.x > deathStopX)
    {
        position.x = deathStopX;
        velocity.x = 0.0f;
    }

    // Lands on currentGroundLevel (whatever surface Game.cpp last
    // confirmed the player was on, such as a car's roof), not the fixed
    // base-road groundY, so death lands on the correct surface even when
    // it happens while standing on top of a vehicle. Tire is
    // deactivated, so this code doesn't need to handle that collision
    // type.
    if (position.y >= currentGroundLevel)
    {
        position.y = currentGroundLevel;
        velocity.y = 0.0f;
        velocity.x = 0.0f;
        isOnGround = true;
        deathLanded = true;

        if (!deathFinished)
        {
            deathFrameIndex = 3;
            deathFrameTimer = 0.0f;
            deathFinished = true;
            deathFinalFrameEvent = true;
            setFrameTexture(deathTextures[3], deathFeetPadding[3]);
        }
    }
}

void Player::updateRollPhysics(float deltaTime)
{
    rollElapsed += deltaTime * rollSpeedMultiplier;

    // Phase 1 (frames 0-1): smoothly descend from wherever the roll
    // started down to ground level, over this many seconds.
    float airDuration = rollFrameDurations[0] + rollFrameDurations[1];

    if (rollElapsed < airDuration)
    {
        float t = rollElapsed / airDuration; // 0..1
        position.y = rollStartY + (currentGroundLevel - rollStartY) * t;
    }
    else
    {
        // Phase 2 (frames 2-4): locked at ground level, read live each
        // frame rather than cached, so roll tracks a walk-free zone
        // that's about to scroll out of range instead of plowing
        // through it.
        position.y = currentGroundLevel;
        isOnGround = true;
    }

    // Roll finished; back to normal running.
    float totalDuration = airDuration + rollFrameDurations[2] + rollFrameDurations[3] + rollFrameDurations[4];
    if (rollElapsed >= totalDuration)
    {
        state = PlayerState::Running;
        currentFrame = 0;
        isOnGround = true;
        position.y = currentGroundLevel;
        setFrameTexture(runTextures[0], runFeetPadding[0]);
    }
}

void Player::beginRun()
{
    // Called by Game::startGame() to restart the run animation and force
    // the run texture; resets currentFrame so the run cycle starts clean
    // each time.
    state = PlayerState::Running;
    currentFrame = 0;
    setFrameTexture(runTextures[0], runFeetPadding[0]);
}

void Player::jump()
{
    if (isOnGround && !isSliding && state != PlayerState::Dead)
    {
        isOnGround = false;
        state = PlayerState::Jumping;
        currentFrame = 0;

        // 70% classic jump, 10% front flip, 10% back flip, 10% side
        // flip, picked fresh each time. Back flip and side flip only
        // trigger when jumping from an elevated surface (standing on top
        // of a car, not base ground); currentGroundLevel is set every
        // frame by Game.cpp's collision system and sits below groundY
        // (smaller Y = higher on screen) in that case. The odds are
        // unaffected either way: if back flip's or side flip's slot
        // comes up while on plain ground, it falls through to classic.
        bool onCar = currentGroundLevel < groundY - 1.0f;

        int jumpRoll = std::rand() % 100;
        useFrontFlip = jumpRoll < 10;
        useBackFlip = onCar && (jumpRoll >= 10 && jumpRoll < 20);
        useSideFlip = onCar && (jumpRoll >= 20 && jumpRoll < 30); // 10% chance, same slot size as back flip; car only

        // All flip types launch with the same scaled speed; classic jump
        // always launches at the plain -600.0f, untouched by that knob.
        bool isFlip = useFrontFlip || useBackFlip || useSideFlip;
        velocity.y = isFlip ? (-600.0f * jumpPhysicsSpeedMultiplier) : -600.0f;

        // Van flip slow-motion: only when this jump is an actual flip
        // (classic jump never triggers it) and launched specifically
        // from a Van's surface (not Jeep, not plain ground). One fresh
        // 50/50 roll per eligible jump; Game.cpp picks this event up
        // next frame via consumeVanFlipSlowMoEvent() and drives the
        // slow-motion timeline and sound from there.
        if (isFlip && standingOnVan && (std::rand() % 100 < 50)) {
            vanFlipSlowMoEvent = true;
        }

        if (useFrontFlip)
        {
            jumpFrameIndex = 0;
            jumpFrameTimer = 0.0f;
            setFrameTexture(jumpTextures[0], jumpFeetPadding[0]);
        }
        else if (useBackFlip)
        {
            backFlipFrameIndex = 0;
            backFlipFrameTimer = 0.0f;
            setFrameTexture(backFlipTextures[0], backFlipFeetPadding[0]);
        }
        else if (useSideFlip)
        {
            sideFlipFrameIndex = 0;
            sideFlipFrameTimer = 0.0f;
            setFrameTexture(sideFlipTextures[0], sideFlipFeetPadding[0]);
        }
        else
        {
            setFrameTexture(oldJumpTextures[0], oldJumpFeetPadding[0]);
        }
    }
}

void Player::slide()
{
    if (isOnGround && !isSliding && state != PlayerState::Dead && slideCooldownTimer <= 0.0f)
    {
        isSliding = true;
        state = PlayerState::Sliding;
        currentFrame = 0;

        slideTimer = slideDuration;

        setFrameTexture(slideTextures[0], 80.0f);
    }
}

void Player::roll()
{
    // Only makes sense while actually airborne from a jump, and blocked
    // during a back flip or side flip specifically, since dropping into
    // a scripted fast-fall mid-spin looks wrong. Front flip is
    // untouched; roll still works normally there.
    if (state != PlayerState::Jumping || useBackFlip || useSideFlip)
    {
        return;
    }

    state = PlayerState::Rolling;
    isOnGround = false;
    velocity = { 0.0f, 0.0f }; // roll is fully scripted, not gravity-driven

    rollStartY = position.y;  // phase 1 descends smoothly from here
    rollElapsed = 0.0f;
    rollFrameIndex = 0;
    rollFrameTimer = 0.0f;

    setFrameTexture(rollTextures[0], rollFeetPadding[0]);
}

void Player::onDownPressed()
{
    // Single entry point for the Down key; decides slide vs roll based
    // on current context, so Game.cpp doesn't need to know Player's
    // internal state to make that call.
    if (isOnGround && !isSliding && state != PlayerState::Dead)
    {
        slide();
    }
    else if (state == PlayerState::Jumping)
    {
        roll();
    }
}

void Player::die(float stopX, bool knockForward)
{
    if (state == PlayerState::Dead)
    {
        return;
    }

    state = PlayerState::Dead;
    isSliding = false;

    isOnGround = false;
    // knockForward = true means the death was caused by hitting a
    // vehicle's front (nose) side; see Game::triggerDeath(). The
    // vehicle's body extends in -x from a front-hit point (nose is
    // always the higher-X edge), so the usual backward knock would
    // send the player straight into it; flipping to +x here sends him
    // forward into open road instead. deathKnockbackSpeed is stored
    // negative, so negating it gives the forward (+x) speed.
    velocity.x = knockForward ? -deathKnockbackSpeed : deathKnockbackSpeed;
    velocity.y = deathBounceSpeed;
    deathStopX = stopX;

    deathFrameIndex = 0;
    deathFrameTimer = 0.0f;
    deathFinished = false;
    deathLanded = false;
    deathFinalFrameEvent = false;

    setFrameTexture(deathTextures[0], deathFeetPadding[0]);
}

void Player::reset()
{
    position = { 140.0f, 675.0f };
    velocity = { 0.0f, 0.0f };
    isOnGround = true;
    currentGroundLevel = groundY;
    isSliding = false;
    slideTimer = 0.0f;
    slideCooldownTimer = 0.0f;

    // beginRun() gets called right after reset() by Game::startGame()
    // and sets state/currentFrame/texture itself, so this is just a
    // safe default in between.
    state = PlayerState::Running;
    currentFrame = 0;
    animTimer = 0.0f;

    jumpFrameIndex = 0;
    jumpFrameTimer = 0.0f;

    backFlipFrameIndex = 0;
    backFlipFrameTimer = 0.0f;

    sideFlipFrameIndex = 0;
    sideFlipFrameTimer = 0.0f;

    rollFrameIndex = 0;
    rollFrameTimer = 0.0f;
    rollElapsed = 0.0f;
    rollStartY = 0.0f;
    jumpLandedEvent = false;

    deathFrameIndex = 0;
    deathFrameTimer = 0.0f;
    deathFinished = false;
    deathLanded = false;
    deathFinalFrameEvent = false;

    sprite.emplace(runTextures[0]);
    float texH = runTextures[0].getSize().y;
    float scale = 105.0f / texH;
    sprite->setScale({ scale, scale });

    sf::Vector2u size0 = runTextures[0].getSize();
    sprite->setOrigin({ size0.x / 2.0f, static_cast<float>(size0.y) });
    sprite->setPosition(position);
}

void Player::update(float deltaTime)
{
    if (slideCooldownTimer > 0.0f)
    {
        slideCooldownTimer -= deltaTime;
    }

    if (state == PlayerState::Dead)
    {
        updateDeathPhysics(deltaTime);
        updateAnimation(deltaTime);
        sprite->setPosition(position);
        return;
    }

    if (state == PlayerState::Rolling)
    {
        updateRollPhysics(deltaTime);
        updateAnimation(deltaTime);
        sprite->setPosition(position);
        return;
    }

    applyGravity(deltaTime);
    updateAnimation(deltaTime);
    sprite->setPosition(position);
}

void Player::draw(sf::RenderWindow& window, float sizeMultiplier)
{
    // Shadow drawn first, underneath the body, only during normal
    // gameplay rendering (sizeMultiplier == 1.0), not the menu preview.
    // Shrinks/fades as he gets higher off the ground, based on
    // currentGroundLevel (the actual surface he'd land on, a car's roof
    // counts too, not just the fixed base road) rather than position.y
    // directly, so the shadow always tracks real ground.
    if (sizeMultiplier == 1.0f && shadowSprite.has_value() && sprite.has_value()) {
        sf::Vector2u shadeTexSize = shadeTexture.getSize();
        if (shadeTexSize.x > 0) {
            sf::FloatRect bodyBounds = sprite->getGlobalBounds();

            float heightOffGround = std::max(0.0f, currentGroundLevel - position.y);
            const float maxJumpHeight = 120.0f; // max jump height in pixels; see the shadow-loading comment in the constructor for the formula
            float heightFrac = std::min(heightOffGround / maxJumpHeight, 1.0f);

            // Shrinks down to 55% size and fades to about 40% opacity at
            // the peak of a jump.
            float shadowScaleFactor = 1.0f - (heightFrac * 0.45f);
            std::uint8_t shadowAlpha = static_cast<std::uint8_t>(255.0f - (heightFrac * (255.0f - 100.0f)));

            const float shadowWidthFrac = 0.55f; // flat fraction of the player's body width; there's no wheelbase to measure, unlike the vehicles
            float shadowWidth = bodyBounds.size.x * shadowWidthFrac * shadowScaleFactor;
            float shadowScale = shadowWidth / shadeTexSize.x;
            shadowSprite->setScale({shadowScale, shadowScale});
            shadowSprite->setOrigin({shadeTexSize.x / 2.0f, shadeTexSize.y / 2.0f});
            shadowSprite->setColor(sf::Color(0, 0, 0, shadowAlpha));
            shadowSprite->setPosition({
                bodyBounds.position.x + bodyBounds.size.x / 2.0f,

                currentGroundLevel -4
            });
            window.draw(*shadowSprite);
        }
    }

    // sizeMultiplier only affects this draw call: the sprite's scale is
    // restored right after, so it never persists. That matters because
    // getBounds() (the gameplay hitbox) reads the sprite's scale
    // directly, so a bigger menu render would otherwise also mean a
    // bigger, harder-to-dodge hitbox in an actual run. Restoring
    // immediately keeps the two completely independent, so the menu can
    // look different from gameplay without gameplay being affected at
    // all.
    if (sizeMultiplier != 1.0f) {
        sf::Vector2f originalScale = sprite->getScale();
        sprite->setScale({ originalScale.x * sizeMultiplier, originalScale.y * sizeMultiplier });
        window.draw(*sprite);
        sprite->setScale(originalScale);
    } else {
        window.draw(*sprite);
    }
}

bool Player::consumeFootstepLeftEvent()
{
    bool value = footstepLeftEvent;
    footstepLeftEvent = false;
    return value;
}

bool Player::consumeFootstepRightEvent()
{
    bool value = footstepRightEvent;
    footstepRightEvent = false;
    return value;
}

bool Player::consumeJumpLandedEvent()
{
    bool value = jumpLandedEvent;
    jumpLandedEvent = false; // consumed: won't report true again until next landing
    return value;
}

bool Player::consumeVanFlipSlowMoEvent()
{
    bool value = vanFlipSlowMoEvent;
    vanFlipSlowMoEvent = false; // consumed: won't report true again until the next eligible flip
    return value;
}

bool Player::consumeDeathFinalFrameEvent()
{
    bool value = deathFinalFrameEvent;
    deathFinalFrameEvent = false;
    return value;
}

sf::FloatRect Player::getBounds() const
{
    sf::FloatRect bounds = sprite->getGlobalBounds();

    float left, right, top, bottom;

    if (state == PlayerState::Sliding)
    {
        left = 15.0f;
        right = 8.0f;
        top = 30.0f;
        bottom = 20.0f;
    }
    else if (state == PlayerState::Jumping)
    {
        left = 28.0f;
        right = 32.0f;
        top = 23.0f;
        bottom = 15.0f;
    }

    else if (state == PlayerState::Rolling)

    {

        left = 28.0f;

        right = 28.0f;

        top = 28.0f;

        bottom = 15.0f;

    }
    else
    {
        left = 35.0f;
        right = 35.0f;
        top = 5.0f;
        bottom = 5.0f;
    }

    return sf::FloatRect(
        { bounds.position.x + left, bounds.position.y + top },
        { bounds.size.x - left - right, bounds.size.y - top - bottom }
    );
}
