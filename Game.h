#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <deque>
#include <optional>
#include <vector>
#include "Background.h"
#include "Player.h"
#include "Obstacle.h"
#include "ScoreManager.h"

enum class GameState {
    Menu,     // title screen, waiting for Play (click or Enter)
    Intro,    // scripted warehouse chase cutscene, plays once before the first Playing state
    Playing,  // normal gameplay
    Paused,   // gameplay frozen, showing Resume/Quit
    Dying,    // world frozen, death animation playing until landed
    GameOver  // death finished, showing G-Over graphic + score, waiting for restart
};

// Ambient corner-diagonal particle for the menu background -- spawns
// at screen center, travels toward one of the 4 diagonal corners,
// growing in scale as it goes, then resets. See the particle block in
// Game.h/Game.cpp for spawn/update/draw logic.
struct MenuParticle {
    sf::Sprite sprite;
    float dirX, dirY;  // unit vector toward this particle's assigned corner
    float distanceTraveled; // pixels moved so far this cycle, from center
    float speed;       // pixels/sec along dirX/dirY
    bool isDot;         // true = dot texture (faster, grows to 200%), false = cluster (grows to 100%)
};

class Game {
public:
    Game(sf::RenderWindow& window);
    void run();

private:
    sf::RenderWindow& window;

    // ==========================================================
    // ZOOM-OUT VIEW (preview version) -- shows MORE of the world
    // compressed into the same 800x600 window, shrinking everything
    // (player, obstacles, background) uniformly so obstacles read as
    // smaller/further away when they first appear, with more visible
    // time before they reach the player. Width and height are
    // adjustable INDEPENDENTLY -- raise viewWidth alone for a wider
    // view (same height), raise viewHeight alone for a taller view
    // (same width), or both together for a uniform zoom-out.
    // Starting at 800/600 (exactly the window size, i.e. no zoom) so
    // nothing changes until you raise these.
    //
    // PREVIEW LIMITATION: this applies to EVERYTHING right now,
    // including the score/pause UI -- those will also shrink/shift
    // with the zoom. The full version will separate the world view
    // from the UI so the HUD stays crisp and fixed-size regardless of
    // zoom -- this preview is just for testing how the zoom itself
    // feels before doing that extra work.
    sf::View gameView;
    float viewWidth;
    float viewHeight;
    sf::Clock clock;

    GameState state;

    Background background;
    Player player;
    ScoreManager scoreManager;

    std::deque<Obstacle> obstacles;
    float spawnTimer;
    float spawnInterval;

    // ==========================================================
    // SCRIPTED CAR-JUMP SCENE -- two variants sharing the same state
    // machine: the full Car -> Jeep -> DownlaneVan chain, or just
    // Car -> Jeep. Each vehicle taller than the last (176 -> 281 ->
    // 386, a 105px step each time, safely under the 120px max jump
    // height). While either variant is running, normal random
    // obstacle spawning is completely paused -- resumes automatically
    // once the sequence finishes.
    //
    // Call triggerCarJumpScene3() or triggerCarJumpScene2() to start
    // one (currently nothing calls these automatically -- hook up
    // whatever trigger condition you want: score milestone, random
    // chance, etc. Both do nothing if a scene is already running.
    // ==========================================================
    // ==========================================================
    // SCRIPTED CAR-JUMP SCENE -- fires as an ALTERNATE to a normal
    // single-obstacle spawn (15% chance, only once score >= 5000),
    // instead of a separate timed event. Randomly picks the 3-vehicle
    // chain (Car->Jeep->Van) or the 2-vehicle one (Car->Jeep) each
    // time it triggers. No clearing-path delay anymore -- the first
    // vehicle spawns immediately, same moment a normal spawn would
    // have happened. A 10s cooldown after any scene finishes blocks
    // the next one from rolling until it expires.
    // ==========================================================
    // WaitingClear: added so the scene waits for its LAST vehicle's
    // own width (+ a fresh random gap) to clear before handing control
    // back to normal spawning -- mirrors the width+gap wait already
    // used between every other hand-off in the chain (Car->Jeep,
    // Jeep->Van), just applied once more after the final vehicle.
    // Without this, the very first normal car after a scene reused a
    // leftover gap value that was sized for normal Car-to-Car spacing,
    // sized/calculated before the scene even started -- letting it
    // spawn too close behind the much wider Jeep/Van.
    enum class CarSceneState { Inactive, WaitingCar, WaitingJeep, WaitingVan, WaitingClear };
    CarSceneState carSceneState;
    float carSceneTimer;
    bool carSceneIncludesVan; // true = 3-vehicle chain, false = Car->Jeep only
    float carSceneCooldownTimer; // counts down after a scene finishes; must reach 0 before another can trigger
    // Spawn X shared by every vehicle in one scene run -- computed
    // once via computeSpawnX() when the scene is triggered, then
    // reused for the Jeep/Van spawns instead of each recomputing (or
    // worse, hardcoding 900.0f) independently. Keeps every vehicle in
    // the chain anchored to the exact same reference point normal
    // spawning uses, at whatever speed the run is currently at.
    float carSceneSpawnX;
    void triggerCarJumpScene3(); // Car -> Jeep -> Van
    void triggerCarJumpScene2(); // Car -> Jeep only
    void updateCarJumpScene(float deltaTime, float obstacleSpeed);

