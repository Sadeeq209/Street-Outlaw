#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <vector>

class Background {
public:
    Background();
    void update(float deltaTime, float groundSpeed, bool isPlaying);
    void draw(sf::RenderWindow& window);
    void reset();

    bool consumeHelicopterSpawnEvent();
    bool isHelicopterActive() const { return helicopterActive; }
    float getHelicopterVolumeFactor() const;

    // Lets Game.cpp read the EXACT speed obstacles should move at --
    // tiles are back, so obstacles sync to THEM again (not the
    // background), since tiles are the layer obstacles visually sit on.
    float getTileSpeedMultiplier() const { return tileSpeedMultiplier; }

private:
    // ==========================================================
    // LAYER 1 (drawn first, furthest back): SKY -- 2 images,
    // Sky1.png / Sky2.png, moves VERY slowly. Everything else sits on
    // top of this; any transparent parts of the Background city
    // images let this show through.
    // ==========================================================
    static const int NUM_SKY_IMAGES = 2;
    std::vector<sf::Texture> skyTextures;
    std::vector<sf::Sprite> skySprites;
    float skyScaledWidth;
    float skyY; // vertical position, adjustable -- same pattern as backgroundY/farBuildingY
    int skyNextTextureIndex;
    float skySpeedMultiplier; // fraction of the BACKGROUND's own effective speed (see below) -- keep tiny

    // SKY OVERLAP: same seam-gap fix as background/tiles used to have --
    // pulls each recycled sky image in by this many pixels.
    float skyOverlapAmount;

    // ==========================================================
    // LAYER 1.5 (drawn after sky, before the city background): FAR
    // BUILDINGS -- 2 images, FarBuildings1.png / FarBuildings2.png,
    // sits between sky and the main background city for extra
    // parallax depth. Moves slightly faster than sky (barely
    // noticeable) via farBuildingSpeedMultiplier -- same "fraction of
    // background's own speed" pattern as sky.
    // ==========================================================
    static const int NUM_FARBUILDING_IMAGES = 2;
    std::vector<sf::Texture> farBuildingTextures;
    std::vector<sf::Sprite> farBuildingSprites;
    float farBuildingScaledWidth;
    int farBuildingNextTextureIndex;
    float farBuildingSpeedMultiplier; // fraction of background's effective speed -- keep small, just above skySpeedMultiplier
    float farBuildingY; // vertical position -- adjust to sit these where you want them
    float farBuildingOverlapAmount; // same seam-gap fix as sky/background

    // ==========================================================
    // LAYER 1.75 (drawn after FarBuildings, before city Background):
    // MID BUILDINGS -- 3 images, MidBuildings1-3.png, sits between
    // FarBuildings and the close city buildings for a 3rd depth step.
    // ==========================================================
    static const int NUM_MIDBUILDING_IMAGES = 3;
    std::vector<sf::Texture> midBuildingTextures;
    std::vector<sf::Sprite> midBuildingSprites;
    float midBuildingScaledWidth;
    int midBuildingNextTextureIndex;
    float midBuildingSpeedMultiplier;
    float midBuildingY;
    float midBuildingOverlapAmount;

    // ==========================================================
    // LAYER 2: BACKGROUND CITY -- now has its OWN independent speed
    // curve, separate from the "world" speed (GROUND_SPEED growing
    // over time in GameConfig.h/Game.cpp) that obstacles/tire
    // use. This lets the background scroll slower and grow speed
    // more gradually than actual gameplay, for a nicer parallax feel.
    // ==========================================================
    float scaledWidth;
    int nextTextureIndex;

    // BACKGROUND Y: vertical position, applied to all 15 images
    // uniformly. Negative = raised up, positive = pushed down. 0.0f =
    // original position (top of sprite at the very top of the window).
    float backgroundY;

    // backgroundSpeedMultiplier: background's BASE speed = GROUND_SPEED
    // (the constant, not the live growing value) * this number. Lower
    // = slower background scroll. Adjust freely.
    float backgroundSpeedMultiplier;

    // backgroundGrowthDampening: how much of the WORLD's speed growth
    // (currentGroundSpeed - GROUND_SPEED, i.e. how much speed has
    // grown so far this run) carries over to the background's own
    // speed. 1.0 = grows exactly as fast as the world. 0.6 = grows at
    // 60% the rate (i.e. 40% slower growth) -- current default, per
    // your request. Lower this further for even slower background
    // speed-up over time.
    float backgroundGrowthDampening;

