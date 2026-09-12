#include "Background.h"
#include "GameConfig.h"
#include "Obstacle.h" // for Obstacle::getCarWheelLayout() -- uplane Car mirror source
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <string>

Background::Background() : scaledWidth(0.0f), nextTextureIndex(0),
    // BACKGROUND Y:
    backgroundY(90.0f),
    // BACKGROUND BASE SPEED: background's speed at survival-time-zero
    // = GROUND_SPEED (the constant from GameConfig.h) * this number.
    // Lower = slower background
    backgroundSpeedMultiplier(0.6f),
    // BACKGROUND GROWTH DAMPENING: how much of the world's speed
    // growth-over-time carries over to the background. 1.0 = grows
    // exactly as fast as gameplay speed.
    backgroundGrowthDampening(0.8f),
    skyScaledWidth(0.0f), skyNextTextureIndex(0), skyY(-100.0f),
    // SKY SPEED: fraction of the current ground speed
    skySpeedMultiplier(0.08f),
    // SKY OVERLAP: pulls each recycled sky image in by this many
    // pixels so there's never a thin gap line at the joint.
    skyOverlapAmount(4.0f),
    farBuildingScaledWidth(0.0f), farBuildingNextTextureIndex(0),
    // FAR BUILDINGS SPEED: just above skySpeedMultiplier so it's
    // barely noticeably faster than the sky, giving extra parallax
    // depth without being distracting.
    farBuildingSpeedMultiplier(0.20f),
    // FAR BUILDINGS Y:
    farBuildingY(35.0f),
    // FAR BUILDINGS OVERLAP:
    farBuildingOverlapAmount(4.0f),
    midBuildingScaledWidth(0.0f), midBuildingNextTextureIndex(0),
    // MID BUILDINGS SPEED:
    midBuildingSpeedMultiplier(0.38f),
    // MID BUILDINGS Y:
    midBuildingY(85.0f),
    midBuildingOverlapAmount(4.0f),
    // BACKGROUND OVERLAP:
    backgroundOverlapAmount(4.0f),
    currentBackgroundSpeed(0.0f),
    tileScaledWidth(0.0f), tileNextTextureIndex(0),
    // TILE SIZE: independent width/height...
    tileTargetWidth(1900.0f),
    tileTargetHeight(235.0f),
    // TILE GROUND LEVEL: vertical position...
    tileGroundY(475.0f),
    // TILE OVERLAP:
    tileOverlapAmount(4.0f),
    // TILE SPEED: multiplier of the LIVE ground speed
    tileSpeedMultiplier(1.6f),

    // UP-LANE TRAFFIC:
    upLaneCarSpawnTimer(0.0f),
    upLaneCarSpawnInterval(6.0f),
    upLaneWheelRotationFactor(1.0f),
    farBuildingsVisible(true),
    helicopterActive(false),
    helicopterSpawnEvent(false),
    helicopterTimer(0.0f),
    helicopterSpawnInterval(110.0f),
    // HELICOPTER SPEED MULTIPLIER: effective speed = groundSpeed *
    // this number.
    helicopterSpeedMultiplier(240.0f / 350.0f),
    // HELICOPTER SPAWN X: Horizontal starting position...
    helicopterSpawnX(-300.0f),
    helicopterY(-50.0f),
    helicopterBodyWidth(0.0f)
{
    Obstacle::loadTextures();

    // LAYER 1: SKY (drawn first/furthest back) to 2 images, moves
    // VERY slowly.
    skyTextures.resize(NUM_SKY_IMAGES);
    skyTextures[0].loadFromFile("assets/Sky1.png");
    skyTextures[1].loadFromFile("assets/Sky2.png");

    // SKY SIZE: scaled by HEIGHT only (uniform, preserves the image's
    // own proportions)
    float skyTargetHeight = 420.0f;
    float skyScale = skyTargetHeight / skyTextures[0].getSize().y;
    skyScaledWidth = skyTextures[0].getSize().x * skyScale;

    // SKY SPACING: see skyOverlapAmount above: pulls each image in
    // slightly so there's never a gap at the seam.
    float skySpacing = skyScaledWidth - skyOverlapAmount;

    for (int i = 0; i < 2; i++) {
        skySprites.emplace_back(skyTextures[i]);
        skySprites.back().setScale({ skyScale, skyScale });
        skySprites.back().setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * skySpacing, skyY });
    }
    skyNextTextureIndex = 2 % NUM_SKY_IMAGES;

    // LAYER 1.5: FAR BUILDINGS (drawn after sky, before the main
    // background city): 2 images, barely faster than sky.
    farBuildingTextures.resize(NUM_FARBUILDING_IMAGES);
    farBuildingTextures[0].loadFromFile("assets/FarBuildings1.png");
    farBuildingTextures[1].loadFromFile("assets/FarBuildings2.png");

    // FAR BUILDINGS SIZE: scaled by HEIGHT only, same pattern as sky.
    float farBuildingTargetHeight = 300.0f;
    float farBuildingScale = farBuildingTargetHeight / farBuildingTextures[0].getSize().y;
    farBuildingScaledWidth = farBuildingTextures[0].getSize().x * farBuildingScale;

    float farBuildingSpacing = farBuildingScaledWidth - farBuildingOverlapAmount;

    for (int i = 0; i < 2; i++) {
        farBuildingSprites.emplace_back(farBuildingTextures[i]);
        farBuildingSprites.back().setScale({ farBuildingScale, farBuildingScale });
        farBuildingSprites.back().setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * farBuildingSpacing, farBuildingY });
    }
    farBuildingNextTextureIndex = 2 % NUM_FARBUILDING_IMAGES;
    
    // LAYER 1.75: MID BUILDINGS: 3 images, between far and close.
    midBuildingTextures.resize(NUM_MIDBUILDING_IMAGES);
    midBuildingTextures[0].loadFromFile("assets/MidBuildings1.png");
    midBuildingTextures[1].loadFromFile("assets/MidBuildings2.png");
    midBuildingTextures[2].loadFromFile("assets/MidBuildings3.png");

    float midBuildingTargetHeight = 300.0f;
    float midBuildingScale = midBuildingTargetHeight / midBuildingTextures[0].getSize().y;
    midBuildingScaledWidth = midBuildingTextures[0].getSize().x * midBuildingScale;

    float midBuildingSpacing = midBuildingScaledWidth - midBuildingOverlapAmount;

    for (int i = 0; i < 3; i++) {
        midBuildingSprites.emplace_back(midBuildingTextures[i]);
        midBuildingSprites.back().setScale({ midBuildingScale, midBuildingScale });
        midBuildingSprites.back().setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * midBuildingSpacing, midBuildingY });
    }
    midBuildingNextTextureIndex = 3 % NUM_MIDBUILDING_IMAGES;

    // LAYER 2: BACKGROUND CITY (drawn second, middle) - 40 images.
    bgTextures.resize(NUM_BG_IMAGES_TOTAL);
    for (int i = 0; i < NUM_BG_IMAGES_TOTAL; i++) {
        std::string path = "assets/Background" + std::to_string(i + 1) + ".png";
        bgTextures[i].loadFromFile(path);
        if (bgTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << path << " -- check that this file exists in assets/ and the filename/number matches.\n";
        }
    }

    float bgTargetHeight = 480.0f;
    float bgScale = bgTargetHeight / bgTextures[0].getSize().y;
    scaledWidth = bgTextures[0].getSize().x * bgScale;

    // BACKGROUND SPACING: Each image is placed backgroundOverlapAmount pixels closer than
    // its full width so there's never a gap at the seam. Exactly the
    // "Background2 starts at 99 instead of 100" idea... the next
    // image is already slightly lapping the previous one's tail end
    // before the scroll brings them together, so no clean full-width
    // gap can ever show.
    float bgSpacing = scaledWidth - backgroundOverlapAmount;

    for (int i = 0; i < 3; i++) {
        bgSprites.emplace_back(bgTextures[i]);
        bgSprites.back().setScale({ bgScale, bgScale });
        bgSprites.back().setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * bgSpacing, backgroundY });
        bgSpriteTexIndex.push_back(i);
    }
    nextTextureIndex = 3 % NUM_BG_IMAGES;

    // LAYER 3: TILES... 3 images, Tile1.png / Tile2.png / Tile3.png,
    // fixed cycling order (1,2,3,1,2,3...). Width/height independent,
    // ground position ("gravity") and speed all adjustable below.
    tileTextures.resize(NUM_TILE_IMAGES);
    if (!tileTextures[0].loadFromFile("assets/Tiles1.png")) {
        std::cerr << "Warning: could not load assets/Tiles1.png -- tile layer will be blank at this slot.\n";
    }
    if (!tileTextures[1].loadFromFile("assets/Tiles2.png")) {
        std::cerr << "Warning: could not load assets/Tiles2.png -- tile layer will be blank at this slot.\n";
    }
    if (!tileTextures[2].loadFromFile("assets/Tiles3.png")) {
        std::cerr << "Warning: could not load assets/Tiles3.png -- tile layer will be blank at this slot.\n";
    }

    float tileScaleX = tileTargetWidth / tileTextures[0].getSize().x;
    float tileScaleY = tileTargetHeight / tileTextures[0].getSize().y;

    // tileScaledWidth reads the sprite's ACTUAL rendered width (via
    // getGlobalBounds).
    sf::Sprite tileSizeProbe(tileTextures[0]);
    tileSizeProbe.setScale({ tileScaleX, tileScaleY });
    tileScaledWidth = tileSizeProbe.getGlobalBounds().size.x;

    float tileSpacing = tileScaledWidth - tileOverlapAmount;

    // 5 sprite instances, cycling through the 3 textures in order,
    // enough to always cover the screen with spares on both sides.
    for (int i = 0; i < 5; i++) {
        int texIndex = i % NUM_TILE_IMAGES;
        tileSprites.emplace_back(tileTextures[texIndex]);
        tileSprites.back().setScale({ tileScaleX, tileScaleY });
        tileSprites.back().setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * tileSpacing, tileGroundY });
    }
    tileNextTextureIndex = 5 % NUM_TILE_IMAGES;
    
    // UP-LANE TRAFFIC... decorative only, no collision. Loads once;
    // actual car instances are created/removed on the fly in
    // updateUpLaneTraffic(). 4 contiguous blocks: Car (mirrors the
    // real downlane Car, reusing its wheel art directly), Jeep (same
    // idea), Van (same idea), Truck (uplane-only, no downlane
    // counterpart to mirror).
    upLaneShadeLoaded = upLaneShadeTexture.loadFromFile("assets/Shade.png");
    if (!upLaneShadeLoaded) std::cerr << "Warning: could not load assets/Shade.png -- up-lane vehicle shadows will be invisible until this file exists.\n";

    upLaneCarBodyTextures.resize(NUM_UPLANE_VARIANTS);
    upLaneCarWheelTextures.resize(NUM_UPLANE_VARIANTS);
    upLaneWheelLayouts.resize(NUM_UPLANE_VARIANTS);

    // Car mirrors, indices UPLANE_CAR_START..+COUNT (15). Own body art
    // (facing left, oncoming), but the WHEEL is the exact same
    // downlane CarWheelN.png file, a circular tire looks identical
    // either direction, no separate uplane wheel art needed!
    for (int i = 0; i < UPLANE_CAR_COUNT; i++) {
        int idx = UPLANE_CAR_START + i;
        std::string bodyPath = "assets/UpLaneCarBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/CarWheel" + std::to_string(i + 1) + ".png"; // reused from downlane, unmirrored
        upLaneCarBodyTextures[idx].loadFromFile(bodyPath);
        if (upLaneCarBodyTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- up-lane car " << (i+1) << " will be invisible/broken until this file exists.\n";
        }
        upLaneCarWheelTextures[idx].loadFromFile(wheelPath);
        if (upLaneCarWheelTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << "\n";
        }
    }

    // Jeep mirrors, indices UPLANE_JEEP_START..+COUNT (10). Same idea
    // -- own body art, wheel reused directly from downlane JeepWheelN.png.
    for (int i = 0; i < UPLANE_JEEP_COUNT; i++) {
        int idx = UPLANE_JEEP_START + i;
        std::string bodyPath = "assets/UpLaneJeepBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/JeepWheel" + std::to_string(i + 1) + ".png"; // reused from downlane, unmirrored
        upLaneCarBodyTextures[idx].loadFromFile(bodyPath);
        if (upLaneCarBodyTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- up-lane jeep " << (i+1) << " will be invisible/broken until this file exists.\n";
        }
        upLaneCarWheelTextures[idx].loadFromFile(wheelPath);
        if (upLaneCarWheelTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << "\n";
        }
    }

    // Van mirrors, indices UPLANE_VAN_START..+COUNT (5). Same idea --
    // wheel reused directly from downlane VanWheelN.png.
    for (int i = 0; i < UPLANE_VAN_COUNT; i++) {
        int idx = UPLANE_VAN_START + i;
        std::string bodyPath = "assets/UpLaneVanBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/VanWheel" + std::to_string(i + 1) + ".png"; // reused from downlane, unmirrored
        upLaneCarBodyTextures[idx].loadFromFile(bodyPath);
        if (upLaneCarBodyTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- up-lane van " << (i+1) << " will be invisible/broken until this file exists.\n";
        }
        upLaneCarWheelTextures[idx].loadFromFile(wheelPath);
        if (upLaneCarWheelTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << "\n";
        }
    }

    // Truck, indices UPLANE_TRUCK_START..+COUNT (2). Uplane-only --
    // there's no downlane Truck to mirror from, so this has its own
    // standalone wheel art (TruckWheel1-2.png), not reused from
    // anywhere else.
    for (int i = 0; i < UPLANE_TRUCK_COUNT; i++) {
        int idx = UPLANE_TRUCK_START + i;
        std::string bodyPath = "assets/UpLaneTruckBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/TruckWheel" + std::to_string(i + 1) + ".png";
        upLaneCarBodyTextures[idx].loadFromFile(bodyPath);
        if (upLaneCarBodyTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- up-lane truck " << (i+1) << " will be invisible/broken until this file exists.\n";
        }
        upLaneCarWheelTextures[idx].loadFromFile(wheelPath);
        if (upLaneCarWheelTextures[idx].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << "\n";
        }
    }

    loadUpLaneSounds();

    // Same reallocation-safety reserve as obstacles in Game.cpp...
    // each car here owns a live sf::Sound too.
    upLaneCars.reserve(32);
    
    // WHEEL FINE-TUNE, PER UP-LANE VEHICLE.
    //
    // Car (0-39): COMPUTED MIRROR of downlane's real values, not
    // hand-typed, since every up-lane Car PNG is now
    // a guaranteed pixel-mirror of its downlane counterpart (done via
    // IbisPaintX's mirror feature only, no redrawing), the math can't
    // drift out of sync the way the old hand-typed table could.
    // X flips (1 - x, both the front/rear X fraction AND the X pixel
    // offset); Y/diameter/Y-offset carry over unchanged since mirroring
    // is purely horizontal.
    for (int i = 0; i < UPLANE_CAR_COUNT; i++) {
        // auto, not a named type... CarWheelLayout is a private nested
        // type inside Obstacle; only the ACCESSOR function itself is
        // public, so external code can use what it returns but can't
        // name the type directly (Obstacle::CarWheelLayout would fail
        // to compile here with a private-access error).
        const auto& dl = Obstacle::getCarWheelLayout(i);
        upLaneWheelLayouts[i].frontXFrac = 1.0f - dl.frontXFrac;
        upLaneWheelLayouts[i].frontYFrac = dl.frontYFrac;
        upLaneWheelLayouts[i].rearXFrac = 1.0f - dl.rearXFrac;
        upLaneWheelLayouts[i].rearYFrac = dl.rearYFrac;
        upLaneWheelLayouts[i].wheelDiameterFrac = dl.wheelDiameterFrac;
        upLaneWheelLayouts[i].frontOffsetX = -dl.frontOffsetX;
        upLaneWheelLayouts[i].frontOffsetY = dl.frontOffsetY;
        upLaneWheelLayouts[i].rearOffsetX = -dl.rearOffsetX;
        upLaneWheelLayouts[i].rearOffsetY = dl.rearOffsetY;
    }

    // Jeep (10): COMPUTED MIRROR of downlane's real values now too,
    // same pattern/reasoning as Car above.
    for (int i = 0; i < UPLANE_JEEP_COUNT; i++) {
        int idx = UPLANE_JEEP_START + i;
        const auto& dl = Obstacle::getJeepWheelLayout(i);
        upLaneWheelLayouts[idx].frontXFrac = 1.0f - dl.frontXFrac;
        upLaneWheelLayouts[idx].frontYFrac = dl.frontYFrac;
        upLaneWheelLayouts[idx].rearXFrac = 1.0f - dl.rearXFrac;
        upLaneWheelLayouts[idx].rearYFrac = dl.rearYFrac;
        upLaneWheelLayouts[idx].wheelDiameterFrac = dl.wheelDiameterFrac;
        upLaneWheelLayouts[idx].frontOffsetX = -dl.frontOffsetX;
        upLaneWheelLayouts[idx].frontOffsetY = dl.frontOffsetY;
        upLaneWheelLayouts[idx].rearOffsetX = -dl.rearOffsetX;
        upLaneWheelLayouts[idx].rearOffsetY = dl.rearOffsetY;
    }

    // Van PLACEHOLDER, set directly per variant. Also untouched by
    // the mirroring change.
    upLaneWheelLayouts[UPLANE_VAN_START + 0] = { 0.801f, 0.917f, 0.175f, 0.922f, 0.132f }; // Van1
    upLaneWheelLayouts[UPLANE_VAN_START + 1] = { 0.812f, 0.891f, 0.220f, 0.889f, 0.144f }; // Van2
    upLaneWheelLayouts[UPLANE_VAN_START + 2] = { 0.799f, 0.889f, 0.158f, 0.886f, 0.144f }; // Van3
    upLaneWheelLayouts[UPLANE_VAN_START + 3] = { 0.859f, 0.917f, 0.211f, 0.922f, 0.144f }; // Van4
    upLaneWheelLayouts[UPLANE_VAN_START + 4] = { 0.857f, 0.901f, 0.210f, 0.907f, 0.144f }; // Van5

    // Truck: no downlane counterpart, standalone placeholder numbers.
    upLaneWheelLayouts[UPLANE_TRUCK_START + 0] = { 0.792f, 0.913f, 0.107f, 0.908f, 0.106f }; // Truck1 - PLACEHOLDER
    upLaneWheelLayouts[UPLANE_TRUCK_START + 1] = { 0.788f, 0.933f, 0.146f, 0.936f, 0.107f }; // Truck2 - PLACEHOLDER


    // UP-LANE SIZE + SPEED + GROUND Y, PER VARIANT - one shared value
    // per TYPE block (Car/Jeep/Van/Truck), matching how each type
    // rendered before this extension. Jeep/Van/Truck sit taller and
    // LOWER (bigger groundY) than Car so their extra height doesn't
    // push them up into the down-lane. All still independently
    // overridable per-index below the loops if any one variant ever
    // needs to differ from its type.
    upLaneCarTargetHeights.resize(NUM_UPLANE_VARIANTS);
    upLaneCarSpeedMultipliers.resize(NUM_UPLANE_VARIANTS);
    upLaneCarGroundYs.resize(NUM_UPLANE_VARIANTS);

    for (int i = UPLANE_CAR_START; i < UPLANE_CAR_START + UPLANE_CAR_COUNT; i++) {
        upLaneCarTargetHeights[i] = 75.0f;
        upLaneCarSpeedMultipliers[i] = 2.25f; // was 3.0
        upLaneCarGroundYs[i] = 465.0f;
    }
    for (int i = UPLANE_JEEP_START; i < UPLANE_JEEP_START + UPLANE_JEEP_COUNT; i++) {
        upLaneCarTargetHeights[i] = 110.0f;
        upLaneCarSpeedMultipliers[i] = 2.4f; // was 3.0
        upLaneCarGroundYs[i] = 430.0f;
    }
    for (int i = UPLANE_VAN_START; i < UPLANE_VAN_START + UPLANE_VAN_COUNT; i++) {
        upLaneCarTargetHeights[i] = 130.0f;
        upLaneCarSpeedMultipliers[i] = 2.4f; // was 3.0
        upLaneCarGroundYs[i] = 400.0f;
    }
    for (int i = UPLANE_TRUCK_START; i < UPLANE_TRUCK_START + UPLANE_TRUCK_COUNT; i++) {
        upLaneCarTargetHeights[i] = 210.0f;
        upLaneCarSpeedMultipliers[i] = 2.55f; // was 3.0 -- kept slightly higher than the rest on purpose: all 4 types are now a TIGHT cluster (1.5-1.7) so no type can meaningfully close a gap on another over time, since nothing tracks the car ahead once spawned
        upLaneCarGroundYs[i] = 340.0f; // PUSH LOWER (increase) if it still sits too high/close to your lane
    }

    // HELICOPTER -- single body sprite.
    // 4 body variants loaded, one picked at random per spawn (see
    // updateHelicopter())... 25% chance each.
    heliBodyTextures.resize(NUM_HELI_VARIANTS);
    heliBodyTextures[0].loadFromFile("assets/Helicopter1.png");
    heliBodyTextures[1].loadFromFile("assets/Helicopter2.png");
    heliBodyTextures[2].loadFromFile("assets/Helicopter3.png");
    heliBodyTextures[3].loadFromFile("assets/Helicopter4.png");
    heliBodySprite.emplace(heliBodyTextures[0]);

    // HELICOPTER SIZE: your tuned value, preserved. All 4 variants
    // close enough in size to share this one scale/width.
    float heliScale = 150.0f / heliBodyTextures[0].getSize().y;
    heliBodySprite->setScale({heliScale, heliScale});

    helicopterBodyWidth = heliBodyTextures[0].getSize().x * heliScale;
}