    // Scaled spawn X used by BOTH normal obstacles (spawnObstacle())
    // and the scripted scene (triggerCarJumpScene2/3, and the Jeep/Van
    // spawns inside updateCarJumpScene()). Having every spawn path
    // share this one function is what keeps the gap math consistent
    // at the scene's entry/exit boundaries -- previously the scene
    // spawned at a flat 900.0f while normal cars scaled with
    // currentGroundSpeed, so the intended gap silently shrank or grew
    // depending on how far the run had sped up.
    float computeSpawnX() const;

    // ==========================================================
    // CAR-JUMP SCENE ADJUSTMENT KNOBS -- Jeep and Van ONLY (Car in
    // the scene keeps its own existing height behavior, untouched).
    // Does NOT affect normal random spawning at all -- spawnObstacle()/
    // the 15% trigger roll are separate and unchanged.
    //
    // Size only now, independent per vehicle: sceneJeepHeight /
    // sceneVanHeight -- ground line is no longer scene-specific for
    // ANY vehicle in the chain (Car/Jeep/Van all use their own normal
    // per-variant ground line -- carGroundYs/jeepGroundYs/vanGroundYs
    // in Obstacle.cpp -- in the scene too, same as random spawns).
    //
    // Spacing between vehicles in the chain is no longer a per-hand-
    // off pixel-gap constant -- every hand-off (Car->Jeep, Jeep->Van,
    // and the final exit) now waits on the same real position-check,
    // isDownlaneClearFor(sceneClearScreenFraction), so there's one
    // shared knob for how tight the whole chain feels, not several
    // separate ones that could drift out of sync with each other.
    // ==========================================================
    float sceneCarHeight = 98.0f;
    float sceneJeepHeight = 138.0f;
    float sceneVanHeight = 160.0f;

    sf::Font font;
    bool fontLoaded;

    // Second font, only for symbol glyphs (like the pause icon) that
    // font.ttf doesn't include -- Noto Sans Symbols 2 covers ⏸/▶/⏹
    // and similar technical symbols.
    sf::Font symbolFont;
    bool symbolFontLoaded;

    // ---- Speed scaling over time ----
    // currentGroundSpeed grows the longer you survive, passed into
    // Background/Obstacle each frame instead of them reading a fixed
    // constant. Obstacle spawn distance also scales with it (see
    // spawnObstacle()) so reaction time stays roughly fair as speed
    // increases.
    float currentGroundSpeed;
    float speedSurvivalTime;

    // ---- Background music ----
    sf::Music bgMusic;
    bool musicLoaded;

    // ---- Sound effects ----
    // Each sound has its own SoundBuffer + optional<Sound> (SFML's
    // sf::Sound has no default constructor, so it's only emplaced once
    // its buffer loads successfully) and its own adjustable volume
    // (0-100 scale) -- tune each independently in the constructor.
    sf::SoundBuffer hoverBuffer, clickBuffer, jumpBuffer, landingBuffer,
                     slideBuffer, rollBuffer, hitBuffer, milestoneBuffer,
                     heliBuffer, bodyfallBuffer, slowMoBuffer;
    std::optional<sf::Sound> hoverSound, clickSound, jumpSound, landingSound,
                              slideSound, rollSound, hitSound, milestoneSound,
                              heliSound, bodyfallSound, slowMoSound;
    bool hoverLoaded, clickLoaded, jumpLoaded, landingLoaded,
         slideLoaded, rollLoaded, hitLoaded, milestoneLoaded,
         heliLoaded, bodyfallLoaded, slowMoLoaded;

    // Per-obstacle death hit sounds -- Tire/Box/Drone get their own
    // distinct impact sound instead of the generic hitSound above.
    // Everything else (Car, Jeep, Van, ThrowingCar, etc.)
    // still falls back to hitSound, unchanged. Picked by killer->getType()
    // in triggerDeath().
    sf::SoundBuffer tireHitBuffer, boxHitBuffer, droneHitBuffer;
    std::optional<sf::Sound> tireHitSound, boxHitSound, droneHitSound;
    bool tireHitLoaded = false, boxHitLoaded = false, droneHitLoaded = false;

    // Volume range the helicopter sound fades between as it flies by
    // (quiet at the edges, loudest when overhead) -- see
    // Background::getHelicopterVolumeFactor().
    float heliMinVolume;
    float heliMaxVolume;

    // ---- Footstep sounds -- fired from Player's one-shot footstep
    // events (see consumeFootstepLeftEvent/RightEvent in Player.h),
    // one pair for bare ground and one pair for standing on a
    // vehicle (Car/Van/Jeep), picked via player.isStandingOnVehicle().
    sf::SoundBuffer groundFootstepLeftBuffer, groundFootstepRightBuffer,
                     carFootstepLeftBuffer, carFootstepRightBuffer;
    std::optional<sf::Sound> groundFootstepLeftSound, groundFootstepRightSound,
                              carFootstepLeftSound, carFootstepRightSound;
    bool groundFootstepLeftLoaded = false, groundFootstepRightLoaded = false,
         carFootstepLeftLoaded = false, carFootstepRightLoaded = false;
    float groundFootstepVolume = 80.0f;
    float carFootstepVolume = 100.0f;