    // NUM_BG_IMAGES: how many actually CYCLE/show in gameplay --
    // now 50, all connected. NUM_BG_IMAGES_TOTAL: how many get
    // LOADED from disk -- also 50, matching. Loading logic reads
    // "assets/Background" + (i+1) + ".png" for i in
    // [0, NUM_BG_IMAGES_TOTAL), so all 50 files (Background1.png
    // through Background50.png) must exist in assets/.
    static const int NUM_BG_IMAGES = 50;
    static const int NUM_BG_IMAGES_TOTAL = 50;
    std::vector<sf::Texture> bgTextures;
    std::vector<sf::Sprite> bgSprites;

    // Which texture index (0..NUM_BG_IMAGES-1) each active bgSprites
    // entry currently shows -- same order/size as bgSprites. Needed
    // so we can tell when the water background (index 13,
    // Background14.png) is the one actually on screen right now.
    std::vector<int> bgSpriteTexIndex;
    static const int WATER_BACKGROUND_INDEX = 12; // Background13.png (13th image, 0-indexed)

    // FarBuildings hides while the water background is on screen (no
    // buildings floating over open water) -- updated once per frame
    // in update(), read by draw().
    bool farBuildingsVisible;

    // BACKGROUND OVERLAP: how many pixels each background image is
    // pulled IN from its neighbor before the next one is placed/
    // recycled -- e.g. if each image is 100 wide and this is 1, the
    // next one starts at 99, not 100, so it's already lapping slightly
    // before the previous one fully ends. Prevents the thin gap line
    // that can appear at the join. Increase if a gap ever reappears,
    // decrease if it looks bunched up. Keep this SMALL -- just enough
    // to kill the seam.
    float backgroundOverlapAmount;

    // Cached result of this frame's update() -- the background city
    // layer's actual current speed (base + dampened growth). No longer
    // read by obstacles (tiles handle that again), kept in case it's
    // useful later.
    float currentBackgroundSpeed;

    // ==========================================================
    // LAYER 3 (drawn after background city, frontmost/closest before
    // helicopter): TILES -- back again. 3 images, Tile1-3.png, fixed
    // cycling order. Obstacles ride THIS layer's speed (via
    // getTileSpeedMultiplier()) since they visually sit on it.
    // ==========================================================
    static const int NUM_TILE_IMAGES = 3;
    std::vector<sf::Texture> tileTextures;
    std::vector<sf::Sprite> tileSprites;
    float tileScaledWidth; // actual rendered width, read from the sprite's real bounds
    int tileNextTextureIndex;

    // TILE SIZE -- width and height independently adjustable, so you
    // can stretch/shrink one without affecting the other.
    float tileTargetWidth;
    float tileTargetHeight;

    // TILE GROUND LEVEL -- vertical position. Bump this if the new
    // background art needs tiles sitting somewhere different than
    // before.
    float tileGroundY;

    // TILE OVERLAP -- same seam-gap fix as sky/background/farBuildings.
    float tileOverlapAmount;

    // TILE SPEED -- multiplier of the LIVE ground speed (not a fixed
    // number), same as before. This is what obstacles sync to.
    float tileSpeedMultiplier;

    // ==========================================================
    // UP-LANE TRAFFIC -- purely decorative background cars, driving
    // ==========================================================
    // UP-LANE TRAFFIC -- oncoming/decorative vehicles, moving in
    // the OLD direction (left, oncoming), NO collision at all.
    //
    // Car/Jeep/Van here are MIRRORS of the real downlane Obstacle
    // variants -- own body art (facing the opposite way), but the
    // WHEEL texture and wheel-placement fractions are the downlane
    // ones, X-flipped (mirroredX = 1 - downlaneX, Y/size unchanged).
    // Truck has no downlane counterpart -- it's uplane-only, its own
    // standalone numbers.
    //
    // Single flat array (0..NUM_UPLANE_VARIANTS-1), laid out as 4
    // contiguous blocks in this order: Car, Jeep, Van, Truck. Use the
    // *_START/*_COUNT constants below instead of raw indices anywhere
    // you need to check "is this variant a van/truck/etc" -- keeps
    // everything correct if any of these counts change again later.
    // ==========================================================
    struct UpLaneWheelLayout {
        float frontXFrac, frontYFrac;
        float rearXFrac, rearYFrac;
        float wheelDiameterFrac;
        float frontOffsetX = 0.0f, frontOffsetY = 0.0f;
        float rearOffsetX = 0.0f, rearOffsetY = 0.0f;
    };
    static const int UPLANE_CAR_COUNT = 40; // was 15 -- now mirrors ALL 40 downlane cars, see Background.cpp's loadTextures() for the computed-mirror loop
    static const int UPLANE_JEEP_COUNT = 10;
    static const int UPLANE_VAN_COUNT = 5;
    static const int UPLANE_TRUCK_COUNT = 2;
    static const int UPLANE_CAR_START = 0;
    static const int UPLANE_JEEP_START = UPLANE_CAR_START + UPLANE_CAR_COUNT;    // 15
    static const int UPLANE_VAN_START = UPLANE_JEEP_START + UPLANE_JEEP_COUNT;   // 25
    static const int UPLANE_TRUCK_START = UPLANE_VAN_START + UPLANE_VAN_COUNT;   // 30
    static const int NUM_UPLANE_VARIANTS = UPLANE_TRUCK_START + UPLANE_TRUCK_COUNT; // 32
    std::vector<sf::Texture> upLaneCarBodyTextures;
    std::vector<sf::Texture> upLaneCarWheelTextures;
    std::vector<UpLaneWheelLayout> upLaneWheelLayouts;