void Background::updateHelicopter(float deltaTime, float groundSpeed, bool isPlaying) {
    if (!helicopterActive) {
        if (isPlaying) {
            helicopterTimer += deltaTime;
            if (helicopterTimer >= helicopterSpawnInterval) {
                helicopterTimer = 0.0f;
                helicopterActive = true;
                helicopterSpawnEvent = true;

                // Random variant, 25% chance each of the 4.
                int variant = std::rand() % NUM_HELI_VARIANTS;
                heliBodySprite->setTexture(heliBodyTextures[variant], true);
                heliBodySprite->setPosition({ helicopterSpawnX, helicopterY });
            }
        }
        return;
    }

    float effectiveHeliSpeed = groundSpeed * helicopterSpeedMultiplier;

    sf::Vector2f bodyPos = heliBodySprite->getPosition();
    bodyPos.x += effectiveHeliSpeed * deltaTime;
    heliBodySprite->setPosition(bodyPos);

    // HELICOPTER EXIT: uses VIEW_RIGHT_EDGE (GameConfig.h), not a
    // hardcoded number... matches the actual visible right edge of
    // the zoom-out view
    if (bodyPos.x > VIEW_RIGHT_EDGE) {
        helicopterActive = false;
    }
}

bool Background::consumeHelicopterSpawnEvent() {
    bool value = helicopterSpawnEvent;
    helicopterSpawnEvent = false;
    return value;
}