    // ---- Drone sound -- owned here (not per-Obstacle-instance)
    // so it can start before the drone obstacle exists and keep
    // playing briefly after it's gone. Plain, non-positional
    // playback -- no listener/3D audio. See the drone sound block
    // in update() for the full lifecycle:
    //   1. Starts ~1s before the drone is scheduled to spawn.
    //   2. First 2 seconds of playback at 70% volume, then 100%.
    //   3. Cut ~0.5s after the drone leaves the screen.
    sf::SoundBuffer droneBuffer;
    std::optional<sf::Sound> droneSound;
    bool droneLoaded = false;
    float droneSoundVolume = 100.0f; // base/max volume (0-100)
    bool droneSoundActive = false;   // true from the pre-spawn cue through the cutoff
    bool droneHasAppeared = false;   // true once the real drone obstacle has been seen on screen
    float droneOffscreenTimer = 0.0f; // seconds since last confirmed on-screen (or since it vanished)
    float droneSoundElapsed = 0.0f;   // seconds since this playback started, for the volume envelope
    float droneSpawnCountdown = -1.0f; // -1 = not counting down; else counts down from 1.0 to the actual spawn

    // ---- TIRE SPAWN -- exact same choreography as Drone above
    // (separate timer -> pending -> headroom guard -> countdown ->
    // spawn), just a ground vehicle instead of a flying one. Reuses
    // the shared isDownlaneClearFor() helper at its own fraction.
    float tireSpawnTimer = 0.0f;
    float tireSpawnInterval = 20.0f;
    bool tireSpawnPending = false;
    float tireSpawnCountdown = -1.0f;
    float tireClearScreenFraction = 0.60f;
    float tirePostSpawnCarBlockTimer = 0.0f;
    float tirePostSpawnCarBlockSeconds = 1.1f; // was 2.0 -- cars resume even sooner after a tire appears

    // True from the moment a Tire actually spawns until it's fully
    // left the screen and been removed (see the pop_front() block in
    // update()) -- same "covers the whole on-screen lifetime" shape as
    // droneSoundActive. Drone/Box now check THIS instead of the old
    // tireSpawnCountdown < 0.0f check, which only covered tire's brief
    // ~1s pre-spawn countdown and let drone/box arm during a tire's
    // entire time actually rolling across the screen.
    bool tireActive = false;
    void spawnTire();

    // ==========================================================
    // VAN FLIP SLOW-MOTION -- world (background/obstacles) + player
    // both slow down together, triggered from Player::jump() (50%
    // chance, front/back flip off a Van only). Player.cpp only fires
    // a one-shot event (consumeVanFlipSlowMoEvent()); this whole
    // timeline lives here since it needs to scale deltaTime for
    // several different systems, not just the player.
    //
    // Timeline (all real, unscaled seconds -- see updateSlowMo()):
    //   PreDelay  0.0 -> 0.2s   : nothing yet, world still normal speed
    //   RampDown  0.2 -> 0.5s   : eases from 1.0x down to slowMoMinScale
    //   Hold      0.5 -> 1.5s   : held at slowMoMinScale
    //   RampUp    1.5 -> 2.0s   : eases back from slowMoMinScale to 1.0x
    // worldTimeScale is what actually gets multiplied into deltaTime
    // before it's handed to background/obstacles/player each frame.
    // ==========================================================
    enum class SlowMoPhase { Inactive, PreDelay, RampDown, Hold, RampUp };
    SlowMoPhase slowMoPhase = SlowMoPhase::Inactive;
    float slowMoTimer = 0.0f;       // elapsed time within the CURRENT phase only
    float worldTimeScale = 1.0f;    // 1.0 = normal speed; multiplies deltaTime for world+player systems
    float slowMoPitch = 1.0f;       // mirrors worldTimeScale -- sounds play back at the same rate the world is moving
    float slowMoVolumeMultiplier = 1.0f; // 1.0 normal, down to 0.5 at max slow-motion -- see updateSlowMo()
    const float slowMoPreDelay = 0.2f;
    const float slowMoRampDownDuration = 0.3f;
    const float slowMoHoldDuration = 1.5f;
    const float slowMoRampUpDuration = 0.8f;
    const float slowMoMinScale = 0.33f; // how slow "max slow-motion" actually is -- tune freely
    void updateSlowMo(float realDeltaTime); // advances the timeline using REAL (unscaled) time

    // ---- Milestone blink (score beats previous high score) ----
    bool milestoneBlinking;
    float milestoneBlinkTimer;
    float milestoneColorTimer;
    int milestoneColorIndex;
    std::vector<sf::Color> milestoneColors;

    // ---- Menu UI ----
    // New static-background menu: title/background is one full-screen
    // image now (no more live gameplay scene behind it), buttons are
    // Play/About/Quit images stacked vertically, About opens an empty
    // overlay panel (content TBD -- add later).
    std::optional<sf::Text> menuHighScoreText;
    std::optional<sf::Text> skipIntroText; // "Press Enter to skip >" -- bottom-right during the intro cutscene only
    std::optional<sf::Text> aboutTitleText;
    std::optional<sf::Text> aboutBodyText;
    sf::FloatRect playButtonRect;
    sf::FloatRect aboutButtonRect;
    sf::FloatRect quitButtonRect;
    bool playHovered;
    bool aboutHovered;
    bool quitHovered;
    bool showAboutOverlay; // true while the empty About panel is open