    struct UpLaneCarInstance {
        sf::Sprite body;
        sf::Sprite frontWheel;
        sf::Sprite rearWheel;
        float wheelRotationDeg;
        int variant;
        // This car's ACTUAL speed multiplier -- the type's base value
        // (upLaneCarSpeedMultipliers[variant]) with a small random
        // jitter applied once at spawn, so same-type cars aren't all
        // moving in identical lockstep. Read every frame instead of
        // looking the type's base value up fresh, so this car keeps
        // its own jittered speed for its whole lifetime.
        float speedMultiplier = 1.0f;
        // Own looping engine sound (truck gets its own buffer, every
        // other variant shares the one generic "passing" buffer) --
        // own sf::Sound instance per car so multiple can overlap.
        std::optional<sf::Sound> engineSound;
        // Fires once per car if the horn's % chance passes, near the
        // left edge -- see updateUpLaneTraffic().
        std::optional<sf::Sound> hornSound;
        bool hornRolled = false;
        // Shadow -- same shared upLaneShadeTexture for every up-lane
        // vehicle (see Background class members below), set up once at
        // spawn, moved every frame alongside body/wheels, drawn first
        // (underneath) in Background::draw().
        std::optional<sf::Sprite> shadow;
        UpLaneCarInstance(sf::Texture& bodyTex, sf::Texture& wheelTex, int variantIndex)
            : body(bodyTex), frontWheel(wheelTex), rearWheel(wheelTex), wheelRotationDeg(0.0f), variant(variantIndex) {}
    };
    std::vector<UpLaneCarInstance> upLaneCars;

    // Shared shadow texture for ALL up-lane vehicles -- same Shade.png
    // the real (down-lane) cars use, loaded here as Background's own
    // copy since it's a separate class from Obstacle.
    sf::Texture upLaneShadeTexture;
    bool upLaneShadeLoaded = false;

    // ==========================================================
    // UP-LANE / TRUCK / VAN SOUNDS -- truck (variant 5) and van
    // (variant 6) each get their OWN dedicated engine + horn buffers
    // now. Every remaining variant (the 5 sedans, 0-4) shares one
    // generic "passing" engine buffer and one generic horn buffer.
    // Each car instance gets its own sf::Sound (see UpLaneCarInstance
    // above) so multiple cars can have engines/horns overlapping.
    // ==========================================================
    sf::SoundBuffer upLaneEngineBuffer;      // sedan sound AFTER score 10,000 (the "woosh" you're happy with)
    sf::SoundBuffer upLaneEngineBufferEarly; // sedan sound BEFORE score 10,000
    sf::SoundBuffer upLaneHornBuffer;    // generic horn, sedans only
    sf::SoundBuffer truckEngineBuffer;   // truck's own distinct engine sound
    sf::SoundBuffer truckHornBuffer;     // truck's own distinct horn
    sf::SoundBuffer vanEngineBuffer;     // van's own distinct engine sound
    sf::SoundBuffer vanHornBuffer;       // van's own distinct horn
    bool upLaneEngineBufferLoaded = false;
    bool upLaneEngineBufferEarlyLoaded = false;
    bool upLaneHornBufferLoaded = false;
    bool truckEngineBufferLoaded = false;
    bool truckHornBufferLoaded = false;
    bool vanEngineBufferLoaded = false;
    bool vanHornBufferLoaded = false;
    float upLaneEngineVolume = 70.0f; // 0-100
    float upLaneHornVolume = 70.0f;   // 0-100
    float truckEngineVolume = 80.0f;  // 0-100
    float truckHornVolume = 80.0f;    // 0-100
    float vanEngineVolume = 80.0f;    // 0-100
    float vanHornVolume = 80.0f;      // 0-100