float Background::getHelicopterVolumeFactor() const {
    if (!helicopterActive) return 0.0f;

    float bodyX = heliBodySprite->getPosition().x;
    float travelStart = helicopterSpawnX;
    float travelEnd = VIEW_RIGHT_EDGE; // matches the exit check in updateHelicopter()

    float t = (travelStart - bodyX) / (travelStart - travelEnd);
    t = std::clamp(t, 0.0f, 1.0f);

    return 1.0f - std::abs(2.0f * t - 1.0f);
}

void Background::loadUpLaneSounds() {
    // GENERIC up-lane passing sound... SEDANS ONLY now (0-4). Gated
    // by score >= 10,000, see updateUpLaneTraffic().
    upLaneEngineBufferLoaded = upLaneEngineBuffer.loadFromFile("assets/UpLaneCarPass.wav");
    if (!upLaneEngineBufferLoaded) {
        std::cerr << "Warning: could not load assets/UpLaneCarPass.wav -- sedan up-lane engine (post-10000) will be silent.\n";
    }

    // EARLY sedan sound... plays from the start of the run until
    // score hits 10,000, then UpLaneCarPass.wav (above) takes over.
    upLaneEngineBufferEarlyLoaded = upLaneEngineBufferEarly.loadFromFile("assets/UpLaneCarPassEarly.wav");
    if (!upLaneEngineBufferEarlyLoaded) std::cerr << "Warning: could not load assets/UpLaneCarPassEarly.wav\n";

    // GENERIC up-lane horn (sedans) -- 20% chance, ~70% across screen.
    upLaneHornBufferLoaded = upLaneHornBuffer.loadFromFile("assets/UpLaneCarHorn.wav");
    if (!upLaneHornBufferLoaded) std::cerr << "Warning: could not load assets/UpLaneCarHorn.wav\n";

    // TRUCK's own distinct engine sound.
    truckEngineBufferLoaded = truckEngineBuffer.loadFromFile("assets/TruckEngine.wav");
    if (!truckEngineBufferLoaded) std::cerr << "Warning: could not load assets/TruckEngine.wav\n";

    // TRUCK's own distinct horn -- 30% chance, ~30% across the screen.
    truckHornBufferLoaded = truckHornBuffer.loadFromFile("assets/TruckHorn.wav");
    if (!truckHornBufferLoaded) std::cerr << "Warning: could not load assets/TruckHorn.wav\n";

    // VAN's own distinct engine sound (separate from truck and sedans now).
    vanEngineBufferLoaded = vanEngineBuffer.loadFromFile("assets/VanEngine.wav");
    if (!vanEngineBufferLoaded) std::cerr << "Warning: could not load assets/VanEngine.wav\n";

    // VAN's own distinct horn.
    vanHornBufferLoaded = vanHornBuffer.loadFromFile("assets/VanHorn.wav");
    if (!vanHornBufferLoaded) std::cerr << "Warning: could not load assets/VanHorn.wav\n";
}