    // assets/MenuBackground.png -- full 800x600 static scene (title +
    // character art baked in), replaces the old live gameplay preview.
    sf::Texture menuBackgroundTexture;
    bool menuBackgroundLoaded;
    std::optional<sf::Sprite> menuBackgroundSprite;

    // assets/MenuPlay.png, MenuAbout.png, MenuQuit.png
    sf::Texture menuPlayTexture;
    bool menuPlayLoaded;
    std::optional<sf::Sprite> menuPlaySprite;

    sf::Texture menuAboutTexture;
    bool menuAboutLoaded;
    std::optional<sf::Sprite> menuAboutSprite;

    sf::Texture menuQuitTexture;
    bool menuQuitLoaded;
    std::optional<sf::Sprite> menuQuitSprite;

    // ==========================================================
    // MENU AMBIENT PARTICLES -- dots + clusters spawn at screen
    // center and travel toward one of the 4 diagonal corners,
    // growing in scale as they go. Each corner has its own fixed
    // texture (index-matched, no randomness): index 0=top-left,
    // 1=top-right, 2=bottom-left, 3=bottom-right, so e.g.
    // ParticleDot2.png/ParticleCluster2.png always travel top-right.
    // Additive-blended for a neon glow look. See
    // setupMenuParticles()/updateMenuParticles()/renderMenuParticles()
    // in Game.cpp.
    // ==========================================================
    std::vector<sf::Texture> particleDotTextures;     // index 0-3 = top-left, top-right, bottom-left, bottom-right
    std::vector<sf::Texture> particleClusterTextures; // index 0-3 = top-left, top-right, bottom-left, bottom-right
    bool menuParticlesLoaded;
    std::vector<MenuParticle> menuParticles;
    void setupMenuParticles();
    void updateMenuParticles(float deltaTime);
    void renderMenuParticles();

    // ==========================================================
    // MENU MUSIC -- 10 tracks, one plays at a time, starts 3s after
    // the menu first appears, and a new random track (never the same
    // one twice in a row, and not one of the last 2 played) picks up
    // automatically as soon as the current one finishes. sf::Music
    // streams from disk rather than loading fully into memory, which
    // is the right tool for full songs (vs sf::Sound/SoundBuffer,
    // used for short one-shot effects elsewhere in this file) -- one
    // shared sf::Music instance is reused and re-opened with a new
    // file for each track, since only one is ever playing at once.
    // ==========================================================
    static const int NUM_MENU_SONGS = 10;
    sf::Music menuMusic;
    bool menuMusicStarted;       // has the initial 5s delay elapsed + first track begun?
    float menuMusicStartTimer;   // counts UP from 0 while in Menu state, triggers at 5.0f
    bool menuMusicWaitingGap;    // true during the 3s silence between one song ending and the next starting
    float menuMusicGapTimer;     // counts UP from 0 while waiting, triggers at 3.0f
    int menuMusicLastPlayed;     // most recently played track index, -1 = none yet
    int menuMusicSecondLastPlayed; // the one before that, -1 = none yet
    void playRandomMenuSong();   // picks + plays a track, avoiding the last 2 played

    // ---- In-game score (Playing/Dying) ----
    std::optional<sf::Text> scoreText;

    // ---- Pause button + overlay ----
    sf::FloatRect pauseButtonRect;   // small icon, top-right, click to pause

    // "Pause" icon image -- assets/PauseIcon.png (neon-glow style,
    // matches Busted/PressEnterToRestart)
    sf::Texture pauseIconTexture;
    bool pauseIconLoaded;
    std::optional<sf::Sprite> pauseIconSprite;
    std::optional<sf::Text> pausedTitleText;
    std::optional<sf::Text> resumeText;
    std::optional<sf::Text> pausedQuitText;
    sf::FloatRect resumeButtonRect;
    sf::FloatRect pausedQuitButtonRect;
    bool resumeHovered;
    bool pausedQuitHovered;

    // ---- Game over screen ----
    std::optional<sf::Text> scoreLabelText;
    std::optional<sf::Text> scoreValueText;
    std::optional<sf::Text> highScoreLabelText;
    std::optional<sf::Text> highScoreValueText;
    float gameOverScoreLineY;
    float gameOverHighScoreLineY;
    sf::FloatRect gameOverQuitButtonRect;
    bool gameOverQuitHovered;

    // "QUIT" image -- assets/GameOverQuit.png (Game Over screen only;
    // the main menu's Quit button is a separate image, menuQuitTexture)
    sf::Texture gameOverQuitTexture;
    bool gameOverQuitLoaded;
    std::optional<sf::Sprite> gameOverQuitSprite;

