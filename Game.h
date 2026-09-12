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

// Ambient menu particle: spawns at screen center, travels toward one
// of the 4 diagonal corners, growing in scale, then resets.
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

    // Zoom-out view: shrinks player/obstacles/background uniformly so
    // obstacles appear smaller and further away. viewWidth/viewHeight
    // scale independently; 800/600 matches the window (no zoom).
    // Currently affects the whole scene including the score/pause UI;
    // a full version would separate the world view from a fixed UI.
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

    // Scripted car-jump scene: fires as an alternate to a normal
    // spawn (15% chance, score >= 5000). Randomly picks the 3-vehicle
    // chain (Car->Jeep->Van, heights 176/281/386) or the 2-vehicle one
    // (Car->Jeep). Pauses normal spawning while active; a 10s cooldown
    // after finishing blocks the next roll until it expires.
    // WaitingClear: waits for the last vehicle's width + gap to clear
    // before handing control back to normal spawning, same as the
    // width+gap wait used between every other hand-off in the chain.
    enum class CarSceneState { Inactive, WaitingCar, WaitingJeep, WaitingVan, WaitingClear };
    CarSceneState carSceneState;
    float carSceneTimer;
    bool carSceneIncludesVan; // true = 3-vehicle chain, false = Car->Jeep only
    float carSceneCooldownTimer; // counts down after a scene finishes; must reach 0 before another can trigger
    // Spawn X shared by every vehicle in one scene run: computed
    // once via computeSpawnX() when the scene is triggered, then
    // reused for the Jeep/Van spawns instead of each recomputing (or
    // worse, hardcoding 900.0f) independently. Keeps every vehicle in
    // the chain anchored to the exact same reference point normal
    // spawning uses, at whatever speed the run is currently at.
    float carSceneSpawnX;
    void triggerCarJumpScene3(); // Car -> Jeep -> Van
    void triggerCarJumpScene2(); // Car -> Jeep only
    void updateCarJumpScene(float deltaTime, float obstacleSpeed);

    // Spawn X used by both normal obstacles and the scripted scene,
    // scaled with currentGroundSpeed so the gap stays consistent
    // regardless of run speed.
    float computeSpawnX() const;

    // Car-jump scene sizing (Jeep/Van only; Car keeps its normal
    // height). Each vehicle uses its own per-variant ground line
    // (carGroundYs/jeepGroundYs/vanGroundYs in Obstacle.cpp). Spacing
    // between hand-offs (Car->Jeep, Jeep->Van, exit) is governed by
    // isDownlaneClearFor(sceneClearScreenFraction), not a fixed gap.
    float sceneCarHeight = 98.0f;
    float sceneJeepHeight = 138.0f;
    float sceneVanHeight = 160.0f;

    sf::Font font;
    bool fontLoaded;

    // Second font, only for symbol glyphs (like the pause icon) that
    // font.ttf doesn't include: Noto Sans Symbols 2 covers ⏸/▶/⏹
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
    // (0-100 scale): tune each independently in the constructor.
    sf::SoundBuffer hoverBuffer, clickBuffer, jumpBuffer, landingBuffer,
                     slideBuffer, rollBuffer, hitBuffer, milestoneBuffer,
                     heliBuffer, bodyfallBuffer, slowMoBuffer;
    std::optional<sf::Sound> hoverSound, clickSound, jumpSound, landingSound,
                              slideSound, rollSound, hitSound, milestoneSound,
                              heliSound, bodyfallSound, slowMoSound;
    bool hoverLoaded, clickLoaded, jumpLoaded, landingLoaded,
         slideLoaded, rollLoaded, hitLoaded, milestoneLoaded,
         heliLoaded, bodyfallLoaded, slowMoLoaded;

    // Per-obstacle death hit sounds: Tire/Box/Drone get their own
    // distinct impact sound instead of the generic hitSound above.
    // Everything else (Car, Jeep, Van, ThrowingCar, etc.)
    // still falls back to hitSound, unchanged. Picked by killer->getType()
    // in triggerDeath().
    sf::SoundBuffer tireHitBuffer, boxHitBuffer, droneHitBuffer;
    std::optional<sf::Sound> tireHitSound, boxHitSound, droneHitSound;
    bool tireHitLoaded = false, boxHitLoaded = false, droneHitLoaded = false;

    // Volume range the helicopter sound fades between as it flies by
    // (quiet at the edges, loudest when overhead): see
    // Background::getHelicopterVolumeFactor().
    float heliMinVolume;
    float heliMaxVolume;

    // ---- Footstep sounds: fired from Player's one-shot footstep
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

    // ---- Drone sound: owned here (not per-Obstacle-instance)
    // so it can start before the drone obstacle exists and keep
    // playing briefly after it's gone. Plain, non-positional
    // playback: no listener/3D audio. See the drone sound block
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

    // ---- TIRE SPAWN: exact same choreography as Drone above
    // (separate timer -> pending -> headroom guard -> countdown ->
    // spawn), just a ground vehicle instead of a flying one. Reuses
    // the shared isDownlaneClearFor() helper at its own fraction.
    float tireSpawnTimer = 0.0f;
    float tireSpawnInterval = 20.0f;
    bool tireSpawnPending = false;
    float tireSpawnCountdown = -1.0f;
    float tireClearScreenFraction = 0.60f;
    float tirePostSpawnCarBlockTimer = 0.0f;
    float tirePostSpawnCarBlockSeconds = 1.1f; // cars resume shortly after a tire appears

    // True from the moment a Tire actually spawns until it's fully
    // left the screen and removed (see the pop_front() block in
    // update()), covering its whole on-screen lifetime, same as
    // droneSoundActive. Drone/Box check this rather than
    // tireSpawnCountdown, which only covers the brief pre-spawn
    // countdown and not the tire's actual time on screen.
    bool tireActive = false;
    void spawnTire();

    // Van-flip slow-motion: world + player both slow down, triggered
    // from Player::jump() (50% chance, flip off a Van only). Lives
    // here rather than in Player since it scales deltaTime for
    // multiple systems. Timeline (real, unscaled seconds):
    //   PreDelay  0.0-0.2s : normal speed
    //   RampDown  0.2-0.5s : eases 1.0x -> slowMoMinScale
    //   Hold      0.5-1.5s : held at slowMoMinScale
    //   RampUp    1.5-2.0s : eases back to 1.0x
    // worldTimeScale multiplies deltaTime before it reaches
    // background/obstacles/player each frame.
    enum class SlowMoPhase { Inactive, PreDelay, RampDown, Hold, RampUp };
    SlowMoPhase slowMoPhase = SlowMoPhase::Inactive;
    float slowMoTimer = 0.0f;       // elapsed time within the CURRENT phase only
    float worldTimeScale = 1.0f;    // 1.0 = normal speed; multiplies deltaTime for world+player systems
    float slowMoPitch = 1.0f;       // mirrors worldTimeScale: sounds play back at the same rate the world is moving
    float slowMoVolumeMultiplier = 1.0f; // 1.0 normal, down to 0.5 at max slow-motion: see updateSlowMo()
    const float slowMoPreDelay = 0.2f;
    const float slowMoRampDownDuration = 0.3f;
    const float slowMoHoldDuration = 1.5f;
    const float slowMoRampUpDuration = 0.8f;
    const float slowMoMinScale = 0.33f; // speed multiplier at max slow-motion
    void updateSlowMo(float realDeltaTime); // advances the timeline using REAL (unscaled) time

    // ---- Milestone blink (score beats previous high score) ----
    bool milestoneBlinking;
    float milestoneBlinkTimer;
    float milestoneColorTimer;
    int milestoneColorIndex;
    std::vector<sf::Color> milestoneColors;

    // ---- Menu UI ----
    // Static background image with title baked in, Play/About/Quit
    // buttons stacked vertically, About opens an overlay panel.
    std::optional<sf::Text> menuHighScoreText;
    std::optional<sf::Text> skipIntroText; // "Press Enter to skip >": bottom-right during the intro cutscene only
    std::optional<sf::Text> aboutTitleText;
    std::optional<sf::Text> aboutBodyText;
    sf::FloatRect playButtonRect;
    sf::FloatRect aboutButtonRect;
    sf::FloatRect quitButtonRect;
    bool playHovered;
    bool aboutHovered;
    bool quitHovered;
    bool showAboutOverlay; // true while the empty About panel is open

    // assets/MenuBackground.png: full 800x600 static scene (title +
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

    // Menu ambient particles: dots/clusters spawn at screen center and
    // travel toward one of 4 diagonal corners, growing in scale.
    // Texture index is fixed per corner (0=TL, 1=TR, 2=BL, 3=BR).
    // Additive-blended for a glow look.
    std::vector<sf::Texture> particleDotTextures;     // index 0-3 = top-left, top-right, bottom-left, bottom-right
    std::vector<sf::Texture> particleClusterTextures; // index 0-3 = top-left, top-right, bottom-left, bottom-right
    bool menuParticlesLoaded;
    std::vector<MenuParticle> menuParticles;
    void setupMenuParticles();
    void updateMenuParticles(float deltaTime);
    void renderMenuParticles();

    // Menu music: 10 tracks, one at a time, starts 3s after the menu
    // appears; picks a new random track (not the last 2 played) when
    // the current one ends. Uses sf::Music (streamed) rather than
    // sf::Sound since these are full songs, not one-shot effects; one
    // shared instance is reopened per track.
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

    // "Pause" icon image: assets/PauseIcon.png (neon-glow style,
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

    // "QUIT" image: assets/GameOverQuit.png (Game Over screen only;
    // the main menu's Quit button is a separate image, menuQuitTexture)
    sf::Texture gameOverQuitTexture;
    bool gameOverQuitLoaded;
    std::optional<sf::Sprite> gameOverQuitSprite;

    // "RETURN TO MENU" image: assets/GameOverMenu.png. Stacked
    // between Restart and Quit; sends state back to GameState::Menu
    // (see click handling in the event loop, and the menu-music reset
    // that goes with it: see notes there).
    sf::Texture gameOverMenuTexture;
    bool gameOverMenuLoaded;
    std::optional<sf::Sprite> gameOverMenuSprite;
    sf::FloatRect gameOverMenuButtonRect;
    bool gameOverMenuHovered;
    bool restartHovered;

    // "BUSTED" logo image: assets/Busted.png
    sf::Texture bustedLogoTexture;
    bool bustedLogoLoaded;
    std::optional<sf::Sprite> bustedLogoSprite;

    // "Press Enter to Restart" image: assets/PressEnterToRestart.png
    sf::Texture restartTexture;
    bool restartTextureLoaded;
    std::optional<sf::Sprite> restartSprite;
    sf::FloatRect restartButtonRect;

    void processEvents();
    void update(float deltaTime);
    void render();
    void spawnObstacle();
    void checkCollisions();

    // Drone spawning: separate timer/state from normal Car spawning.
    // Fires on droneSpawnInterval, then goes "pending" and waits for
    // isDownlaneClearForDrone() before actually spawning, then blocks
    // normal Car spawning for dronePostSpawnCarBlockSeconds.
    // droneClearScreenFraction: fraction of the screen from the spawn
    // side that must be free of vehicles before a pending drone spawns.
    float droneSpawnTimer = 0.0f;
    float droneSpawnInterval = 15.0f;
    bool droneSpawnPending = false;
    float dronePostSpawnCarBlockTimer = 0.0f;
    float droneClearScreenFraction = 0.60f;
    // Same as droneClearScreenFraction but for the car-jump scene's
    // exit check; narrower since the player jumps the Jeep/Van rather
    // than needing room to slide under it.
    float sceneClearScreenFraction = 0.80f;
    float dronePostSpawnCarBlockSeconds = 1.5f;
    void spawnDrone();
    bool isDownlaneClearFor(float clearScreenFraction) const; // shared clearance check, drone + scene exit both use this
    bool isDownlaneClearForDrone() const;

    // Box throw: a Box (projectile arc aimed at a fixed forehead-height
    // point) paired with a ThrowingCar that spawns once the box is
    // boxThrowCarSpawnFraction through its flight (distance/BOX_SPEED,
    // matching Obstacle.cpp's physics). Same timer/pending/clear-screen
    // shape as Drone, using isDownlaneClearFor() at its own fraction.
    //   boxThrowClearScreenFraction: fraction of the screen (from the
    //     exit edge) that must be clear before a pending throw spawns.
    //   boxThrowTargetForeheadY: fixed Y for the box's bottom edge,
    //     derived once from Player's groundY/standing height, not
    //     tracked live.
    //   boxSpawnEdgeMargin: distance past VIEW_RIGHT_EDGE the box
    //     spawns, so it visibly enters rather than popping into view.
    float boxThrowSpawnTimer = 0.0f;
    float boxThrowSpawnInterval = 18.0;
    bool boxThrowSpawnPending = false;
    bool boxThrowActive = false; // true from the box's spawn until its paired ThrowingCar has been spawned
    float boxThrowElapsedTimer = 0.0f; // time since the currently-active throw's box spawned
    float boxThrowActualFlightDuration = 1.0f; // THIS throw's actual flight time (distance/BOX_SPEED): computed once in spawnBoxThrow(), same formula as Obstacle.cpp's boxFlightDuration so both stay in sync
    float boxThrowClearScreenFraction = 0.75f;
    float boxThrowTargetForeheadY = 562.0f;
    float boxLandTargetScreenPercent = 10.0f; // 0 = far left, 100 = far right: where the box lands, as % of screen width
    float boxSpawnEdgeMargin = 70.0f;

    // Fraction of the box's flight duration after which the
    // ThrowingCar spawns (a small beat after the box, not halfway).
    float boxThrowCarSpawnFraction = 0.2f;

    // Blocks normal Car spawning for a moment after boxThrowActive
    // clears, so a Car can't spawn on top of the just-appeared
    // ThrowingCar. Same shape as dronePostSpawnCarBlockTimer.
    float boxThrowPostSpawnCarBlockTimer = 0.0f;
    float boxThrowPostSpawnCarBlockSeconds = 2.0f;
    void spawnBoxThrow();

    // Shared by every death case. killer = the obstacle whose collision
    // caused the death, excluded from the backstop scan since it's
    // always touching the player. hitFromFront = true knocks the
    // player forward (nose-side hit) instead of the default backward
    // knock used for other deaths.
    void triggerDeath(const Obstacle* killer = nullptr, bool hitFromFront = false);
    // Picks the surface the player's death fall lands on, called
    // before player.die(). Checked in order: (1) killed by directly
    // hitting a vehicle's nose/back edge: lands on that vehicle's
    // bounding-box top; (2) killed by something else while overlapping
    // another vehicle's footprint: lands on that vehicle; (3) neither
    //: falls to the base road level.
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

    // Intro cutscene: scripted warehouse chase, plays once when Play
    // is pressed from the Menu (not on restart-after-death). Uses its
    // own lightweight sprites for player and agent rather than the
    // gameplay Player class, since it needs off-screen entry, a second
    // running character, and full script control instead of physics.
    // Reuses the same Run1-8.png asset files.
    //
    // 3 phases:
    //   TitleCard: title art fades in over black, sound fires at
    //                50% fade-in, holds, then crossfades out to
    //                reveal Background1.
    //   Chase    : player runs from introPlayerSpawnX. Crossing
    //                introHeyStopTriggerX fires "Hey stop!" and swaps
    //                to bg2. 1s later the bg2/bg3 alarm flicker+beep
    //                loop starts; 2s after the first beep the agent
    //                spawns and starts chasing, until the garage door.
    //   ExitTouch: both freeze, whip sound fires, a white flash
    //                (plain expanding overlay standing in for a radial
    //                blur) ramps up, bass drop fires partway through,
    //                then startGame() runs and the flash fades out
    //                over the first moments of gameplay.
    enum class IntroPhase { TitleCard, Chase, ExitTouch };
    IntroPhase introPhase = IntroPhase::TitleCard;
    float introPhaseTimer = 0.0f; // elapsed time within the current phase

    void startIntro();
    void updateIntro(float deltaTime);
    void renderIntro();

    // ---- Intro player (separate from the gameplay Player class) ----
    std::vector<sf::Texture> introPlayerRunTextures; // 8 frames: assets/Run1.png-Run8.png (same files the real Player uses)
    bool introPlayerTexturesLoaded = false;
    std::optional<sf::Sprite> introPlayerSprite;
    sf::Vector2f introPlayerPos;
    int introPlayerFrame = 0;
    float introPlayerAnimTimer = 0.0f;
    float introPlayerSpawnX = -150.0f; // off-screen left: Chase's start AND the edge the agent later spawns from too

    // ---- Agent: only ever appears in this cutscene ----
    std::vector<sf::Texture> agentRunTextures; // 8 frames: assets/AgentRun1.png-AgentRun8.png
    bool agentTexturesLoaded = false;
    std::optional<sf::Sprite> agentSprite;
    sf::Vector2f agentPos;
    int agentFrame = 0;
    float agentAnimTimer = 0.0f;
    bool introAgentSpawned = false; // false until 2s after the first beep; gates drawing and updating

    // ---- 3 warehouse backgrounds: assets/IntroBackground1-3.png ----
    sf::Texture introBgTexture1, introBgTexture2, introBgTexture3;
    bool introBgLoaded = false;
    std::optional<sf::Sprite> introBgSprite1, introBgSprite2, introBgSprite3;
    int introCurrentBg = 1; // 1, 2, or 3: which one renderIntro() draws this frame
    float introBgScaleMultiplier = 1.247f; // on top of the "fit 800px width" baseline
    float introBgPosX = -99.0f;
    float introBgPosY = -235.0f;

    // ---- Title card: assets/TitleBreach.png. Position is fraction-
    // based (0-1 of window size) so it stays put if resolution changes.
    // introTitleAnchorXFraction=0.09 = 9% from the left;
    // introTitleAnchorYFraction=0.775 = 77.5% from the top. ----
    sf::Texture introTitleTexture;
    bool introTitleTexLoaded = false;
    std::optional<sf::Sprite> introTitleSprite;
    float introTitleScaleMultiplier = 0.25f;      // 1.0 = fits exactly the window's full width, same baseline the background uses: 3x smaller than before
    float introTitleAnchorXFraction = 0.15f;     // 0 = left edge, 1 = right edge
    float introTitleAnchorYFraction = 0.775f;    // 0 = top edge, 1 = bottom edge
    float introTitleDuration = 4.0f;             // TOTAL time on screen, fade-in AND fade-out both included
    float introTitleFadeInDuration = 0.7f;       // first chunk of introTitleDuration: black + title both fade IN together (eased)
    float introTitleFadeOutDuration = 0.6f;      // last chunk of introTitleDuration: black + title both fade OUT together (eased), revealing Background1 underneath (already drawn the whole time)
    float introTitleSoundTriggerFraction = 0.5f; // title sound fires once the fade-IN crosses this fraction (0.5 = halfway), not at time 0
    float introTitleFadeOutRiseY = 20.0f;        // title drifts up this many px while fading out, for a slight "lifting away" cinematic feel
    bool introTitleSoundPlayed = false;          // one-shot guard for the above

    // ---- Chase timing: position + real sound duration ----
    float introHeyStopTriggerDistance = 20.0f; // "Hey stop!" fires once the player has run this many px PAST HIS SPAWN POINT (introPlayerSpawnX): distance-from-spawn, not a screen-fraction, so it's not silently inflated by however far off-screen he starts. Lower toward 0 for an almost-instant shout.
    bool introHeyStopTriggered = false;        // one-shot guard
    float introHeyStopDurationSeconds = 1.25f; // overwritten in init() with the REAL file duration via getDuration(): 1.25 here is just a safe fallback if the file fails to load
    bool introBeepsStarted = false;            // one-shot guard: flips true the instant the shout ACTUALLY finishes (duration-based, not a guessed delay)
    int introBeepCount = 0;                    // counts up each beep: agent bursts out on the 2nd

    // ---- Flicker, Chase phase only (post-beeps-start) ----
    float introFlickerTimer = 0.0f;
    float introBeepOnDuration = 0.3f;  // no longer used for the flicker switch itself: kept only in case you want it back later
    float introBeepGapSeconds = 0.35f;  // no longer used for the flicker switch itself: kept only in case you want it back later
    float introFlickerBeatDuration = 0.3f; // THE flicker speed now: one fixed beat for both on/off, same as the original front-flip-era version. Fast and punchy, but the beep sound CAN get cut off if it's longer than this.

    // ---- White flash (fake radial blur): deliberately NOT tied to
    // GameState::Intro, since it needs to keep fading during the first
    // moments of GameState::Playing too. Drawn unconditionally at the
    // top of render(), checked by alpha > 0 rather than by state.
    float introFlashAlpha = 0.0f;
    bool introFlashRisingUp = false;   // true = ExitTouch ramping 0->255
    bool introFlashFadingOut = false;  // true = post-handoff, ramping 255->0
    float introFlashTimer = 0.0f;      // elapsed time since the rise started: this IS the flash timer
    float introFlashRiseDuration = 0.6f;  // seconds for the flash to go fully white: raise for a slower flash, lower for a snappier one
    float introFlashFadeDuration = 0.85f; // seconds for the fade back down to normal gameplay after the handoff
    float introBassDropTriggerFraction = 0.75f; // bass drop fires when the rise is this fraction done (0.75 = 3/4 of the way to full white): NOT at the same moment as the whip
    bool introBassDropPlayed = false; // one-shot guard for the above

    // ---- Intro-specific tunable positions/speeds: every number a
    // scripted scene like this needs eyeballing on, all in one place ----
    float introRunSpeed = 260.0f;          // px/s: Chase, start to finish
    float introGroundY = 672.0f;           // matches the real Player's own groundY: both player and agent run on this line
    float introGarageDoorX = 850.0f;       // ExitTouch trigger line, matches the garage door art
    float introAgentSpawnX = -150.0f;      // off-screen left, same edge the player himself starts from
    float introAgentGapX = 160.0f;         // final settled distance behind the player once caught up
    float introAgentCatchUpSpeed = 340.0f; // px/s while still closing the gap: faster than introRunSpeed so he visibly gains ground instead of matching 1:1
    float introAgentSizeMultiplier = 1.10f; // agent drawn slightly bigger than the player: more presence as the chaser

    // ---- Post-intro agent cameo: purely cosmetic, no collision.
    // Reuses the SAME agent sprite/textures as the intro cutscene
    // (agentPos/agentFrame/agentAnimTimer/agentSprite above) since the
    // two never run at the same time. Spawns at the start of every
    // Playing session: after the intro AND after every restart --
    // sits running near the left edge (behind the player, who is
    // fixed at screen X=150) until postIntroAgentSlowdownStart
    // seconds, THEN eases into a deceleration (fast at first, visibly
    // slowing down as he goes, NOT a flat constant speed) so he reads
    // as tiring out/giving up the chase rather than just popping off
    //: fully off-screen (past VIEW_LEFT_EDGE) by
    // postIntroAgentGoneBy seconds, then deactivated for good.
    bool postIntroAgentActive = false;
    float postIntroAgentTimer = 0.0f;
    float postIntroAgentSlowdownStart = 0.4f; // seconds spent just running in place before he starts fading back
    float postIntroAgentGoneBy = 5.0f;        // seconds (from spawn) by which he's fully off-screen and deactivated: longer window now, for a more visibly gradual slowdown
    float postIntroAgentStartX = 0.5f;       // near the left edge, behind the player's fixed X=150
    float postIntroAgentSizeMultiplier = 0.90f; // 80% of the previous 1.15 intro size: independent of the intro cutscene's own agent scale

    // Shadow beneath the post-intro agent: same Shade.png the player
    // and vehicles use, loaded as Game's own copy since Player keeps
    // its shadeTexture private. Flat, full-size, full-opacity (unlike
    // the player's jump-height-based shrink/fade) since this agent
    // never leaves the ground.
    sf::Texture agentShadeTexture;
    bool agentShadeLoaded = false;
    std::optional<sf::Sprite> agentShadowSprite;

    // ---- Intro sounds: placeholder filenames ----
    sf::SoundBuffer introTitleSoundBuffer, introWhooshBuffer, introHeyStopBuffer, introBeepBeepBuffer, introWhipBuffer, introBassDropBuffer;
    std::optional<sf::Sound> introTitleSound, introWhooshSound, introHeyStopSound, introBeepBeepSound, introWhipSound, introBassDropSound;
    bool introTitleSoundLoaded = false, introWhooshLoaded = false, introHeyStopLoaded = false, introBeepBeepLoaded = false, introWhipLoaded = false, introBassDropLoaded = false;
};

#endif