void Background::pauseUpLaneSounds() {
    for (auto& car : upLaneCars) {
        if (car.engineSound && car.engineSound->getStatus() == sf::Sound::Status::Playing) car.engineSound->pause();
        if (car.hornSound && car.hornSound->getStatus() == sf::Sound::Status::Playing) car.hornSound->pause();
    }
}

void Background::updateUpLaneTraffic(float deltaTime, float groundSpeed) {
    // Up-lane traffic (sedans included) doesn't start spawning at all
    // until score 500+ -- was active from score 0. Timer only starts
    // counting once unlocked, so it doesn't instantly fire the moment
    // it unlocks from having silently accumulated beforehand.
    if (currentScoreForAudioGate >= 200) {
        // - Spawn a new one on a timer -
        upLaneCarSpawnTimer += deltaTime;
        if (upLaneCarSpawnTimer >= upLaneCarSpawnInterval) {
            upLaneCarSpawnTimer = 0.0f;

            // - SCATTERED BURST SPAWNING -
        // Cars come in clusters of 1-2 (max 2 now -- 3 was too tight,
        // especially with truck/van being much longer than sedans).
        // Between-cluster gap is a real breather, never over 2s. The
        // WITHIN-cluster gap (if this car starts/continues a cluster)
        // gets computed further below, AFTER we know this car's
        // actual width -- a flat time gap was fine for sedans but let
        // long vehicles overlap the next spawn.
        bool midBurst = (upLaneBurstRemaining > 0);
        if (midBurst) {
            upLaneBurstRemaining--;
        } else {
            int roll = std::rand() % 100;
            int burstSize = (roll < 40) ? 1 : 2; // 40% solo, 60% pair -- max cluster is 2
            upLaneBurstRemaining = burstSize - 1;
        }

        // Truck doesn't start spawning until score hits 18,000 --
        // below that, only pick from Car/Jeep/Van (indices 0..29).
        bool bigVehiclesUnlocked = currentScoreForAudioGate >= 18000;
        int poolSize = bigVehiclesUnlocked ? NUM_UPLANE_VARIANTS : UPLANE_TRUCK_START; // Car+Jeep+Van always in the pool, Truck gated
        int variant = std::rand() % poolSize;
        if (variant == lastUpLaneVariant) {
            variant = (variant + 1) % poolSize; // never the same variant twice (or more) in a row
        }
        lastUpLaneVariant = variant;

        // Capture the CURRENT real position of whichever car is most
        // recently spawned (if any exist) BEFORE adding this new one
        // - used below as a hard, position-based safety floor so no
        // car can ever spawn overlapping/behind another, regardless
        // of what the time-interval math predicted. This is what
        // actually guarantees no overtaking, independent of any lead-
        // time or interval calculation quirks.
        bool hadPreviousCar = !upLaneCars.empty();
        float previousCarX = hadPreviousCar ? upLaneCars.back().body.getPosition().x : 0.0f;
        float previousCarWidth = hadPreviousCar ? upLaneCars.back().body.getGlobalBounds().size.x : 0.0f;
        int previousCarVariant = hadPreviousCar ? upLaneCars.back().variant : -1;

        upLaneCars.emplace_back(upLaneCarBodyTextures[variant], upLaneCarWheelTextures[variant], variant);
        UpLaneCarInstance& car = upLaneCars.back();
        const UpLaneWheelLayout& layout = upLaneWheelLayouts[variant];

        // Small per-instance jitter (+/-8%) so same-type cars aren't
        // all moving in perfect lockstep - reads as more alive. Kept
        // deliberately small: the 4 types are already a tight cluster
        // (1.5-1.7), and widening this too much would reopen the same
        // catch-up risk that clustering the base speeds was meant to
        // close.
        float jitter = 0.92f + (static_cast<float>(std::rand() % 17)) / 100.0f; // 0.92-1.08
        car.speedMultiplier = upLaneCarSpeedMultipliers[variant] * jitter;

        float bodyScale = upLaneCarTargetHeights[variant] / upLaneCarBodyTextures[variant].getSize().y;
        car.body.setScale({bodyScale, bodyScale});

        // NOW that we know this car's real scaled width, set the next
        // spawn interval: within a cluster, it's "this car's width /
        // its speed" plus a small buffer - guarantees the NEXT car
        // can't appear until this one has actually cleared, no matter
        // how long it is (fixes truck/van overlapping). Between
        // clusters, same 0.8-2.0s breather as before, capped at 2s.
        float thisCarWidth = upLaneCarBodyTextures[variant].getSize().x * bodyScale;
        float thisCarSpeed = groundSpeed * car.speedMultiplier;
        bool willContinueBurst = (upLaneBurstRemaining > 0);
        if (willContinueBurst) {
            // 3x the bare minimum safe clearance is the CLOSEST these
            // two can ever be - never tighter than that, however
            // random variation on top means it's not a fixed distance
            // every time, sometimes noticeably more than 3x.
            float minClearance = (thisCarWidth + 40.0f) * 3.0f;
            float extraRandom = static_cast<float>(std::rand() % 400); // 0-400px extra, on top of the 3x floor
            upLaneCarSpawnInterval = (minClearance + extraRandom) / thisCarSpeed;
        } else {
            float normalRoll = 0.9f + (static_cast<float>(std::rand() % 1600) / 1000.0f); // 0.9-2.5s
            // Safety floor: whatever vehicle just spawned (car, van,
            // or truck) must have enough time to actually clear
            // before the next cluster starts - this only kicks in if
            // the normal random roll happened to be shorter than that,
            // which shouldn't be common now that all speeds match, but
            // guards against a wide vehicle (truck/van) getting a too-
            // short gap by pure bad luck on the random roll.
            float minClearance = thisCarWidth + 40.0f;
            float minClearanceTime = minClearance / thisCarSpeed;
            upLaneCarSpawnInterval = std::max(normalRoll, minClearanceTime);
        }

        // Van/Truck spawn extra travel-time further right than the
        // visible edge - their engine sound starts the instant the
        // object is created (below), so by the time they actually
        // scroll into view, the sound's already been playing that long.
        bool isBigVehicle = (variant >= UPLANE_VAN_START); // Van or Truck (both ranges are contiguous at the end)
        bool isTruckVariant = (variant >= UPLANE_TRUCK_START);
        float actualCarSpeed = groundSpeed * car.speedMultiplier;
        float leadSeconds = isTruckVariant ? 1.2f : (isBigVehicle ? 0.5f : 0.0f); // van 0.5s, truck 1.2s
        float spawnX = isBigVehicle ? (VIEW_RIGHT_EDGE + actualCarSpeed * leadSeconds) : VIEW_RIGHT_EDGE;

        // HARD SAFETY CLAMP: whatever the natural spawn point above
        // works out to, it can never be at or behind the previous
        // car's current position - this is what makes "no overtake"
        // an actual guarantee instead of something the interval math
        // merely predicts. This is the real fix for the truck-related
        // overlap bug: a big lead-time (like truck's 1.2s) used to be
        // able to silently eat into the intended clearance; now it
        // physically can't, no matter what the interval math assumed.
        if (hadPreviousCar) {
            // Flat 60px is fine for sedan-to-sedan gaps, but a truck is
            // so much bigger than the rest that the same 60px reads as
            // a near-miss next to one - widen it specifically when a
            // truck is on EITHER side of this gap (the one just ahead,
            // or the one about to spawn), leave everything else at the
            // original 60px so normal car/jeep/van pacing is untouched.
            bool previousWasTruck = previousCarVariant >= UPLANE_TRUCK_START;
            float actualSafetyBuffer = (previousWasTruck || isTruckVariant) ? 140.0f : 60.0f;
            float minSpawnX = previousCarX + previousCarWidth + actualSafetyBuffer;
            if (spawnX < minSpawnX) spawnX = minSpawnX;
        }

        car.body.setPosition({spawnX, upLaneCarGroundYs[variant]});

        sf::FloatRect bodyBounds = car.body.getGlobalBounds();
        float wheelDiameter = bodyBounds.size.x * layout.wheelDiameterFrac;
        float wheelScale = wheelDiameter / upLaneCarWheelTextures[variant].getSize().x;
        car.frontWheel.setScale({wheelScale, wheelScale});
        car.rearWheel.setScale({wheelScale, wheelScale});

        sf::Vector2u wheelTexSize = upLaneCarWheelTextures[variant].getSize();
        car.frontWheel.setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});
        car.rearWheel.setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});

        car.frontWheel.setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.frontXFrac + layout.frontOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.frontYFrac + layout.frontOffsetY
        });
        car.rearWheel.setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.rearXFrac + layout.rearOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.rearYFrac + layout.rearOffsetY
        });

        // Shadow: same approach as the down-lane vehicles: full body
        // width (+5% overhang), height kept independent/fixed so wide
        // vehicles (van/truck).
        if (upLaneShadeLoaded) {
            car.shadow.emplace(upLaneShadeTexture);
            sf::Vector2u shadeTexSize = upLaneShadeTexture.getSize();
            if (shadeTexSize.x > 0) {
                float shadowWidth = bodyBounds.size.x * 1.05f;
                float shadowScaleX = shadowWidth / shadeTexSize.x;
                const float upLaneShadowHeightScale = 0.35f; // ADJUST to make it thinner/thicker
                const float upLaneShadowYOffset = 2.0f; // ADJUST to nudge up(-)/down(+) from the body's bottom edge
                car.shadow->setScale({shadowScaleX, upLaneShadowHeightScale});
                car.shadow->setOrigin({shadeTexSize.x / 2.0f, shadeTexSize.y / 2.0f});
                car.shadow->setPosition({
                    bodyBounds.position.x + bodyBounds.size.x * 0.5f,
                    bodyBounds.position.y + bodyBounds.size.y + upLaneShadowYOffset + 8
                });
            }
        }

        // Engine sound: Truck and Van each get their own distinct
        // buffer; Car and Jeep share the one generic passing-engine
        // buffer (same "isSedan" bucket as before this extension).
        bool isTruck = (variant >= UPLANE_TRUCK_START);
        bool isVan = (variant >= UPLANE_VAN_START && variant < UPLANE_TRUCK_START);
        bool sedanUnlocked = currentScoreForAudioGate >= SEDAN_AUDIO_UNLOCK_SCORE;

        sf::SoundBuffer& engineBuf = isTruck ? truckEngineBuffer
            : isVan ? vanEngineBuffer
            : sedanUnlocked ? upLaneEngineBuffer
            : upLaneEngineBufferEarly;
        bool engineBufLoaded = isTruck ? truckEngineBufferLoaded
            : isVan ? vanEngineBufferLoaded
            : sedanUnlocked ? upLaneEngineBufferLoaded
            : upLaneEngineBufferEarlyLoaded;

        if (engineBufLoaded) {
            car.engineSound.emplace(engineBuf);
            car.engineSound->setLooping(false);
            car.engineSound->setVolume(0.0f); // real volume set every frame below
            car.engineSound->play();
        }
    }
    } // closes the score >= 500 gate wrapping the whole spawn-trigger block

    // Move + spin every active instance, same old-style plain
    // leftward scroll (oncoming direction), not the closing-speed math
    // the real interactive cars use. Speed is looked up from each
    // car's OWN jittered speedMultiplier now, set once at spawn -
    // not the shared type value - so it keeps whatever jitter it
    // spawned with for its whole lifetime instead of drifting if the
    // type's base value changes mid-run.
    for (auto& car : upLaneCars) {
        float upLaneSpeed = groundSpeed * car.speedMultiplier;
        float moveAmount = -upLaneSpeed * deltaTime;

        car.body.move({moveAmount, 0.0f});
        car.frontWheel.move({moveAmount, 0.0f});
        car.rearWheel.move({moveAmount, 0.0f});
        if (car.shadow.has_value()) car.shadow->move({moveAmount, 0.0f});

        // Rotation is driven by the SIGNED movement (moveAmount), not
        // just speed magnitude, so the spin direction always matches
        // which way the car is actually traveling - up-lane traffic
        // moves left (moveAmount negative), so this correctly spins
        // wheels the opposite way from the real rightward-moving cars.
        car.wheelRotationDeg += upLaneWheelRotationFactor * moveAmount;
        if (car.wheelRotationDeg > 360.0f) car.wheelRotationDeg -= 360.0f;
        if (car.wheelRotationDeg < -360.0f) car.wheelRotationDeg += 360.0f;
        car.frontWheel.setRotation(sf::degrees(car.wheelRotationDeg));
        car.rearWheel.setRotation(sf::degrees(car.wheelRotationDeg));

        bool isTruck = (car.variant >= UPLANE_TRUCK_START);
        bool isVan = (car.variant >= UPLANE_VAN_START && car.variant < UPLANE_TRUCK_START);
        bool isSedan = !isTruck && !isVan; // Car or Jeep mirror -- same generic engine/horn bucket as before

        // Fraction of the trip from spawn (VIEW_RIGHT_EDGE) to the
        // left edge - used for the horn trigger only.
        float traveled = VIEW_RIGHT_EDGE - car.body.getPosition().x;
        float totalTrip = VIEW_RIGHT_EDGE - VIEW_LEFT_EDGE;
        float fraction = (totalTrip > 0.0f) ? (traveled / totalTrip) : 0.0f;

        // ENGINE VOLUME - flat, no code-side curve (fade shape
        // lives in the .wav file itself now). Sedan sound is still
        // gated to silence until score >= 10,000, then plays LOUD
        // (1.4x boost) once unlocked. Slow-motion volume multiplier
        // layered on top of all of this, same as every other gameplay
        // sound in Game.cpp.
        if (car.engineSound) {
            float baseVolume = isTruck ? truckEngineVolume : (isVan ? vanEngineVolume : upLaneEngineVolume);

            if (isSedan) {
                // Which SOUND plays (early vs late) was already decided
                // at spawn time based on score - no more silence gate,
                // just play whichever one this car got, boosted a bit
                // once past the unlock score for extra impact.
                bool unlocked = currentScoreForAudioGate >= SEDAN_AUDIO_UNLOCK_SCORE;
                car.engineSound->setVolume((unlocked ? baseVolume * 1.4f : baseVolume) * slowMoAudioVolumeMult);
            } else {
                car.engineSound->setVolume(baseVolume * slowMoAudioVolumeMult);
            }

            // Truck: flat pitch, 20% faster/higher than normal (fixed
            // 1.2x) - its own distinct sound, no speed-scaling.
            // Sedans + van: pitch scales with how fast the world is
            // moving, same as before. Slow-motion pitch multiplies
            // into whichever of these was already chosen.
            if (isTruck) {
                car.engineSound->setPitch(1.2f * slowMoAudioPitch);
            } else {
                float pitch = groundSpeed / GROUND_SPEED;
                if (pitch < 0.7f) pitch = 0.7f;
                if (pitch > 1.6f) pitch = 1.6f;
                car.engineSound->setPitch(pitch * slowMoAudioPitch);
            }
        }

        // HORN TRIGGER: fires once per car. Truck: 30% chance at
        // ~30% across the trip. Van: same 20%/70% rule as sedans for
        // now (own buffer, same timing). Sedan horn only rolls once
        // score >= 10,000 (matches the engine gate).
        if (!car.hornRolled) {
            float triggerFraction = isTruck ? 0.30f : 0.70f;
            int chancePercent = isTruck ? 30 : 20;
            bool sedanGateOpen = !isSedan || (currentScoreForAudioGate >= SEDAN_AUDIO_UNLOCK_SCORE);

            if (fraction >= triggerFraction && sedanGateOpen) {
                car.hornRolled = true;
                if ((std::rand() % 100) < chancePercent) {
                    sf::SoundBuffer& hornBuf = isTruck ? truckHornBuffer : (isVan ? vanHornBuffer : upLaneHornBuffer);
                    bool hornBufLoaded = isTruck ? truckHornBufferLoaded : (isVan ? vanHornBufferLoaded : upLaneHornBufferLoaded);
                    if (hornBufLoaded) {
                        car.hornSound.emplace(hornBuf);
                        car.hornSound->setVolume((isTruck ? truckHornVolume : (isVan ? vanHornVolume : upLaneHornVolume)) * slowMoAudioVolumeMult);
                        car.hornSound->setPitch(slowMoAudioPitch);
                        car.hornSound->play();
                    }
                }
            }
        }
    }

    // Remove only once off-screen AND the engine sound has
    // actually finished playing on its own -- so it isn't cut off the
    // instant the car scrolls past the edge.
    upLaneCars.erase(
        std::remove_if(upLaneCars.begin(), upLaneCars.end(),
            [](const UpLaneCarInstance& car) {
                sf::FloatRect b = car.body.getGlobalBounds();
                bool offScreen = b.position.x + b.size.x < VIEW_LEFT_EDGE;
                if (!offScreen) return false;
                if (car.engineSound && car.engineSound->getStatus() == sf::Sound::Status::Playing) return false;
                return true;
            }),
        upLaneCars.end()
    );
}