    // "RETURN TO MENU" image -- assets/GameOverMenu.png. Stacked
    // between Restart and Quit; sends state back to GameState::Menu
    // (see click handling in the event loop, and the menu-music reset
    // that goes with it -- see notes there).
    sf::Texture gameOverMenuTexture;
    bool gameOverMenuLoaded;
    std::optional<sf::Sprite> gameOverMenuSprite;
    sf::FloatRect gameOverMenuButtonRect;
    bool gameOverMenuHovered;
    bool restartHovered;

    // "BUSTED" logo image -- assets/Busted.png
    sf::Texture bustedLogoTexture;
    bool bustedLogoLoaded;
    std::optional<sf::Sprite> bustedLogoSprite;

    // "Press Enter to Restart" image -- assets/PressEnterToRestart.png
    sf::Texture restartTexture;
    bool restartTextureLoaded;
    std::optional<sf::Sprite> restartSprite;
    sf::FloatRect restartButtonRect;

    void processEvents();
    void update(float deltaTime);
    void render();
    void spawnObstacle();
    void checkCollisions();

    // ==========================================================
    // DRONE SPAWNING -- fully separate timer/state from the normal
    // Car spawnTimer above, since it has its own extra conditions:
    // 1) fires on its own interval (droneSpawnInterval), 2) once that
    // interval elapses it goes "pending" and waits (doesn't just spawn // immediately) until isDownlaneClearForDrone() says the screen is
    // clear enough, 3) once it actually spawns, dronePostSpawnCarBlockTimer
    // is set to block normal Car spawning for a few seconds.
    //
    // ADJUSTMENT KNOBS:
    //   droneSpawnInterval        -- average seconds between drone
    //                                 spawn attempts (re-randomized
    //                                 after each spawn, see .cpp)
    //   droneClearScreenFraction  -- how much of the screen (from the
    //                                 spawn/right side) must be free of
    //                                 any Car/Jeep/Van before a pending
    //                                 drone is allowed to actually
    //                                 spawn. 0.95 = last 5% near the
    //                                 exit edge is the only place a
    //                                 vehicle's allowed to still be.
    //   dronePostSpawnCarBlockSeconds -- how long normal Car spawning
    //                                 pauses after a drone spawns.
    // ==========================================================
    float droneSpawnTimer = 0.0f;
    float droneSpawnInterval = 15.0f;
    bool droneSpawnPending = false;
    float dronePostSpawnCarBlockTimer = 0.0f;
    float droneClearScreenFraction = 0.60f; // was 0.90 -- lowered since Box/ThrowingCar made the downlane busier, giving drone a real chance to find a clear moment again
    // Same idea as droneClearScreenFraction, but for the car-jump
    // scene's exit-back-to-normal-spawning check -- slightly narrower
    // (less clearance required) than drone's, since the player is
    // expected to JUMP the just-passed Jeep/Van, not slide under it
    // the way drone requires room for -- there's naturally more slack.
    float sceneClearScreenFraction = 0.80f;
    float dronePostSpawnCarBlockSeconds = 1.5f; // was 2.0 -- cars resume even sooner after a drone appears, more challenging
    void spawnDrone();
    bool isDownlaneClearFor(float clearScreenFraction) const; // shared clearance check, drone + scene exit both use this
    bool isDownlaneClearForDrone() const;

    // ==========================================================
    // BOX THROW -- a Box (real projectile arc, aimed at a fixed
    // forehead-height point) paired with a ThrowingCar (spawns once
    // the box is boxThrowCarSpawnFraction through its ACTUAL flight --
    // see BOX_SPEED in GameConfig.h, shared with Obstacle.cpp's actual
    // physics so the two can't drift out of sync; flight time is now
    // distance/BOX_SPEED, computed per-throw, not a fixed constant).
    // Same "separate timer, goes pending, waits for clear screen"
    // shape as Drone above, reusing the same isDownlaneClearFor()
    // helper at its own 85% fraction.
    //
    // ADJUSTMENT KNOBS:
    //   boxThrowSpawnInterval     -- average seconds between box-throw
    //                                attempts (re-randomized after
    //                                each one, see .cpp)
    //   boxThrowClearScreenFraction -- 0.85 = last 15% of the screen
    //                                near the exit edge is the only
    //                                place a vehicle's allowed to
    //                                still be, before a pending throw
    //                                is allowed to actually spawn.
    //   boxThrowTargetForeheadY   -- the fixed Y the box's BOTTOM edge
    //                                is aimed at, computed once from
    //                                Player's own standing metrics
    //                                (groundY=672, standing height=125
    //                                in Player.cpp -- NOT re-measured
    //                                live, NOT tracking the player).
    //                                Nudge this directly if the box
    //                                doesn't look like it's catching
    //                                him at head height.
    //   boxSpawnEdgeMargin        -- how far PAST the visible screen
    //                                edge (VIEW_RIGHT_EDGE=900) the box
    //                                actually spawns, so it visibly
    //                                enters rather than popping into
    //                                view mid-air. Raise for more
    //                                off-screen travel before it's
    //                                visible, lower for it to appear
    //                                almost immediately.
    // ==========================================================
    float boxThrowSpawnTimer = 0.0f;
    float boxThrowSpawnInterval = 18.0;
    bool boxThrowSpawnPending = false;
    bool boxThrowActive = false; // true from the box's spawn until its paired ThrowingCar has been spawned
    float boxThrowElapsedTimer = 0.0f; // time since the currently-active throw's box spawned
    float boxThrowActualFlightDuration = 1.0f; // THIS throw's actual flight time (distance/BOX_SPEED) -- computed once in spawnBoxThrow(), same formula as Obstacle.cpp's boxFlightDuration so both stay in sync
    float boxThrowClearScreenFraction = 0.75f;
    float boxThrowTargetForeheadY = 562.0f;
    float boxLandTargetScreenPercent = 10.0f; // 0 = far left, 100 = far right -- where the box lands, as % of screen width
    float boxSpawnEdgeMargin = 70.0f;