    // SEDAN audio only unlocks once the player's score crosses 10,000
    // -- set every frame from Game.cpp via setScore(). Truck and van
    // are NOT gated by this, only the generic sedan sound is.
    int currentScoreForAudioGate = 0;
    static const int SEDAN_AUDIO_UNLOCK_SCORE = 28500; // was 10000 -- recalculated for the slower speed ramp so the "feel" at switch-time stays the same

    void loadUpLaneSounds();

public:
    // Same 0-100 pattern as every other sound volume in Game.cpp.
    // Applied live each frame to already-playing cars too.
    void setUpLaneEngineVolume(float v) { upLaneEngineVolume = v; }
    void setUpLaneHornVolume(float v) { upLaneHornVolume = v; }
    void setTruckEngineVolume(float v) { truckEngineVolume = v; }
    void setTruckHornVolume(float v) { truckHornVolume = v; }
    void setVanEngineVolume(float v) { vanEngineVolume = v; }
    void setVanHornVolume(float v) { vanHornVolume = v; }

    // Call every frame with the current score -- gates the sedan
    // (generic) up-lane sound so it only plays once score >= 10,000.
    void setScore(int score) { currentScoreForAudioGate = score; }

    // Called once per frame from Game.cpp with the current slow-motion
    // pitch/volume-multiplier, so every up-lane car's engine+horn
    // plays back slower/quieter exactly in sync with the world --
    // same idea as Obstacle::setSlowMoAudio() for the downlane cars.
    // pitch: mirrors worldTimeScale directly. volumeMultiplier: 1.0
    // normal, down to 0.5 at max slow-motion.
    void setSlowMoAudio(float pitch, float volumeMultiplier) {
        slowMoAudioPitch = pitch;
        slowMoAudioVolumeMult = volumeMultiplier;
    }

    // Pauses every up-lane car's engine + horn (if playing) -- call at
    // game over, same idea as bgMusic.pause()/heliSound.pause().
    void pauseUpLaneSounds();
private:
    float slowMoAudioPitch = 1.0f;
    float slowMoAudioVolumeMult = 1.0f;

    // UP-LANE CAR SIZE, PER VARIANT -- own target height per car, since
    // Jeep/Van/Truck need to be bigger than the Car mirrors. Indexed
    // by the UPLANE_*_START/*_COUNT ranges above (Car/Jeep/Van/Truck).
    std::vector<float> upLaneCarTargetHeights;

    // UP-LANE CAR GROUND LEVEL, PER VARIANT -- own ground Y per car,
    // since Jeep/Van/Truck are scaled taller and a shared ground Y
    // makes them sit visually higher (closer to the down-lane) than
    // the Car mirrors. Same indexing as upLaneCarTargetHeights/
    // upLaneCarSpeedMultipliers.
    std::vector<float> upLaneCarGroundYs;

    // UP-LANE CAR SPEED, PER VARIANT -- own speed multiplier per car
    // (fraction of live ground speed), since Jeep/Van/Truck will likely
    // want to move slower than the Car mirrors.
    std::vector<float> upLaneCarSpeedMultipliers;

    // UP-LANE CAR SPAWN TIMING -- how often a new one appears.
    float upLaneCarSpawnTimer;
    float upLaneCarSpawnInterval;

    // BURST SPAWNING -- instead of one steady rhythm, cars come in
    // scattered clusters of 1-3, tightly spaced within a cluster, with
    // a real (but capped) breather between clusters. See
    // updateUpLaneTraffic() in the .cpp for the actual logic.
    int upLaneBurstRemaining = 0; // how many MORE cars left in the current cluster (0 = between clusters)
    int lastUpLaneVariant = -1; // avoids the same variant spawning twice (or more) in a row

    // UP-LANE WHEEL SPIN -- same idea as the real cars' rotation
    // factor, degrees of spin per pixel traveled.
    float upLaneWheelRotationFactor;

    void updateUpLaneTraffic(float deltaTime, float groundSpeed);

    // ---- Helicopter (single body sprite, no rotors) ----
    // 4 body variants, picked randomly each time a new helicopter
    // spawns (25% chance each) -- see updateHelicopter(). All 4 are
    // close enough in size to share one scale/width, computed once
    // from the first texture.
    static const int NUM_HELI_VARIANTS = 4;
    std::vector<sf::Texture> heliBodyTextures;
    std::optional<sf::Sprite> heliBodySprite;

    bool helicopterActive;
    bool helicopterSpawnEvent;
    float helicopterTimer;
    float helicopterSpawnInterval;
    float helicopterSpeedMultiplier;
    float helicopterSpawnX;
    float helicopterY;
    float helicopterBodyWidth;

    void updateHelicopter(float deltaTime, float groundSpeed, bool isPlaying);
};

#endif