void Background::update(float deltaTime, float groundSpeed, bool isPlaying) {
    // Compute background's own independent, slower speed -
    // Base speed at survival-time-zero, plus a DAMPENED share of
    // however much groundSpeed has grown above its starting constant
    // so far this run. This is what makes the background scroll
    // slower overall AND speed up more gradually than gameplay itself.
    float speedGrownSoFar = groundSpeed - GROUND_SPEED; // 0 at run start, grows over time
    float backgroundSpeed = (GROUND_SPEED * backgroundSpeedMultiplier) + (speedGrownSoFar * backgroundGrowthDampening);
    currentBackgroundSpeed = backgroundSpeed; // cached in case it's useful later -- obstacles sync to tiles again now

    // Sky: very slow, derived from the background's own
    // (already slower) speed, so the parallax ordering stays
    // sky < far buildings < background.
    float effectiveSkySpeed = backgroundSpeed * skySpeedMultiplier;
    float skySpacing = skyScaledWidth - skyOverlapAmount;
    for (auto& s : skySprites) {
        s.move({ -effectiveSkySpeed * deltaTime, 0.0f });
    }
    float skyRightmostX = skySprites[0].getPosition().x;
    for (auto& s : skySprites) {
        if (s.getPosition().x > skyRightmostX) skyRightmostX = s.getPosition().x;
    }
    for (auto& s : skySprites) {
        if (s.getPosition().x + skyScaledWidth < VIEW_LEFT_EDGE) {
            s.setTexture(skyTextures[skyNextTextureIndex], true);
            s.setPosition({ skyRightmostX + skySpacing, skyY });
            skyRightmostX += skySpacing;
            skyNextTextureIndex = (skyNextTextureIndex + 1) % NUM_SKY_IMAGES;
        }
    }

    // Far buildings: barely faster than sky, sits in front of it,
    // behind the main background city.
    float effectiveFarBuildingSpeed = backgroundSpeed * farBuildingSpeedMultiplier;
    float farBuildingSpacing = farBuildingScaledWidth - farBuildingOverlapAmount;
    for (auto& s : farBuildingSprites) {
        s.move({ -effectiveFarBuildingSpeed * deltaTime, 0.0f });
    }
    float farBuildingRightmostX = farBuildingSprites[0].getPosition().x;
    for (auto& s : farBuildingSprites) {
        if (s.getPosition().x > farBuildingRightmostX) farBuildingRightmostX = s.getPosition().x;
    }
    for (auto& s : farBuildingSprites) {
        if (s.getPosition().x + farBuildingScaledWidth < VIEW_LEFT_EDGE) {
            s.setTexture(farBuildingTextures[farBuildingNextTextureIndex], true);
            s.setPosition({ farBuildingRightmostX + farBuildingSpacing, farBuildingY });
            farBuildingRightmostX += farBuildingSpacing;
            farBuildingNextTextureIndex = (farBuildingNextTextureIndex + 1) % NUM_FARBUILDING_IMAGES;
        }
    }

    // - Mid buildings: faster than far, slower than close city. -
    float effectiveMidBuildingSpeed = backgroundSpeed * midBuildingSpeedMultiplier;
    float midBuildingSpacing = midBuildingScaledWidth - midBuildingOverlapAmount;
    for (auto& s : midBuildingSprites) {
        s.move({ -effectiveMidBuildingSpeed * deltaTime, 0.0f });
    }
    float midBuildingRightmostX = midBuildingSprites[0].getPosition().x;
    for (auto& s : midBuildingSprites) {
        if (s.getPosition().x > midBuildingRightmostX) midBuildingRightmostX = s.getPosition().x;
    }
    for (auto& s : midBuildingSprites) {
        if (s.getPosition().x + midBuildingScaledWidth < VIEW_LEFT_EDGE) {
            s.setTexture(midBuildingTextures[midBuildingNextTextureIndex], true);
            s.setPosition({ midBuildingRightmostX + midBuildingSpacing, midBuildingY });
            midBuildingRightmostX += midBuildingSpacing;
            midBuildingNextTextureIndex = (midBuildingNextTextureIndex + 1) % NUM_MIDBUILDING_IMAGES;
        }
    }

    // - Background city: uses its own computed backgroundSpeed -
    float bgSpacing = scaledWidth - backgroundOverlapAmount;
    for (auto& s : bgSprites) {
        s.move({ -backgroundSpeed * deltaTime, 0.0f });
    }
    float rightmostX = bgSprites[0].getPosition().x;
    for (auto& s : bgSprites) {
        if (s.getPosition().x > rightmostX) rightmostX = s.getPosition().x;
    }
    for (size_t i = 0; i < bgSprites.size(); i++) {
        if (bgSprites[i].getPosition().x + scaledWidth < VIEW_LEFT_EDGE) {
            bgSprites[i].setTexture(bgTextures[nextTextureIndex], true);
            bgSprites[i].setPosition({ rightmostX + bgSpacing, backgroundY });
            bgSpriteTexIndex[i] = nextTextureIndex;
            rightmostX += bgSpacing;
            nextTextureIndex = (nextTextureIndex + 1) % NUM_BG_IMAGES;
        }
    }

    // FarBuildings visibility masking over the water background has
    // been removed per request... it now shows over every background,
    // including the water one, just like the rest of the layers.

    // Tiles: slightly faster than groundSpeed (the SAME speed
    // value obstacles use, since obstacles sit on the tiles and must
    // stay in lockstep with them, not with the slower background).
    float effectiveTileSpeed = groundSpeed * tileSpeedMultiplier;
    float tileSpacing = tileScaledWidth - tileOverlapAmount;
    for (auto& s : tileSprites) {
        s.move({ -effectiveTileSpeed * deltaTime, 0.0f });
    }
    float tileRightmostX = tileSprites[0].getPosition().x;
    for (auto& s : tileSprites) {
        if (s.getPosition().x > tileRightmostX) tileRightmostX = s.getPosition().x;
    }
    for (auto& s : tileSprites) {
        if (s.getPosition().x + tileScaledWidth < VIEW_LEFT_EDGE) {
            s.setTexture(tileTextures[tileNextTextureIndex], true);
            s.setPosition({ tileRightmostX + tileSpacing, s.getPosition().y });
            tileRightmostX += tileSpacing;
            tileNextTextureIndex = (tileNextTextureIndex + 1) % NUM_TILE_IMAGES;
        }
    }

    updateHelicopter(deltaTime, groundSpeed, isPlaying);
    updateUpLaneTraffic(deltaTime, groundSpeed);
}