    // How soon (as a fraction of THIS throw's actual flight duration --
    // see boxThrowActualFlightDuration above) after the box spawns the
    // ThrowingCar spawns. Was hardcoded at 0.5 (car waited until the
    // box was HALFWAY through its flight) -- way too slow per your
    // "car should already be starting to show from the beginning"
    // spec. 0.05 = car spawns almost immediately after the
    // box (a tiny beat later so the box still visibly leads), not
    // halfway through.
    float boxThrowCarSpawnFraction = 0.2f;

    // BUG FIX: without this, the moment boxThrowActive flips back to
    // false (right when the ThrowingCar spawns), a normal Car could
    // spawn in that same frame -- landing exactly on top of the
    // ThrowingCar that had just appeared. Same shape as
    // dronePostSpawnCarBlockTimer, own separate timer.
    float boxThrowPostSpawnCarBlockTimer = 0.0f;
    float boxThrowPostSpawnCarBlockSeconds = 2.0f; // was 3.0 -- cars resume sooner after a box throw
    void spawnBoxThrow();

    void triggerDeath(const Obstacle* killer = nullptr, bool hitFromFront = false); // shared by every death case -- see .cpp. killer = the specific obstacle whose collision caused this death, excluded from the backstop scan since it's always touching the player by definition. hitFromFront = true when the death line/zone that was touched is the vehicle's FRONT (nose) side -- knocks the player forward, away from the vehicle's body, instead of the default backward knock used for every other death (back hits, tire, drone, etc.)
    // Decides what surface the player's death fall should land on --
    // called once, at the top of triggerDeath(), BEFORE player.die().
    // Three cases, checked in order:
    //   1. Killed by directly hitting a Car/Jeep/DownlaneVan's own
    //      nose/back edge (not a landing) -- "catches" him on top of
    //      THAT vehicle instead of letting him fall through to the
    //      road, using its bounding-box top edge (not the walk-free
    //      segments -- segment matching is exactly what already failed
    //      to find a surface here, that's WHY it was a death and not a
    //      landing).
    //   2. Killed by something else (Drone/Tire/TCan) while
    //      his X position currently overlaps some OTHER Car/Jeep/Van's
    //      footprint -- catches him on that vehicle instead, same
    //      bounding-box-top approach.
    //   3. Neither applies -- falls all the way to the normal base
    //      road level, same as before this existed.
    float computeDeathLandingY(const Obstacle* killer) const;
    void updateLandableObstacles(); // handles Crate + Car (things you can jump/land on)
    void restartGame();
    void startGame();
    void returnToMenu(); // Game Over's "Return to Menu" button

    void renderMenu();
    void renderGameplay();
    void renderPaused();
    void renderGameOver();

    bool isMouseOverRect(sf::Vector2i mousePos, const sf::FloatRect& rect) const;

    // ==========================================================
    // INTRO CUTSCENE -- scripted warehouse chase, plays once when Play
    // is pressed from the Menu (NOT on restart-after-death -- that
    // still calls startGame()/restartGame() directly, skipping straight
    // to normal gameplay, same as before this existed).
    //
    // Self-contained: uses its OWN lightweight sprites/textures for
    // the player and agent, NOT the gameplay Player class -- it needs
    // things gameplay Player was never built for (off-screen
    // horizontal entry, a second running character, being driven
    // entirely by a script instead of input/physics-against-
    // obstacles). Reuses the SAME asset files though (Run1-8.png), so
    // no new player art is needed.
    //
    // 3 scripted phases (slide removed -- player spawns already
    // running; no front flip / no slow-mo either):
    //   TitleCard -- "The Breach" title art, lower-left corner, on
    //                black, exactly introTitleDuration seconds total.
    //                Fades IN first (both the black and the title
    //                itself ramp 0->255 together, eased not linear),
    //                the title sound fires once that fade-in crosses
    //                50%, then holds solid, then fades OUT at the end
    //                (same eased eases, title also drifts up slightly
    //                while fading) revealing Background1 underneath
    //                (already drawn the whole time) -- a proper
    //                crossfade, not a hard cut.
    //   Chase     -- entered the instant TitleCard ends. Player is
    //                already running from introPlayerSpawnX, burst
    //                sound fires the same frame. Once he crosses
    //                introHeyStopTriggerX (a POSITION, not a timer --
    //                see the fraction below), "Hey stop!" fires and
    //                background swaps to bg2. 1 second after THAT,
    //                the bg2/bg3 alarm flicker + beep loop starts.
    //                2 seconds after the FIRST beep, the agent resets
    //                to the true left edge and starts chasing (see
    //                introAgentSpawned). Flicker/chase continues
    //                until the garage-door line.
    //   ExitTouch -- triggered on reaching the garage-door line --
    //                both freeze, whip fires immediately, the white
    //                flash (fake radial blur -- SFML has no real
    //                shader-based radial blur without extra render-
    //                texture/GLSL work, so a plain expanding white
    //                overlay stands in) ramps up on its own timer
    //                (introFlashRiseDuration), bass drop fires
    //                partway through that ramp (introBassDropTrigger-
    //                Fraction), and once the ramp completes,
    //                startGame() runs (same as a normal Play press)
    //                and the SAME overlay fades back down over the
    //                first moments of real gameplay -- see
    //                introFlashFadingOut below.
    // ==========================================================
    enum class IntroPhase { TitleCard, Chase, ExitTouch };
    IntroPhase introPhase = IntroPhase::TitleCard;
    float introPhaseTimer = 0.0f; // elapsed time within the CURRENT phase only