void Background::reset() {
    float skySpacing = skyScaledWidth - skyOverlapAmount;
    for (int i = 0; i < 2; i++) {
        skySprites[i].setTexture(skyTextures[i], true);
        skySprites[i].setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * skySpacing, skyY });
    }
    skyNextTextureIndex = 2 % NUM_SKY_IMAGES;

    float farBuildingSpacing = farBuildingScaledWidth - farBuildingOverlapAmount;
    for (int i = 0; i < 2; i++) {
        farBuildingSprites[i].setTexture(farBuildingTextures[i], true);
        farBuildingSprites[i].setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * farBuildingSpacing, farBuildingY });
    }
    farBuildingNextTextureIndex = 2 % NUM_FARBUILDING_IMAGES;

    float midBuildingSpacing = midBuildingScaledWidth - midBuildingOverlapAmount;
    for (int i = 0; i < 3; i++) {
        midBuildingSprites[i].setTexture(midBuildingTextures[i], true);
        midBuildingSprites[i].setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * midBuildingSpacing, midBuildingY });
    }
    midBuildingNextTextureIndex = 3 % NUM_MIDBUILDING_IMAGES;

    float bgSpacing = scaledWidth - backgroundOverlapAmount;
    for (int i = 0; i < 3; i++) {
        bgSprites[i].setTexture(bgTextures[i], true);
        bgSprites[i].setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * bgSpacing, backgroundY });
        bgSpriteTexIndex[i] = i;
    }
    nextTextureIndex = 3 % NUM_BG_IMAGES;

    farBuildingsVisible = true; // Background1 (not water) is always first, so this is always correct on reset

    currentBackgroundSpeed = 0.0f;

    float tileSpacing = tileScaledWidth - tileOverlapAmount;
    for (size_t i = 0; i < tileSprites.size(); i++) {
        int texIndex = i % NUM_TILE_IMAGES;
        tileSprites[i].setTexture(tileTextures[texIndex], true);
        tileSprites[i].setPosition({ VIEW_LEFT_EDGE + static_cast<float>(i) * tileSpacing, tileGroundY });
    }
    tileNextTextureIndex = tileSprites.size() % NUM_TILE_IMAGES;

    helicopterActive = false;
    helicopterSpawnEvent = false;
    helicopterTimer = 0.0f;

    upLaneCars.clear();
    upLaneCarSpawnTimer = 0.0f;
}