    void startIntro();
    void updateIntro(float deltaTime);
    void renderIntro();

    // ---- Intro player (separate sprite/state from the real Player class -- see comment above) ----
    std::vector<sf::Texture> introPlayerRunTextures; // 8 frames -- assets/Run1.png-Run8.png (same files the real Player uses)
    bool introPlayerTexturesLoaded = false;
    std::optional<sf::Sprite> introPlayerSprite;
    sf::Vector2f introPlayerPos;
    int introPlayerFrame = 0;
    float introPlayerAnimTimer = 0.0f;
    float introPlayerSpawnX = -150.0f; // off-screen left -- Chase's start AND the edge the agent later spawns from too

    // ---- Agent -- only ever appears in this cutscene ----
    std::vector<sf::Texture> agentRunTextures; // 8 frames -- assets/AgentRun1.png-AgentRun8.png (rename to match your actual files)
    bool agentTexturesLoaded = false;
    std::optional<sf::Sprite> agentSprite;
    sf::Vector2f agentPos;
    int agentFrame = 0;
    float agentAnimTimer = 0.0f;
    bool introAgentSpawned = false; // false until 2 seconds after the first beep -- gates both drawing AND updating him

    // ---- 3 warehouse backgrounds -- assets/IntroBackground1-3.png (rename to match your actual files) ----
    sf::Texture introBgTexture1, introBgTexture2, introBgTexture3;
    bool introBgLoaded = false;
    std::optional<sf::Sprite> introBgSprite1, introBgSprite2, introBgSprite3;
    int introCurrentBg = 1; // 1, 2, or 3 -- which one renderIntro() draws this frame
    float introBgScaleMultiplier = 1.247f; // on top of the "fit 800px width" baseline -- your own eyeballed value
    float introBgPosX = -99.0f;  // your own eyeballed value
    float introBgPosY = -235.0f; // your own eyeballed value

    // ---- Title card -- assets/TitleBreach.png (rename to match your
    // actual file). Position is fraction-based (0-1 of the actual
    // window size), NOT tied to a specific resolution, so it stays put
    // if the window size ever changes. introTitleAnchorXFraction=0.09
    // means 9% in from the left; introTitleAnchorYFraction=0.775 means
    // 77.5% down from the top -- both directly adjustable. ----
    sf::Texture introTitleTexture;
    bool introTitleTexLoaded = false;
    std::optional<sf::Sprite> introTitleSprite;
    float introTitleScaleMultiplier = 0.25f;      // 1.0 = fits exactly the window's full width, same baseline the background uses -- 3x smaller than before
    float introTitleAnchorXFraction = 0.15f;     // 0 = left edge, 1 = right edge
    float introTitleAnchorYFraction = 0.775f;    // 0 = top edge, 1 = bottom edge
    float introTitleDuration = 4.0f;             // TOTAL time on screen, fade-in AND fade-out both included
    float introTitleFadeInDuration = 0.7f;       // first chunk of introTitleDuration -- black + title both fade IN together (eased)
    float introTitleFadeOutDuration = 0.6f;      // last chunk of introTitleDuration -- black + title both fade OUT together (eased), revealing Background1 underneath (already drawn the whole time)
    float introTitleSoundTriggerFraction = 0.5f; // title sound fires once the fade-IN crosses this fraction (0.5 = halfway), not at time 0
    float introTitleFadeOutRiseY = 20.0f;        // title drifts up this many px while fading out, for a slight "lifting away" cinematic feel
    bool introTitleSoundPlayed = false;          // one-shot guard for the above

    // ---- Chase timing -- position + real sound duration, per your spec ----
    float introHeyStopTriggerDistance = 20.0f; // "Hey stop!" fires once the player has run this many px PAST HIS SPAWN POINT (introPlayerSpawnX) -- distance-from-spawn, not a screen-fraction, so it's not silently inflated by however far off-screen he starts. Lower toward 0 for an almost-instant shout.
    bool introHeyStopTriggered = false;        // one-shot guard
    float introHeyStopDurationSeconds = 1.25f; // overwritten in init() with the REAL file duration via getDuration() -- 1.25 here is just a safe fallback if the file fails to load
    bool introBeepsStarted = false;            // one-shot guard -- flips true the instant the shout ACTUALLY finishes (duration-based, not a guessed delay)
    int introBeepCount = 0;                    // counts up each beep -- agent bursts out on the 2nd