void Background::draw(sf::RenderWindow& window) {
    // Draw order matters: sky first (furthest back), then far
    // buildings, then city, then tiles (closest/frontmost, obstacles
    // sit on these again).
    for (auto& s : skySprites) {
        window.draw(s);
    }

    if (farBuildingsVisible) {
        for (auto& s : farBuildingSprites) {
            window.draw(s);
        }
    }

    for (auto& s : midBuildingSprites) {
        window.draw(s);
    }

    for (auto& s : bgSprites) {
        window.draw(s);
    }

    for (auto& s : tileSprites) {
        window.draw(s);
    }

    // Up-lane traffic: decorative only, drawn AFTER tiles so it's not
    // covered by them...matches how the real interactive cars
    // (drawn separately by Game.cpp) always sit on top of everything.
    for (auto& car : upLaneCars) {
        // ALL up-lane variants now draw wheels on top of the body...
        // same fix as truck/van and downlane cars before them.
        // Wheels-underneath relied on each body art having a real
        // transparent wheel-well cutout lined up exactly right, which
        // wasn't holding up reliably. Drawing on top guarantees the
        // wheel is always visible regardless of the art.
        if (car.shadow.has_value()) window.draw(*car.shadow);
        window.draw(car.body);
        window.draw(car.frontWheel);
        window.draw(car.rearWheel);
    }

    if (helicopterActive) {
        window.draw(*heliBodySprite);
    }
}