    // ---- Flicker, Chase phase only (post-beeps-start) ----
    float introFlickerTimer = 0.0f;
    float introBeepOnDuration = 0.3f;  // no longer used for the flicker switch itself -- kept only in case you want it back later
    float introBeepGapSeconds = 0.35f;  // no longer used for the flicker switch itself -- kept only in case you want it back later
    float introFlickerBeatDuration = 0.3f; // THE flicker speed now -- one fixed beat for both on/off, same as the original front-flip-era version. Fast and punchy, but the beep sound CAN get cut off if it's longer than this.

    // ---- White flash (fake radial blur) -- deliberately NOT tied to
    // GameState::Intro, since it needs to keep fading during the first
    // moments of GameState::Playing too. Drawn unconditionally at the
    // top of render(), checked by alpha > 0 rather than by state.
    float introFlashAlpha = 0.0f;
    bool introFlashRisingUp = false;   // true = ExitTouch ramping 0->255
    bool introFlashFadingOut = false;  // true = post-handoff, ramping 255->0
    float introFlashTimer = 0.0f;      // elapsed time since the rise started -- this IS the flash timer
    float introFlashRiseDuration = 0.6f;  // seconds for the flash to go fully white -- raise for a slower flash, lower for a snappier one
    float introFlashFadeDuration = 0.85f; // seconds for the fade back down to normal gameplay after the handoff
    float introBassDropTriggerFraction = 0.75f; // bass drop fires when the rise is this fraction done (0.75 = 3/4 of the way to full white) -- NOT at the same moment as the whip
    bool introBassDropPlayed = false; // one-shot guard for the above

    // ---- Intro-specific tunable positions/speeds -- every number a
    // scripted scene like this needs eyeballing on, all in one place ----
    float introRunSpeed = 260.0f;          // px/s -- Chase, start to finish
    float introGroundY = 672.0f;           // matches the real Player's own groundY -- both player and agent run on this line
    float introGarageDoorX = 850.0f;       // ExitTouch trigger line -- line this up with your garage door art
    float introAgentSpawnX = -150.0f;      // off-screen left, same edge the player himself starts from
    float introAgentGapX = 160.0f;         // final settled distance behind the player once caught up
    float introAgentCatchUpSpeed = 340.0f; // px/s while still closing the gap -- faster than introRunSpeed so he visibly gains ground instead of matching 1:1
    float introAgentSizeMultiplier = 1.10f; // agent drawn slightly bigger than the player -- more presence as the chaser

    // ---- Post-intro agent cameo -- purely cosmetic, no collision.
    // Reuses the SAME agent sprite/textures as the intro cutscene
    // (agentPos/agentFrame/agentAnimTimer/agentSprite above) since the
    // two never run at the same time. Spawns at the start of every
    // Playing session -- after the intro AND after every restart --
    // sits running near the left edge (behind the player, who is
    // fixed at screen X=150) until postIntroAgentSlowdownStart
    // seconds, THEN eases into a deceleration (fast at first, visibly
    // slowing down as he goes, NOT a flat constant speed) so he reads
    // as tiring out/giving up the chase rather than just popping off
    // -- fully off-screen (past VIEW_LEFT_EDGE) by
    // postIntroAgentGoneBy seconds, then deactivated for good.
    bool postIntroAgentActive = false;
    float postIntroAgentTimer = 0.0f;
    float postIntroAgentSlowdownStart = 0.4f; // seconds spent just running in place before he starts fading back
    float postIntroAgentGoneBy = 5.0f;        // seconds (from spawn) by which he's fully off-screen and deactivated -- longer window now, for a more visibly gradual slowdown
    float postIntroAgentStartX = 0.5f;       // near the left edge, behind the player's fixed X=150 -- adjust freely
    float postIntroAgentSizeMultiplier = 0.90f; // 80% of the previous 1.15 intro size -- independent of the intro cutscene's own agent scale

    // Shadow beneath the post-intro agent -- same Shade.png the player
    // and vehicles use, loaded as Game's own copy since Player keeps
    // its shadeTexture private. Flat, full-size, full-opacity (unlike
    // the player's jump-height-based shrink/fade) since this agent
    // never leaves the ground.
    sf::Texture agentShadeTexture;
    bool agentShadeLoaded = false;
    std::optional<sf::Sprite> agentShadowSprite;

    // ---- Intro sounds -- placeholder filenames per your instruction, rename whenever ----
    sf::SoundBuffer introTitleSoundBuffer, introWhooshBuffer, introHeyStopBuffer, introBeepBeepBuffer, introWhipBuffer, introBassDropBuffer;
    std::optional<sf::Sound> introTitleSound, introWhooshSound, introHeyStopSound, introBeepBeepSound, introWhipSound, introBassDropSound;
    bool introTitleSoundLoaded = false, introWhooshLoaded = false, introHeyStopLoaded = false, introBeepBeepLoaded = false, introWhipLoaded = false, introBassDropLoaded = false;
};

#endif
