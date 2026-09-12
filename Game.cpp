#include "Game.h"
#include "GameConfig.h"
#include "CollisionUtils.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <utility>
#include <iostream>
#include <limits>
#include <cmath>
#include <string>

// Shared gold color: used for the score counter and the pause icon.
// One place to tune the exact shade.
static const sf::Color GOLD_COLOR(255, 215, 0);

// Hover color for the Game Over restart/quit buttons: one shared
// constant so both always end up pixel-identical, never two
// separately-typed colors that drift apart.
static const sf::Color LAVENDER_HOVER_COLOR(200, 162, 255);

Game::Game(sf::RenderWindow& window) : window(window),
    // ZOOM-OUT VIEW: starts at 800x600 (= window size = no zoom).
    // Raise viewWidth/viewHeight independently or together to zoom out.
    viewWidth(1000.0f),
    viewHeight(800.0f),
    state(GameState::Menu),
    spawnTimer(0.0f),
    spawnInterval(2.0f),
    carSceneState(CarSceneState::Inactive),
    carSceneTimer(0.0f),
    carSceneIncludesVan(true),
    carSceneCooldownTimer(0.0f),
    carSceneSpawnX(900.0f),
    fontLoaded(false),
    currentGroundSpeed(GROUND_SPEED),
    speedSurvivalTime(0.0f),
    musicLoaded(false),
    playHovered(false),
    aboutHovered(false),
    quitHovered(false),
    showAboutOverlay(false),
    resumeHovered(false),
    pausedQuitHovered(false),
    gameOverQuitHovered(false),
    gameOverMenuHovered(false),
    restartHovered(false),
    bustedLogoLoaded(false),
    restartTextureLoaded(false),
    pauseIconLoaded(false),
    menuBackgroundLoaded(false),
    menuPlayLoaded(false),
    menuAboutLoaded(false),
    menuQuitLoaded(false),
    menuMusicStarted(false),
    menuMusicStartTimer(0.0f),
    menuMusicWaitingGap(false),
    menuMusicGapTimer(0.0f),
    menuMusicLastPlayed(-1),
    menuMusicSecondLastPlayed(-1),
    menuParticlesLoaded(false),
    gameOverQuitLoaded(false),
    gameOverMenuLoaded(false),
    hoverLoaded(false), clickLoaded(false), jumpLoaded(false), landingLoaded(false),
    slideLoaded(false), rollLoaded(false), hitLoaded(false), milestoneLoaded(false),
    heliLoaded(false), bodyfallLoaded(false),
    heliMinVolume(15.0f), heliMaxVolume(100.0f),
    milestoneBlinking(false),
    milestoneBlinkTimer(0.0f),
    milestoneColorTimer(0.0f),
    milestoneColorIndex(0) {
    std::srand(std::time(nullptr));

    // ZOOM-OUT VIEW: sized from viewWidth/viewHeight above, centered on
    // the window's own center (400,300) so the zoom expands evenly
    // around the middle rather than shifting everything sideways.
    gameView.setSize({viewWidth, viewHeight});
    gameView.setCenter({400.0f, 300.0f});

    // SFML's default key-repeat fires repeated KeyPressed events at the
    // OS repeat rate while a key is held, which would call
    // jumpSound->play() repeatedly and overlap copies of the same
    // sound. Disabling key repeat makes KeyPressed fire once per press.
    window.setKeyRepeatEnabled(false);

    milestoneColors = { sf::Color::Yellow, sf::Color::Red, sf::Color::Green,
                         sf::Color::Cyan, sf::Color::Magenta };

    // obstacles is a std::deque, not std::vector, because deque
    // already guarantees push/emplace_back never invalidates existing
    // elements (including their live sf::Sound members), so no
    // .reserve() call is needed or possible here.

    // ---- Sound effects ----
    // Each one: load buffer, if successful emplace the Sound and set
    // its own independent volume (0-100), currently all at 100.
    // Plain, non-positional playback: no listener/3D audio involved
    // anywhere in this game anymore.
    hoverLoaded = hoverBuffer.loadFromFile("assets/ButtonHover.mp3");
    if (hoverLoaded) { hoverSound.emplace(hoverBuffer); hoverSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/ButtonHover.mp3\n";

    clickLoaded = clickBuffer.loadFromFile("assets/ButtonClick.mp3");
    if (clickLoaded) { clickSound.emplace(clickBuffer); clickSound->setVolume(80.0f); }
    else std::cerr << "Warning: could not load assets/ButtonClick.mp3\n";

    jumpLoaded = jumpBuffer.loadFromFile("assets/Jump.mp3");
    if (jumpLoaded) { jumpSound.emplace(jumpBuffer); jumpSound->setVolume(70.0f); }
    else std::cerr << "Warning: could not load assets/Jump.mp3\n";

    landingLoaded = landingBuffer.loadFromFile("assets/Landing.mp3");
    if (landingLoaded) { landingSound.emplace(landingBuffer); landingSound->setVolume(75.0f); }
    else std::cerr << "Warning: could not load assets/Landing.mp3\n";

    slideLoaded = slideBuffer.loadFromFile("assets/Sliding.mp3");
    if (slideLoaded) { slideSound.emplace(slideBuffer); slideSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/Sliding.mp3\n";

    rollLoaded = rollBuffer.loadFromFile("assets/Roll.mp3");
    if (rollLoaded) { rollSound.emplace(rollBuffer); rollSound->setVolume(70.0f); }
    else std::cerr << "Warning: could not load assets/Roll.mp3\n";

    hitLoaded = hitBuffer.loadFromFile("assets/Hit.wav");
    if (hitLoaded) { hitSound.emplace(hitBuffer); hitSound->setVolume(80.0f); }
    else std::cerr << "Warning: could not load assets/Hit.wav\n";

    tireHitLoaded = tireHitBuffer.loadFromFile("assets/TireHit.wav");
    if (tireHitLoaded) { tireHitSound.emplace(tireHitBuffer); tireHitSound->setVolume(80.0f); }
    else std::cerr << "Warning: could not load assets/TireHit.wav\n";

    boxHitLoaded = boxHitBuffer.loadFromFile("assets/BoxHit.wav");
    if (boxHitLoaded) { boxHitSound.emplace(boxHitBuffer); boxHitSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/BoxHit.wav\n";

    droneHitLoaded = droneHitBuffer.loadFromFile("assets/DroneHit.wav");
    if (droneHitLoaded) { droneHitSound.emplace(droneHitBuffer); droneHitSound->setVolume(80.0f); }
    else std::cerr << "Warning: could not load assets/DroneHit.wav\n";

    milestoneLoaded = milestoneBuffer.loadFromFile("assets/Milestone.wav");
    if (milestoneLoaded) { milestoneSound.emplace(milestoneBuffer); milestoneSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/Milestone.mp3\n";

    // Helicopter flyby sound: volume gets adjusted live each frame
    // while it's on screen (see update()), based on Background's
    // getHelicopterVolumeFactor(). The 100.0f here is just the max/base.
    heliLoaded = heliBuffer.loadFromFile("assets/HelicopterSound.wav");
    if (heliLoaded) { heliSound.emplace(heliBuffer); heliSound->setVolume(150.0f); }
    else std::cerr << "Warning: could not load assets/HelicopterSound.mp3\n";

    // Plays at the exact instant the death animation reaches its final
    // frame (see Player::consumeDeathFinalFrameEvent()).
    bodyfallLoaded = bodyfallBuffer.loadFromFile("assets/Death_bodyfall.mp3");
    if (bodyfallLoaded) { bodyfallSound.emplace(bodyfallBuffer); bodyfallSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/Death_bodyfall.mp3\n";

    // Footstep sounds: fired from Player's footstepLeft/RightEvent,
    // one pair for bare ground and one for standing on a vehicle.
    // Filenames are placeholders.
    groundFootstepLeftLoaded = groundFootstepLeftBuffer.loadFromFile("assets/GroundFootStepLeft.wav");
    if (groundFootstepLeftLoaded) { groundFootstepLeftSound.emplace(groundFootstepLeftBuffer); groundFootstepLeftSound->setVolume(groundFootstepVolume); }
    else std::cerr << "Warning: could not load assets/GroundFootStepLeft.wav\n";

    groundFootstepRightLoaded = groundFootstepRightBuffer.loadFromFile("assets/GroundFootStepRight.wav");
    if (groundFootstepRightLoaded) { groundFootstepRightSound.emplace(groundFootstepRightBuffer); groundFootstepRightSound->setVolume(groundFootstepVolume); }
    else std::cerr << "Warning: could not load assets/GroundFootStepRight.wav\n";

    carFootstepLeftLoaded = carFootstepLeftBuffer.loadFromFile("assets/CarFootStepLeft.wav");
    if (carFootstepLeftLoaded) { carFootstepLeftSound.emplace(carFootstepLeftBuffer); carFootstepLeftSound->setVolume(carFootstepVolume); }
    else std::cerr << "Warning: could not load assets/CarFootStepLeft.wav\n";

    carFootstepRightLoaded = carFootstepRightBuffer.loadFromFile("assets/CarFootStepRight.wav");
    if (carFootstepRightLoaded) { carFootstepRightSound.emplace(carFootstepRightBuffer); carFootstepRightSound->setVolume(carFootstepVolume); }
    else std::cerr << "Warning: could not load assets/CarFootStepRight.wav\n";

    // Intro cutscene assets: see IntroPhase in Game.h for the phase rundown.
    introPlayerRunTextures.resize(8);
    bool introPlayerRunAllLoaded = true;
    for (int i = 0; i < 8; i++) {
        std::string path = "assets/Run" + std::to_string(i + 1) + ".png";
        if (!introPlayerRunTextures[i].loadFromFile(path)) {
            std::cerr << "Warning: could not load " << path << ": intro player run animation will be broken.\n";
            introPlayerRunAllLoaded = false;
        }
    }
    introPlayerTexturesLoaded = introPlayerRunAllLoaded;
    if (introPlayerTexturesLoaded) {
        introPlayerSprite.emplace(introPlayerRunTextures[0]);
    }

    agentRunTextures.resize(8);
    agentTexturesLoaded = true;
    for (int i = 0; i < 8; i++) {
        std::string path = "assets/AgentRun" + std::to_string(i + 1) + ".png";
        if (!agentRunTextures[i].loadFromFile(path)) {
            std::cerr << "Warning: could not load " << path << ": rename to match your actual agent run files if needed.\n";
            agentTexturesLoaded = false;
        }
    }
    if (agentTexturesLoaded) {
        agentSprite.emplace(agentRunTextures[0]);
    }

    agentShadeLoaded = agentShadeTexture.loadFromFile("assets/Shade.png");
    if (agentShadeLoaded) { agentShadowSprite.emplace(agentShadeTexture); }
    else std::cerr << "Warning: could not load assets/Shade.png: agent shadow will be invisible/broken until this file exists.\n";

    introBgLoaded = true;
    if (!introBgTexture1.loadFromFile("assets/IntroBackground1.png")) { std::cerr << "Warning: could not load assets/IntroBackground1.png: rename to match your actual file.\n"; introBgLoaded = false; }
    if (!introBgTexture2.loadFromFile("assets/IntroBackground2.png")) { std::cerr << "Warning: could not load assets/IntroBackground2.png: rename to match your actual file.\n"; introBgLoaded = false; }
    if (!introBgTexture3.loadFromFile("assets/IntroBackground3.png")) { std::cerr << "Warning: could not load assets/IntroBackground3.png: rename to match your actual file.\n"; introBgLoaded = false; }
    if (introBgLoaded) {
        introBgSprite1.emplace(introBgTexture1);
        introBgSprite2.emplace(introBgTexture2);
        introBgSprite3.emplace(introBgTexture3);
    }

    introTitleTexLoaded = introTitleTexture.loadFromFile("assets/TitleBreach.png");
    if (introTitleTexLoaded) { introTitleSprite.emplace(introTitleTexture); }
    else std::cerr << "Warning: could not load assets/TitleBreach.png: rename to match your actual title art file.\n";

    introTitleSoundLoaded = introTitleSoundBuffer.loadFromFile("assets/TitleSound.wav");
    if (introTitleSoundLoaded) { introTitleSound.emplace(introTitleSoundBuffer); introTitleSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/TitleSound.wav\n";

    introWhooshLoaded = introWhooshBuffer.loadFromFile("assets/Whoosh.wav");
    if (introWhooshLoaded) { introWhooshSound.emplace(introWhooshBuffer); introWhooshSound->setVolume(90.0f); }
    else std::cerr << "Warning: could not load assets/Whoosh.wav\n";

    introHeyStopLoaded = introHeyStopBuffer.loadFromFile("assets/HeyStop.wav");
    if (introHeyStopLoaded) {
        introHeyStopSound.emplace(introHeyStopBuffer);
        introHeyStopSound->setVolume(100.0f);
        // Read the REAL duration from the file: the background swap
        // and first beep now fire exactly when the shout finishes, not
        // on a guessed delay.
        introHeyStopDurationSeconds = introHeyStopBuffer.getDuration().asSeconds();
    } else std::cerr << "Warning: could not load assets/HeyStop.wav\n";

    introBeepBeepLoaded = introBeepBeepBuffer.loadFromFile("assets/BeepBeep.wav");
    if (introBeepBeepLoaded) {
        introBeepBeepSound.emplace(introBeepBeepBuffer);
        introBeepBeepSound->setVolume(85.0f);
        // Same idea: the flicker beat length is driven by the actual
        // beep file's own length (plus a little breathing room) so
        // beeps never overlap or get cut off.
        introBeepOnDuration = introBeepBeepBuffer.getDuration().asSeconds();
    } else std::cerr << "Warning: could not load assets/BeepBeep.wav\n";

    introWhipLoaded = introWhipBuffer.loadFromFile("assets/WhipTransition.wav");
    if (introWhipLoaded) { introWhipSound.emplace(introWhipBuffer); introWhipSound->setVolume(90.0f); }
    else std::cerr << "Warning: could not load assets/WhipTransition.wav\n";

    introBassDropLoaded = introBassDropBuffer.loadFromFile("assets/BassDrop.wav");
    if (introBassDropLoaded) { introBassDropSound.emplace(introBassDropBuffer); introBassDropSound->setVolume(100.0f); }
    else std::cerr << "Warning: could not load assets/BassDrop.wav\n";

    // Drone hum: owned centrally here now (not per-obstacle-instance
    // anymore) so it can start before the drone obstacle even exists
    // and keep playing after the obstacle is gone. See the drone sound
    // block in update() for the timing/envelope/cutoff logic.
    droneLoaded = droneBuffer.loadFromFile("assets/Drone.wav");
    if (!droneLoaded) std::cerr << "Warning: could not load assets/Drone.wav\n";

    // Van-flip slow-motion whoosh: plays once, right as the
    // slow-motion effect actually starts (end of the pre-delay, start
    // of the ramp-down). Same "load buffer only, emplace fresh each
    // time it's actually triggered" pattern as the drone sound, since
    // this needs a brand new sf::Sound instance per trigger rather
    // than one long-lived one. See updateSlowMo() in update().
    slowMoLoaded = slowMoBuffer.loadFromFile("assets/VanFlipSlowMo.wav");
    if (!slowMoLoaded) std::cerr << "Warning: could not load assets/VanFlipSlowMo.wav\n";

    // ---- DOWNLANE CAR ENGINE/HORN + UP-LANE TRAFFIC ENGINE/HORN ----
    // These sounds are actually loaded and owned inside Obstacle.cpp
    // (downlane) and Background.cpp (up-lane/truck), since each car
    // instance needs its OWN sf::Sound so many cars can overlap at
    // once: Game.cpp just sets the shared volume for each category
    // here, same 0-100 pattern as everything above.
    Obstacle::setCarEngineVolume(85.0f); // peaks at 55 at screen center; curve handles the rest
    Obstacle::setCarHornVolume(100.0f);
    Obstacle::setDroneSoundVolume(100.0f); // base volume before positional distance attenuation is applied
    background.setUpLaneEngineVolume(90.0f);
    background.setUpLaneHornVolume(90.0f);
    background.setTruckEngineVolume(120.0f);
    background.setTruckHornVolume(100.0f);
    background.setVanEngineVolume(80.0f);
    background.setVanHornVolume(100.0f);

    // BACKGROUND MUSIC: starts the instant Play is pressed, stops the
    // instant the player dies (see updateLandableObstacles/checkCollisions).
    // Music volume is separate from the SFX above, lowered from full
    // volume since it was overpowering the sound effects. Adjust this
    // one number to rebalance music vs everything else.
    const float musicVolume = 100.0f;
    musicLoaded = bgMusic.openFromFile("assets/B-Music.mp3");
    if (!musicLoaded) {
        std::cerr << "Warning: could not load assets/B-Music.mp3: no background music.\n";
    } else {
        bgMusic.setVolume(musicVolume);
        bgMusic.setLooping(true);
    }

    fontLoaded = font.openFromFile("assets/font.ttf");
    symbolFontLoaded = symbolFont.openFromFile("assets/NotoSansSymbols2-Regular.ttf");
    if (!fontLoaded) {
        std::cerr << "Warning: could not load assets/font.ttf: text will not display.\n";
    } else {
        float cx = window.getSize().x / 2.0f;
        float cy = window.getSize().y / 2.0f;

        // ---- Menu high score text ----
        // Positioned to sit just below the QUIT button image: see
        // the "Menu screen graphics" block further down (near the
        // pause icon setup) for where quitCenterY actually lives now
        // that logo/play/quit are sprites, not text.
        const float highScoreY = cy + 150.0f;

        menuHighScoreText.emplace(font, "High Score: 0", 20);
        menuHighScoreText->setFillColor(sf::Color::White);
        menuHighScoreText->setOutlineColor(sf::Color::Black);
        menuHighScoreText->setOutlineThickness(2.0f);
        sf::FloatRect hBounds = menuHighScoreText->getLocalBounds();
        menuHighScoreText->setOrigin({ hBounds.size.x / 2.0f, hBounds.size.y / 2.0f });
        menuHighScoreText->setPosition({ cx, highScoreY });

        // ---- Skip-intro prompt ----
        // Small, plain white, bottom-right of the window: shown for
        // the entire intro cutscene (title card through the chase),
        // gone the instant it hands off into real gameplay since that's
        // a different GameState and this only ever draws during Intro.
        skipIntroText.emplace(font, "Press Enter to skip >", 14);
        skipIntroText->setFillColor(sf::Color::White);
        sf::FloatRect skipBounds = skipIntroText->getLocalBounds();
        float skipMargin = 16.0f;
        skipIntroText->setPosition({
            static_cast<float>(window.getSize().x) - skipBounds.size.x - skipBounds.position.x - skipMargin,
            static_cast<float>(window.getSize().y) - skipBounds.size.y - skipBounds.position.y - skipMargin
        });

        // ---- About overlay content ----
        // Title, centered near the top of the 500x350 panel (see
        // renderMenu()). Panel-relative offsets, not absolute screen
        // position: actual position gets set each frame in
        // renderMenu() once the panel's real on-screen center is known.
        aboutTitleText.emplace(font, "STREET OUTLAW", 26);
        aboutTitleText->setFillColor(sf::Color::White);
        aboutTitleText->setOutlineColor(sf::Color::Black);
        aboutTitleText->setOutlineThickness(2.0f);

        // Body: description + controls, manually line-wrapped to fit
        // the panel width at this character size. Reflects the ACTUAL
        // input handling in this file: UP only for jump (Space was
        // removed), Down behavior depends on airborne vs grounded, Esc
        // pauses. Update this text if any of those ever change.
        aboutBodyText.emplace(font,
            "You're an outlaw on the run through the\n"
            "city streets. Dodge traffic, drones, and\n"
            "everything else in your way. Survive as\n"
            "long as you can and chase the high score.\n"
            "\n"
            "CONTROLS\n"
            "UP: Jump (occasionally breaks into a\n"
            "front flip, back flip, or side flip)\n"
            "DOWN (while airborne): Roll\n"
            "DOWN (on ground): Slide\n"
            "ESC: Pause",
            16);
        aboutBodyText->setFillColor(sf::Color::White);

        // ---- In-game score text ----
        scoreText.emplace(font, "Score: 0", 24);
        scoreText->setFillColor(GOLD_COLOR);
        scoreText->setPosition({ 20.0f, 20.0f });

        // ---- Pause overlay text ----
        pausedTitleText.emplace(font, "PAUSED", 42);
        pausedTitleText->setFillColor(sf::Color::White);
        sf::FloatRect ptBounds = pausedTitleText->getLocalBounds();
        pausedTitleText->setOrigin({ ptBounds.size.x / 2.0f, ptBounds.size.y / 2.0f });
        pausedTitleText->setPosition({ cx, cy - 100.0f });

        resumeText.emplace(font, "RESUME", 30);
        resumeText->setFillColor(sf::Color::White);
        sf::FloatRect rsBounds = resumeText->getLocalBounds();
        resumeText->setOrigin({ rsBounds.size.x / 2.0f, rsBounds.size.y / 2.0f });
        resumeText->setPosition({ cx, cy - 10.0f });

        pausedQuitText.emplace(font, "QUIT", 30);
        pausedQuitText->setFillColor(sf::Color::White);
        sf::FloatRect pqBounds = pausedQuitText->getLocalBounds();
        pausedQuitText->setOrigin({ pqBounds.size.x / 2.0f, pqBounds.size.y / 2.0f });
        pausedQuitText->setPosition({ cx, cy + 70.0f });

        resumeButtonRect = { { cx - 100.0f, cy - 35.0f }, { 200.0f, 50.0f } };
        pausedQuitButtonRect = { { cx - 100.0f, cy + 45.0f }, { 200.0f, 50.0f } };

        // ---- Game over screen layout ----
        // All Y positions below are relative to cy so the whole screen
        // shifts together if cy ever changes. Score/high score values
        // are set live in update() (see isDeathAnimationFinished()),
        // since they depend on the actual run's score: the strings
        // here are just placeholders so the objects exist.
        const float logoCenterY = cy - 110.0f;
        const float scoreLineY = cy - 35.0f;
        const float highScoreLineY = cy - 5.0f;

        // Black outline on all these: they sit directly on the busy
        // background photo, and plain white/gold fill was hard to
        // read against it. A dark outline is the same trick the logo
        // images already use (glow/outline against a photo bg).
        auto addOutline = [](sf::Text& t) {
            t.setOutlineColor(sf::Color::Black);
            t.setOutlineThickness(2.0f);
        };

        scoreLabelText.emplace(font, "SCORE:", 18);
        scoreLabelText->setFillColor(sf::Color::White);
        addOutline(*scoreLabelText);
        scoreValueText.emplace(font, "0", 18);
        scoreValueText->setFillColor(GOLD_COLOR);
        addOutline(*scoreValueText);

        highScoreLabelText.emplace(font, "HIGH SCORE:", 18);
        highScoreLabelText->setFillColor(sf::Color::White);
        addOutline(*highScoreLabelText);
        highScoreValueText.emplace(font, "0", 18);
        highScoreValueText->setFillColor(GOLD_COLOR);
        addOutline(*highScoreValueText);

        // gameOverQuitSprite (image) and restartSprite are loaded
        // further down, alongside bustedLogoSprite, since they
        // don't need the font: their Y positions are hardcoded
        // there (cy+40 and cy+75) rather than reusing local names
        // from this scope.

        // Store these so update() can re-run the score/high-score
        // layout (label+value widths change with the digits) without
        // needing to recompute cx/logoCenterY etc. from scratch.
        gameOverScoreLineY = scoreLineY;
        gameOverHighScoreLineY = highScoreLineY;
    }

    // "BUSTED" logo image: assets/Busted.png (was GameOverLogo.png/"GAME OVER")
    bustedLogoLoaded = bustedLogoTexture.loadFromFile("assets/Busted.png");
    if (!bustedLogoLoaded) {
        std::cerr << "Warning: could not load assets/Busted.png: game over graphic will not display.\n";
    } else {
        bustedLogoSprite.emplace(bustedLogoTexture);

        float targetWidth = 420.0f;
        float texW = static_cast<float>(bustedLogoTexture.getSize().x);
        float scale = targetWidth / texW;
        bustedLogoSprite->setScale({ scale, scale });

        sf::FloatRect goBounds = bustedLogoSprite->getLocalBounds();
        bustedLogoSprite->setOrigin({ goBounds.size.x / 2.0f, goBounds.size.y / 2.0f });
        bustedLogoSprite->setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f - 110.0f });
    }

    // "Press Enter to Restart" image: assets/PressEnterToRestart.png
    restartTextureLoaded = restartTexture.loadFromFile("assets/PressEnterToRestart.png");
    if (!restartTextureLoaded) {
        std::cerr << "Warning: could not load assets/PressEnterToRestart.png: restart prompt will not display.\n";
    } else {
        restartSprite.emplace(restartTexture);

        float targetWidth = 340.0f;
        float texW = static_cast<float>(restartTexture.getSize().x);
        float scale = targetWidth / texW;
        restartSprite->setScale({ scale, scale });

        sf::FloatRect rsBounds = restartSprite->getLocalBounds();
        restartSprite->setOrigin({ rsBounds.size.x / 2.0f, rsBounds.size.y / 2.0f });
        restartSprite->setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f + 40.0f });

        sf::FloatRect rsGlobal = restartSprite->getGlobalBounds();
        restartButtonRect = rsGlobal;
    }

    // Restart's actual rendered letter height: Menu and Quit below
    // both match this height rather than their own native aspect ratio,
    // since they're different source images (different word counts/
    // aspect ratios) and matching by width made them wildly different
    // sizes.
    float restartRenderedHeight = 135.0f * (340.0f / static_cast<float>(restartTexture.getSize().x));
    float restartHalfH = restartRenderedHeight / 2.0f;
    const float gameOverButtonGap = 14.0f; // equal edge-to-edge gap between Restart/Menu/Quit

    // "RETURN TO MENU" image: assets/GameOverMenu.png. Stacked
    // directly under Restart.
    gameOverMenuLoaded = gameOverMenuTexture.loadFromFile("assets/GameOverMenu.png");
    float menuHalfH = 0.0f;
    float menuCenterY = window.getSize().y / 2.0f + 40.0f + restartHalfH; // fallback if load fails
    if (!gameOverMenuLoaded) {
        std::cerr << "Warning: could not load assets/GameOverMenu.png: return-to-menu button will not display.\n";
    } else {
        gameOverMenuSprite.emplace(gameOverMenuTexture);

        float texH = static_cast<float>(gameOverMenuTexture.getSize().y);
        float scale = restartRenderedHeight / texH;
        gameOverMenuSprite->setScale({ scale, scale });

        sf::FloatRect gmBounds = gameOverMenuSprite->getLocalBounds();
        gameOverMenuSprite->setOrigin({ gmBounds.size.x / 2.0f, gmBounds.size.y / 2.0f });

        menuHalfH = (gmBounds.size.y * scale) / 2.0f;
        menuCenterY = (window.getSize().y / 2.0f + 40.0f) + restartHalfH + gameOverButtonGap + menuHalfH;
        gameOverMenuSprite->setPosition({ window.getSize().x / 2.0f, menuCenterY });

        gameOverMenuButtonRect = gameOverMenuSprite->getGlobalBounds();
    }

    // "QUIT" image for the Game Over screen: assets/GameOverQuit.png
    // (matches PressEnterToRestart's neon style; the main menu Quit
    // button is a separate, differently-sized image)
    gameOverQuitLoaded = gameOverQuitTexture.loadFromFile("assets/GameOverQuit.png");
    if (!gameOverQuitLoaded) {
        std::cerr << "Warning: could not load assets/GameOverQuit.png: game over quit button will not display.\n";
    } else {
        gameOverQuitSprite.emplace(gameOverQuitTexture);

        float texH = static_cast<float>(gameOverQuitTexture.getSize().y);
        float scale = restartRenderedHeight / texH;
        gameOverQuitSprite->setScale({ scale, scale });

        sf::FloatRect gqBounds = gameOverQuitSprite->getLocalBounds();
        gameOverQuitSprite->setOrigin({ gqBounds.size.x / 2.0f, gqBounds.size.y / 2.0f });

        float quitHalfH = (gqBounds.size.y * scale) / 2.0f;
        float quitCenterY = menuCenterY + menuHalfH + gameOverButtonGap + quitHalfH;
        gameOverQuitSprite->setPosition({ window.getSize().x / 2.0f, quitCenterY });

        gameOverQuitButtonRect = gameOverQuitSprite->getGlobalBounds();
    }

    // PAUSE BUTTON: small clickable area, top-right corner.
    pauseButtonRect = { { window.getSize().x - 50.0f, 15.0f }, { 35.0f, 35.0f } };

    // Pause icon image: assets/PauseIcon.png
    pauseIconLoaded = pauseIconTexture.loadFromFile("assets/PauseIcon.png");
    if (!pauseIconLoaded) {
        std::cerr << "Warning: could not load assets/PauseIcon.png: pause icon will not display.\n";
    } else {
        pauseIconSprite.emplace(pauseIconTexture);

        float targetHeight = 30.0f; // fits the existing 35x35 button area
        float texH = static_cast<float>(pauseIconTexture.getSize().y);
        float scale = targetHeight / texH;
        pauseIconSprite->setScale({ scale, scale });

        sf::FloatRect piBounds = pauseIconSprite->getLocalBounds();
        pauseIconSprite->setOrigin({ piBounds.size.x / 2.0f, piBounds.size.y / 2.0f });
        pauseIconSprite->setPosition({
            pauseButtonRect.position.x + pauseButtonRect.size.x / 2.0f,
            pauseButtonRect.position.y + pauseButtonRect.size.y / 2.0f
        });
    }

    // ---- Menu screen graphics (static background + button stack) ----
    {
        float winW = static_cast<float>(window.getSize().x);
        float winH = static_cast<float>(window.getSize().y);
        const float buttonCenterX = 0.221f * winW;

        // Full-screen static background: title + character art baked in.
        menuBackgroundLoaded = menuBackgroundTexture.loadFromFile("assets/MenuBackground.png");
        if (!menuBackgroundLoaded) {
            std::cerr << "Warning: could not load assets/MenuBackground.png: menu background will not display.\n";
        } else {
            menuBackgroundSprite.emplace(menuBackgroundTexture);
            sf::Vector2u texSize = menuBackgroundTexture.getSize();
            menuBackgroundSprite->setScale({
                winW / static_cast<float>(texSize.x),
                winH / static_cast<float>(texSize.y)
            });
            menuBackgroundSprite->setPosition({ 0.0f, 0.0f });
        }

        // Load all 3 button textures first (without positioning yet) --
        // their splash-paint art still bleeds a bit past the visible
        // "box" even after trimming, so each one's actual scaled
        // HEIGHT differs a little from the others. Positioning needs
        // those real heights up front to get equal edge-to-edge gaps
        // between elements, instead of guessed fixed Y positions.
        const float buttonTargetWidth = 145.0f; // shrunk further to fit everything above the tagline zone (see stackStartY note)
        const float buttonGap = 12.0f;           // slim edge-to-edge gap between Play/About/Quit

        menuPlayLoaded = menuPlayTexture.loadFromFile("assets/MenuPlay.png");
        if (!menuPlayLoaded) std::cerr << "Warning: could not load assets/MenuPlay.png: play button will not display.\n";

        menuAboutLoaded = menuAboutTexture.loadFromFile("assets/MenuAbout.png");
        if (!menuAboutLoaded) std::cerr << "Warning: could not load assets/MenuAbout.png: about button will not display.\n";

        menuQuitLoaded = menuQuitTexture.loadFromFile("assets/MenuQuit.png");
        if (!menuQuitLoaded) std::cerr << "Warning: could not load assets/MenuQuit.png: quit button will not display.\n";

        auto scaledHalfHeight = [&](const sf::Texture& tex) {
            float texW = static_cast<float>(tex.getSize().x);
            float texH = static_cast<float>(tex.getSize().y);
            float scale = buttonTargetWidth / texW;
            return (texH * scale) / 2.0f;
        };

        float playHalfH  = menuPlayLoaded  ? scaledHalfHeight(menuPlayTexture)  : 0.0f;
        float aboutHalfH = menuAboutLoaded ? scaledHalfHeight(menuAboutTexture) : 0.0f;
        float quitHalfH  = menuQuitLoaded  ? scaledHalfHeight(menuQuitTexture)  : 0.0f;
        float highScoreHalfH = 12.0f; // approx half-height of the 20pt high-score text

        // Order: Play, About, Quit, each separated by a slim
        // buttonGap, then a double gap, then High Score at the
        // bottom. Everything has to fit in the safe zone between the
        // logo's bottom (measured directly from the background art,
        // raw pixel y=360 out of 1086 = 0.331 of the image) and the
        // "RUN. DODGE. OWN THE STREETS." tagline baked into the art
        // (measured the same way, starts at raw y=855 = 0.787). High
        // Score is bare outlined text with no solid box behind it
        // (unlike the buttons, which have an opaque box and safely
        // cover whatever's behind them), so it's the one element that
        // actually needs to clear the tagline with real margin.
        float stackStartY = 0.35f * winH;  // clears the logo's paint-drip tips with ~10px margin
        float playCenterY = stackStartY + playHalfH;
        float aboutCenterY = playCenterY + playHalfH + buttonGap + aboutHalfH;
        float quitCenterY = aboutCenterY + aboutHalfH + buttonGap + quitHalfH;
        float highScoreY = quitCenterY + quitHalfH + (buttonGap * 2.0f) + highScoreHalfH;
        float totalHeight = (playHalfH * 2) + buttonGap +
                             (aboutHalfH * 2) + buttonGap +
                             (quitHalfH * 2) + (buttonGap * 2.0f) +
                             (highScoreHalfH * 2);
        (void)totalHeight; // computed for potential re-centering, unused for now

        // PLAY button: assets/MenuPlay.png
        if (menuPlayLoaded) {
            menuPlaySprite.emplace(menuPlayTexture);
            float scale = buttonTargetWidth / static_cast<float>(menuPlayTexture.getSize().x);
            menuPlaySprite->setScale({ scale, scale });
            sf::FloatRect b = menuPlaySprite->getLocalBounds();
            menuPlaySprite->setOrigin({ b.size.x / 2.0f, b.size.y / 2.0f });
            menuPlaySprite->setPosition({ buttonCenterX, playCenterY });
            playButtonRect = menuPlaySprite->getGlobalBounds();
        }

        // ABOUT button: assets/MenuAbout.png (opens an empty overlay panel)
        if (menuAboutLoaded) {
            menuAboutSprite.emplace(menuAboutTexture);
            float scale = buttonTargetWidth / static_cast<float>(menuAboutTexture.getSize().x);
            menuAboutSprite->setScale({ scale, scale });
            sf::FloatRect b = menuAboutSprite->getLocalBounds();
            menuAboutSprite->setOrigin({ b.size.x / 2.0f, b.size.y / 2.0f });
            menuAboutSprite->setPosition({ buttonCenterX, aboutCenterY });
            aboutButtonRect = menuAboutSprite->getGlobalBounds();
        }

        // QUIT button: assets/MenuQuit.png
        if (menuQuitLoaded) {
            menuQuitSprite.emplace(menuQuitTexture);
            float scale = buttonTargetWidth / static_cast<float>(menuQuitTexture.getSize().x);
            menuQuitSprite->setScale({ scale, scale });
            sf::FloatRect b = menuQuitSprite->getLocalBounds();
            menuQuitSprite->setOrigin({ b.size.x / 2.0f, b.size.y / 2.0f });
            menuQuitSprite->setPosition({ buttonCenterX, quitCenterY });
            quitButtonRect = menuQuitSprite->getGlobalBounds();
        }

        // menuHighScoreText's Y position also needs to follow this new
        // layout: stored so the existing text setup (built earlier,
        // alongside the font block) can be repositioned here too.
        if (fontLoaded) {
            menuHighScoreText->setPosition({ buttonCenterX, highScoreY });
        }
    }

    // ---- Menu ambient particles ----
    // 4 texture variants each: assets/ParticleDot1.png..4.png and
    // assets/ParticleCluster1.png..4.png: randomly picked per
    // particle at spawn/respawn (see setupMenuParticles()/
    // updateMenuParticles()) so identical particles don't all look
    // mechanically identical on screen.
    particleDotTextures.resize(4);
    particleClusterTextures.resize(4);
    bool allLoaded = true;
    for (int i = 0; i < 4; i++) {
        std::string dotPath = "assets/ParticleDot" + std::to_string(i + 1) + ".png";
        std::string clusterPath = "assets/ParticleCluster" + std::to_string(i + 1) + ".png";
        if (!particleDotTextures[i].loadFromFile(dotPath)) {
            std::cerr << "Warning: could not load " << dotPath << ": menu particles will not display.\n";
            allLoaded = false;
        }
        if (!particleClusterTextures[i].loadFromFile(clusterPath)) {
            std::cerr << "Warning: could not load " << clusterPath << ": menu particles will not display.\n";
            allLoaded = false;
        }
    }
    menuParticlesLoaded = allLoaded;
    if (menuParticlesLoaded) {
        setupMenuParticles();
    }
}

void Game::setupMenuParticles() {
    // 28 total, ~70/30 dot/cluster split (20 dots, 8 clusters).
    const int totalParticles = 28;
    const int dotCount = 20;

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    float cx = winW / 2.0f;
    float cy = winH / 2.0f;

    menuParticles.clear();
    menuParticles.reserve(totalParticles);

    // Diagonal unit vectors toward each of the 4 corners, index-matched
    // to the texture arrays below: corner index N always uses
    // ParticleDot(N+1).png / ParticleCluster(N+1).png, so each
    // direction has a fixed, dedicated image (no randomness):
    //   0 = top-left     -> ParticleDot1.png    / ParticleCluster1.png
    //   1 = top-right     -> ParticleDot2.png    / ParticleCluster2.png
    //   2 = bottom-left   -> ParticleDot3.png    / ParticleCluster3.png
    //   3 = bottom-right  -> ParticleDot4.png    / ParticleCluster4.png
    const float diag = 0.70710678f; // 1/sqrt(2)
    float corners[4][2] = {
        { -diag, -diag }, // top-left
        {  diag, -diag }, // top-right
        { -diag,  diag }, // bottom-left
        {  diag,  diag }, // bottom-right
    };

    for (int i = 0; i < totalParticles; i++) {
        bool isDot = i < dotCount;
        int c = std::rand() % 4;
        int variant = c; // texture is fixed per corner: see comment above

        MenuParticle p{
            sf::Sprite(isDot ? particleDotTextures[variant] : particleClusterTextures[variant]),
            corners[c][0], corners[c][1],
            0.0f, // distanceTraveled: starts at center
            0.0f, // speed, set below
            isDot
        };

        // Origin defaults to the sprite's top-left corner, not its
        // center: without this, setPosition() below places the
        // TOP-LEFT of the image at the calculated point instead of
        // its visual middle, making every particle look offset
        // down-and-right of where it's actually supposed to be.
        {
            sf::Vector2u texSize = p.sprite.getTexture().getSize();
            p.sprite.setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });
        }

        // Dots travel 2.5x faster than clusters, and grow further
        // (10%->200% vs 10%->100%): see updateMenuParticles() for
        // the scale interpolation.
        p.speed = isDot ? 50.0f : 20.0f;

        // Stagger starting distance so they don't all launch from
        // dead center in a single visible pulse: spread across the
        // first 90% of the journey on first spawn only; every respawn
        // after this restarts cleanly at distanceTraveled=0.
        float maxDist = std::sqrt(cx * cx + cy * cy);
        p.distanceTraveled = static_cast<float>(std::rand() % 90) / 100.0f * maxDist;

        float scaleFrac = p.distanceTraveled / maxDist; // 0 at center, ~0.9 near edge
        float maxScale = isDot ? 0.5f : 0.25f; // 1/4 of previous max (2.0/1.0)
        float s = 0.10f + scaleFrac * (maxScale - 0.10f);
        p.sprite.setScale({ s, s });

        float px = cx + p.dirX * p.distanceTraveled;
        float py = cy + p.dirY * p.distanceTraveled;
        p.sprite.setPosition({ px, py });

        menuParticles.push_back(std::move(p));
    }
}

void Game::updateMenuParticles(float deltaTime) {
    if (!menuParticlesLoaded) return;

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    float cx = winW / 2.0f;
    float cy = winH / 2.0f;
    float maxDist = std::sqrt(cx * cx + cy * cy); // distance from center to any corner

    const float diag = 0.70710678f;
    float corners[4][2] = {
        { -diag, -diag }, { diag, -diag }, { -diag, diag }, { diag, diag },
    };

    for (auto& p : menuParticles) {
        p.distanceTraveled += p.speed * deltaTime;

        if (p.distanceTraveled >= maxDist) {
            // Reached its corner: reset to center with a freshly
            // random corner assignment. Texture is fixed per corner
            // (index-matched, see setupMenuParticles()), so it's set
            // from the same index: not re-randomized.
            int c = std::rand() % 4;
            p.dirX = corners[c][0];
            p.dirY = corners[c][1];
            p.distanceTraveled = 0.0f;

            p.sprite.setTexture(p.isDot ? particleDotTextures[c] : particleClusterTextures[c]);

            // Re-center the origin for the newly-assigned texture --
            // covers the case where the 4 corner variants aren't all
            // identically sized.
            sf::Vector2u texSize = p.sprite.getTexture().getSize();
            p.sprite.setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });
        }

        float scaleFrac = p.distanceTraveled / maxDist;
        float maxScale = p.isDot ? 0.5f : 0.25f; // 1/4 of previous max (2.0/1.0)
        float s = 0.10f + scaleFrac * (maxScale - 0.10f);
        p.sprite.setScale({ s, s });

        float px = cx + p.dirX * p.distanceTraveled;
        float py = cy + p.dirY * p.distanceTraveled;
        p.sprite.setPosition({ px, py });
    }
}

void Game::renderMenuParticles() {
    if (!menuParticlesLoaded) return;

    sf::RenderStates additive(sf::BlendAdd);
    for (const auto& p : menuParticles) {
        window.draw(p.sprite, additive);
    }
}

bool Game::isMouseOverRect(sf::Vector2i mousePos, const sf::FloatRect& rect) const {
    return mousePos.x >= rect.position.x && mousePos.x <= rect.position.x + rect.size.x &&
           mousePos.y >= rect.position.y && mousePos.y <= rect.position.y + rect.size.y;
}

void Game::run() {
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
            // playHover(): only plays hoverSound on a false->true
            // transition, not every frame the mouse sits still over a
            // button. Pause icon is intentionally NOT wired here --
            // no hover sound during gameplay, per spec.
            auto playHover = [&](bool& flag, bool newVal) {
                if (newVal && !flag && hoverLoaded) hoverSound->play();
                flag = newVal;
            };

            if (state == GameState::Menu) {
                playHover(playHovered, isMouseOverRect(moved->position, playButtonRect));
                playHover(aboutHovered, isMouseOverRect(moved->position, aboutButtonRect));
                playHover(quitHovered, isMouseOverRect(moved->position, quitButtonRect));
            } else if (state == GameState::Paused) {
                playHover(resumeHovered, isMouseOverRect(moved->position, resumeButtonRect));
                playHover(pausedQuitHovered, isMouseOverRect(moved->position, pausedQuitButtonRect));
            } else if (state == GameState::GameOver) {
                playHover(gameOverQuitHovered, isMouseOverRect(moved->position, gameOverQuitButtonRect));
                playHover(gameOverMenuHovered, gameOverMenuLoaded && isMouseOverRect(moved->position, gameOverMenuButtonRect));
                playHover(restartHovered, restartTextureLoaded && isMouseOverRect(moved->position, restartButtonRect));
            }
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button != sf::Mouse::Button::Left) continue;

            if (state == GameState::Menu) {
                if (showAboutOverlay) {
                    // Overlay is open: this click just closes it,
                    // regardless of where on screen it landed. Doesn't
                    // fall through to Play/About/Quit underneath.
                    if (clickLoaded) clickSound->play();
                    showAboutOverlay = false;
                } else if (isMouseOverRect(click->position, playButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    startIntro();
                } else if (isMouseOverRect(click->position, aboutButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    showAboutOverlay = true;
                } else if (isMouseOverRect(click->position, quitButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    window.close();
                }
            } else if (state == GameState::Playing) {
                if (isMouseOverRect(click->position, pauseButtonRect)) {
                    // No click sound here on purpose: pause icon is
                    // excluded from menu-button audio feedback, same
                    // as its hover exclusion.
                    state = GameState::Paused;
                    if (musicLoaded) bgMusic.pause();
                    if (heliLoaded && background.isHelicopterActive()) heliSound->pause();
                    if (droneSoundActive && droneSound) droneSound->pause();
                }
            } else if (state == GameState::Paused) {
                if (isMouseOverRect(click->position, resumeButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    state = GameState::Playing;
                    if (musicLoaded) bgMusic.play();
                    if (heliLoaded && background.isHelicopterActive()) heliSound->play();
                    if (droneSoundActive && droneSound) droneSound->play();
                } else if (isMouseOverRect(click->position, pausedQuitButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    window.close();
                }
            } else if (state == GameState::GameOver) {
                if (isMouseOverRect(click->position, gameOverQuitButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    window.close();
                } else if (gameOverMenuLoaded && isMouseOverRect(click->position, gameOverMenuButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    returnToMenu();
                } else if (restartTextureLoaded && isMouseOverRect(click->position, restartButtonRect)) {
                    if (clickLoaded) clickSound->play();
                    restartGame();
                }
            }
        }

        if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
            if (state == GameState::Menu) {
                if (keyPress->code == sf::Keyboard::Key::Enter && !showAboutOverlay) {
                    if (clickLoaded) clickSound->play();
                    startIntro();
                }
            } else if (state == GameState::Intro) {
                if (keyPress->code == sf::Keyboard::Key::Enter) {
                    // Skips straight to real gameplay, same handoff
                    // point the cutscene reaches naturally at the end
                    // of ExitTouch: startGame() does its own full
                    // reset regardless of which intro phase this was
                    // pressed during, so it's safe from any point.
                    if (clickLoaded) clickSound->play();
                    startGame();
                }
            } else if (state == GameState::GameOver) {
                if (keyPress->code == sf::Keyboard::Key::Enter) {
                    if (clickLoaded) clickSound->play();
                    restartGame();
                }
            } else if (state == GameState::Playing) {
                if (keyPress->code == sf::Keyboard::Key::Up) {
                    player.jump();
                    if (jumpLoaded) {
                        jumpSound->setPitch(slowMoPitch);
                        jumpSound->setVolume(70.0f * slowMoVolumeMultiplier);
                        jumpSound->play();
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::Down) {
                    player.onDownPressed();
                    // onDownPressed() internally picks slide vs roll --
                    // check which one actually resulted to play the
                    // matching sound.
                    if (player.getState() == PlayerState::Sliding && slideLoaded) {
                        slideSound->setPitch(slowMoPitch);
                        slideSound->setVolume(100.0f * slowMoVolumeMultiplier);
                        slideSound->play();
                    } else if (player.getState() == PlayerState::Rolling && rollLoaded) {
                        rollSound->setPitch(slowMoPitch);
                        rollSound->setVolume(70.0f * slowMoVolumeMultiplier);
                        rollSound->play();
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::Escape) {
                    state = GameState::Paused;
                    if (musicLoaded) bgMusic.pause();
                    if (heliLoaded && background.isHelicopterActive()) heliSound->pause();
                    if (droneSoundActive && droneSound) droneSound->pause();
                }
            } else if (state == GameState::Paused) {
                if (keyPress->code == sf::Keyboard::Key::Escape) {
                    state = GameState::Playing;
                    if (musicLoaded) bgMusic.play();
                    if (heliLoaded && background.isHelicopterActive()) heliSound->play();
                    if (droneSoundActive && droneSound) droneSound->play();
                }
            }
        }
    }
}

void Game::triggerCarJumpScene3() {
    if (carSceneState != CarSceneState::Inactive) return; // already running, ignore
    carSceneIncludesVan = true;
    // Spawn X is computed ONCE here and reused for every vehicle in
    // this scene run (Car/Jeep/Van all anchor to carSceneSpawnX) --
    // this is what keeps the scene's gap consistent with whatever a
    // normal car's spawn point currently is, instead of the fixed
    // 900.0f used before.
    carSceneSpawnX = computeSpawnX();
    // No clear-path delay anymore: Car spawns immediately, same
    // moment a normal single-obstacle spawn would have happened.
    obstacles.emplace_back(ObstacleType::Car, carSceneSpawnX, sceneCarHeight);
    carSceneState = CarSceneState::WaitingCar;
    carSceneTimer = 0.0f;
}

void Game::triggerCarJumpScene2() {
    if (carSceneState != CarSceneState::Inactive) return; // already running, ignore
    carSceneIncludesVan = false;
    carSceneSpawnX = computeSpawnX();
    obstacles.emplace_back(ObstacleType::Car, carSceneSpawnX, sceneCarHeight);
    carSceneState = CarSceneState::WaitingCar;
    carSceneTimer = 0.0f;
}

void Game::updateCarJumpScene(float deltaTime, float obstacleSpeed) {
    carSceneTimer += deltaTime;

    // Every hand-off in this chain (Car->Jeep, Jeep->Van, and the
    // final exit back to normal spawning) waits on the same real
    // position-check, isDownlaneClearFor().

    const float CAR_WIDTH = 627.0f;
    const float JEEP_WIDTH = 900.0f;

    switch (carSceneState) {
        case CarSceneState::WaitingCar: {
            float gapTime = (CAR_WIDTH + 210.0f) / obstacleSpeed;
            if (carSceneTimer >= gapTime) {
                obstacles.emplace_back(ObstacleType::Jeep, carSceneSpawnX, 0.0f); // 0.0f defers to per-variant jeepTargetHeights; ground line now comes from jeepGroundYs automatically
                carSceneState = CarSceneState::WaitingJeep;
                carSceneTimer = 0.0f;
            }
            break;
        }

        case CarSceneState::WaitingJeep: {
            float gapTime = (JEEP_WIDTH + 170.0f) / obstacleSpeed;
            if (carSceneTimer >= gapTime) {
                if (carSceneIncludesVan) {
                    // 3-vehicle chain: Van comes next.
                    obstacles.emplace_back(ObstacleType::DownlaneVan, carSceneSpawnX, 0.0f); // 0.0f defers to per-variant vanTargetHeights; ground line now comes from vanGroundYs automatically
                    carSceneState = CarSceneState::WaitingVan;
                    carSceneTimer = 0.0f;
                } else {
                    // 2-vehicle chain: Jeep was the last one, so wait
                    // for the lane to actually clear before resuming
                    // normal spawning (WaitingClear below), same as
                    // every other hand-off in the chain.
                    carSceneState = CarSceneState::WaitingClear;
                    carSceneTimer = 0.0f;
                }
            }
            break;
        }

        case CarSceneState::WaitingVan:
            // Van's spawned: 3-vehicle sequence complete. Wait for
            // the lane to actually clear (WaitingClear below) instead
            // of resuming normal spawning the instant it appears.
            carSceneState = CarSceneState::WaitingClear;
            carSceneTimer = 0.0f;
            break;

        case CarSceneState::WaitingClear: {
            // Final hand-off back to normal spawning: verifies the
            // just-passed Jeep/Van has actually cleared enough of the
            // screen (isDownlaneClearFor()), same check drone uses,
            // rather than trusting a time estimate. Cars own the
            // downlane: every other spawn path checks clearance
            // against them, not the other way around.
            if (isDownlaneClearFor(sceneClearScreenFraction)) {
                carSceneState = CarSceneState::Inactive;
                carSceneCooldownTimer = 10.0f;
                // spawnTimer/spawnInterval both zeroed so the next
                // normal car fires on the very next update() tick --
                // the clearance check above already confirmed the lane
                // is genuinely clear, no need to wait out anything else.
                spawnTimer = 0.0f;
                spawnInterval = 0.0f;
            }
            break;
        }

        case CarSceneState::Inactive:
        default:
            break;
    }
}

float Game::computeSpawnX() const {
    // SPAWN DISTANCE FAIRNESS: scale spawn distance along with current
    // speed, so the TIME the player has to react to an incoming
    // obstacle stays roughly constant even as the game speeds up --
    // otherwise faster speed alone would shrink reaction time.
    const float baseSpawnX = 900.0f;
    return baseSpawnX * (currentGroundSpeed / GROUND_SPEED);
}

void Game::spawnObstacle() {
    // Chain, Barricade, and TCan are not spawned: they don't fit a
    // highway with nothing that ever stops. Tire is also excluded from
    // normal spawning here (kept for spawnTire()). Jeep and
    // DownlaneVan are scene-exclusive (triggerCarJumpScene2/3 below) --
    // at normal random-spawn size they can't be jumped correctly, so
    // only the scripted scene spawns them, at their own heights.
    ObstacleType type = ObstacleType::Car;
    obstacles.emplace_back(type, computeSpawnX());
}

bool Game::isDownlaneClearFor(float clearScreenFraction) const {
    // "Clear" means every currently-active Car/Jeep/DownlaneVan has
    // already traveled at least clearScreenFraction of the way from
    // spawn to the exit edge (or is fully off-screen already): i.e.
    // only the last (1 - clearScreenFraction) sliver of the screen,
    // right at the exit edge, is allowed to still have a vehicle in
    // it. Drone itself and any non-vehicle types are ignored: this
    // is only about not overlapping a still-visible car/jeep/van.
    // Shared by Drone (isDownlaneClearForDrone()) and the car-jump
    // scene's exit-back-to-normal-spawning check: each just passes
    // its own required fraction.
    const float totalWidth = VIEW_RIGHT_EDGE - VIEW_LEFT_EDGE;
    const float clearThreshold = VIEW_LEFT_EDGE + totalWidth * (1.0f - clearScreenFraction);

    for (const auto& obs : obstacles) {
        ObstacleType t = obs.getType();
        if (t != ObstacleType::Car && t != ObstacleType::Jeep && t != ObstacleType::DownlaneVan && t != ObstacleType::ThrowingCar && t != ObstacleType::Tire) continue;

        sf::FloatRect b = obs.getBounds();
        float rightEdge = b.position.x + b.size.x;
        if (rightEdge > clearThreshold) return false; // still occupies more than the allowed near-exit sliver
    }
    return true;
}

bool Game::isDownlaneClearForDrone() const {
    return isDownlaneClearFor(droneClearScreenFraction);
}

void Game::spawnDrone() {
    // Spawns from the same off-screen-right point every Car/Jeep/Van
    // uses, just flying instead of driving. Doesn't scale with speed
    // the way spawnObstacle()'s car spawn distance does: the drone's
    // own closing-speed knob (Obstacle.cpp) already handles pacing.
    float spawnX = 900.0f;
    obstacles.emplace_back(ObstacleType::Drone, spawnX);

    // Block normal Car spawning for a few seconds so the drone has the
    // screen mostly to itself right after appearing.
    dronePostSpawnCarBlockTimer = dronePostSpawnCarBlockSeconds;

    // Sound starts at the moment the drone actually exists, tied to
    // the real spawn rather than the earlier arm/countdown cue.
    if (droneLoaded && !droneSoundActive) {
        droneSound.emplace(droneBuffer);
        droneSound->setLooping(true);
        droneSound->setVolume(droneSoundVolume); // full volume: the sound file has its own fade in/out baked in
        droneSound->play();
        droneSoundActive = true;
        droneHasAppeared = false;
        droneOffscreenTimer = 0.0f;
        droneSoundElapsed = 0.0f;
    }
}

void Game::spawnTire() {
    // Same idea as spawnDrone() above: rolling instead of flying.
    float spawnX = 900.0f;
    obstacles.emplace_back(ObstacleType::Tire, spawnX);

    tirePostSpawnCarBlockTimer = tirePostSpawnCarBlockSeconds;

    // Marks the tire as occupying the lane for its WHOLE time on
    // screen, not just its ~1s pre-spawn countdown: Drone/Box check
    // this now (see update()) so they can't spawn while a tire is
    // still actually rolling across. Cleared once the tire leaves the
    // screen and is removed (see the pop_front() block below).
    tireActive = true;
}

void Game::spawnBoxThrow() {
    // Must spawn past VIEW_RIGHT_EDGE (900) to be genuinely off-screen,
    // otherwise it pops into view mid-air. boxSpawnEdgeMargin controls
    // how far past the edge it starts; since the box travels at a
    // constant BOX_SPEED, a bigger margin just makes the flight
    // proportionally longer without changing its shape.
    float boxSpawnX = VIEW_RIGHT_EDGE + boxSpawnEdgeMargin;

    // Growth scaling: BOX_SPEED stays as-is at run start (growthFraction
    // = 0), and grows by +30% by the time the world hits its speed cap
    //: same growth-fraction formula the other obstacles use, computed
    // independently here since spawnBoxThrow() doesn't have access to
    // update()'s local obstacleSpeed variable (see the boxLandX/duration
    // comment below for the same reasoning).
    const float obstacleSpeedBaseForBox = 750.0f;
    const float obstacleSpeedGrowthPassThroughForBox = 0.15f;
    const float obstacleSpeedCapForBox = 800.0f;
    float currentObstacleSpeedForBox = std::min(
        obstacleSpeedBaseForBox + (currentGroundSpeed - GROUND_SPEED) * obstacleSpeedGrowthPassThroughForBox,
        obstacleSpeedCapForBox
    );
    // Derived from the same numbers maxGroundSpeed uses, so it stays
    // in sync if that growth range is tuned further.
    const float maxGroundSpeedGrowthRange = 160.0f; // must match the +160.0f in maxGroundSpeed above
    float boxGrowthFraction = std::clamp((currentObstacleSpeedForBox - obstacleSpeedBaseForBox) / (maxGroundSpeedGrowthRange * obstacleSpeedGrowthPassThroughForBox), 0.0f, 1.0f);
    float effectiveBoxSpeed = BOX_SPEED * (1.0f + 0.30f * boxGrowthFraction); // +30% by the speed cap; base BOX_SPEED untouched at run start

    // Same formula as Obstacle.cpp's Box constructor (boxFlightDuration
    // = distance / effectiveBoxSpeed): computed independently here from the
    // same launch/target points, so both stay in sync without needing
    // to pass anything back out of the Obstacle after it's created.
    float boxLandX = VIEW_LEFT_EDGE + (boxLandTargetScreenPercent / 100.0f) * (VIEW_RIGHT_EDGE - VIEW_LEFT_EDGE);
    boxThrowActualFlightDuration = std::abs(boxSpawnX - boxLandX) / effectiveBoxSpeed;

    // Fixed forehead target: boxThrowTargetForeheadY never changes
    // per-throw, and playerCenterX is read once right here, not
    // tracked afterward. See the ADJUSTMENT KNOBS comment in Game.h
    // for how boxThrowTargetForeheadY was derived.
    obstacles.emplace_back(ObstacleType::Box, boxSpawnX, 0.0f, 0.0f, boxLandX, boxThrowTargetForeheadY, effectiveBoxSpeed);

    boxThrowActive = true;
    boxThrowElapsedTimer = 0.0f;
}

void Game::updateSlowMo(float realDeltaTime) {
    // Smoothstep (3t^2 - 2t^3): zero velocity at both ends, so the
    // ramps ease in and out instead of moving at a constant rate the
    // whole way. Used identically for both RampDown and RampUp.
    auto smoothstep = [](float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    };

    switch (slowMoPhase) {
        case SlowMoPhase::Inactive:
            worldTimeScale = 1.0f;
            break;

        case SlowMoPhase::PreDelay:
            // Nothing visible yet: just counting down the gap between
            // his foot leaving the van and the effect actually kicking in.
            worldTimeScale = 1.0f;
            slowMoTimer += realDeltaTime;
            if (slowMoTimer >= slowMoPreDelay) {
                slowMoPhase = SlowMoPhase::RampDown;
                slowMoTimer = 0.0f;
                // Sound fires right here: the exact instant the world
                // visibly starts slowing, not at jump liftoff.
                if (slowMoLoaded) {
                    slowMoSound.emplace(slowMoBuffer);
                    slowMoSound->play();
                }
            }
            break;

        case SlowMoPhase::RampDown: {
            slowMoTimer += realDeltaTime;
            float t = smoothstep(slowMoTimer / slowMoRampDownDuration);
            worldTimeScale = 1.0f - t * (1.0f - slowMoMinScale); // 1.0 -> slowMoMinScale
            if (slowMoTimer >= slowMoRampDownDuration) {
                slowMoPhase = SlowMoPhase::Hold;
                slowMoTimer = 0.0f;
                worldTimeScale = slowMoMinScale;
            }
            break;
        }

        case SlowMoPhase::Hold:
            worldTimeScale = slowMoMinScale;
            slowMoTimer += realDeltaTime;
            if (slowMoTimer >= slowMoHoldDuration) {
                slowMoPhase = SlowMoPhase::RampUp;
                slowMoTimer = 0.0f;
            }
            break;

        case SlowMoPhase::RampUp: {
            slowMoTimer += realDeltaTime;
            float t = smoothstep(slowMoTimer / slowMoRampUpDuration);
            worldTimeScale = slowMoMinScale + t * (1.0f - slowMoMinScale); // slowMoMinScale -> 1.0, continuously easing toward normal the whole way
            if (slowMoTimer >= slowMoRampUpDuration) {
                slowMoPhase = SlowMoPhase::Inactive;
                slowMoTimer = 0.0f;
                worldTimeScale = 1.0f;
            }
            break;
        }
    }

    // Every gameplay sound (except the slow-mo whoosh itself) should
    // track worldTimeScale directly: pitch = worldTimeScale gives
    // the classic "everything plays back slower and lower" bullet-time
    // effect, matching however slow the world currently is, not just
    // a fixed value while the effect is running. Volume eases from
    // 1.0 (normal) down to 0.5 (50% quieter) at max slow-motion,
    // along exactly the same curve worldTimeScale is already
    // following: so it fades in/out smoothly with the ramps instead
    // of snapping.
    slowMoPitch = worldTimeScale;
    float t = (worldTimeScale - slowMoMinScale) / (1.0f - slowMoMinScale); // 0 at max slow-mo, 1 at normal
    slowMoVolumeMultiplier = 0.5f + 0.5f * t;
}

void Game::updateLandableObstacles() {
    const float normalGround = player.getGroundY(); // reads the real value: no separate copy to fall out of sync
    float effectiveGround = normalGround;
    bool nowStandingOnVan = false; // set true below only while actually resting on a DownlaneVan's walk-free surface
    bool nowStandingOnCar = false; // same idea, for Car/ThrowingCar's walk-free surface
    bool nowStandingOnJeep = false; // same idea, for Jeep's walk-free surface

    sf::FloatRect pBounds = player.getBounds();
    float playerCenterX = pBounds.position.x + pBounds.size.x / 2.0f;
    float playerBottomY = pBounds.position.y + pBounds.size.y;

    const float landingTolerance = 20.0f; // shared by Car types

    for (auto& obs : obstacles) {
        if (obs.getType() == ObstacleType::Car) {
            // Car's walkable surface is 4 sloped segments instead of
            // one flat line. Where segment X-ranges overlap near a
            // seam, picks whichever valid segment's height is closest
            // to the player's current feet position, avoiding flips
            // between segment heights when segments are near each other.
            std::vector<RotatedRect> segments = obs.getCarWalkFreeLines();
            bool onASegment = false;
            float bestTopY = 0.0f;
            float bestDiff = std::numeric_limits<float>::max();

            for (const auto& seg : segments) {
                std::optional<float> topY = getTopEdgeYAtX(seg, playerCenterX);
                if (!topY.has_value()) continue;
                if (playerBottomY > *topY + landingTolerance) continue;

                float diff = std::abs(playerBottomY - *topY);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestTopY = *topY;
                    onASegment = true;
                }
            }

            if (onASegment) {
                effectiveGround = bestTopY;
                nowStandingOnCar = true;
            } else {
                // Checked separately (not one combined ||) so we know
                // WHICH side was hit: front (nose) knocks the player
                // forward instead of the default backward, see
                // triggerDeath(). Front checked first since if both
                // somehow overlapped at once, the nose is the one he
                // visually ran into.
                if (pBounds.findIntersection(obs.getCarDeathLine()).has_value()) {
                    triggerDeath(&obs, true);
                    return;
                }
                if (pBounds.findIntersection(obs.getCarBackDeathLine()).has_value()) {
                    triggerDeath(&obs, false);
                    return;
                }
            }
        }
        else if (obs.getType() == ObstacleType::ThrowingCar) {
            // Identical logic to Car above: just reads from the
            // ThrowingCar getters instead.
            std::vector<RotatedRect> segments = obs.getThrowingCarWalkFreeLines();
            bool onASegment = false;
            float bestTopY = 0.0f;
            float bestDiff = std::numeric_limits<float>::max();

            for (const auto& seg : segments) {
                std::optional<float> topY = getTopEdgeYAtX(seg, playerCenterX);
                if (!topY.has_value()) continue;
                if (playerBottomY > *topY + landingTolerance) continue;

                float diff = std::abs(playerBottomY - *topY);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestTopY = *topY;
                    onASegment = true;
                }
            }

            if (onASegment) {
                effectiveGround = bestTopY;
                nowStandingOnCar = true;
            } else {
                if (pBounds.findIntersection(obs.getThrowingCarDeathLine()).has_value()) {
                    triggerDeath(&obs, true);
                    return;
                }
                if (pBounds.findIntersection(obs.getThrowingCarBackDeathLine()).has_value()) {
                    triggerDeath(&obs, false);
                    return;
                }
            }
        }
        else if (obs.getType() == ObstacleType::Jeep) {
            // Same "closest valid segment" landing logic as Car, now
            // shared since Jeep also has multiple sloped walk-free
            // segments. Death zones are rotated too (not axis-aligned),
            // so they use the same SAT-based checkCollision() the
            // walk-free segments use, instead of FloatRect intersection.
            std::vector<RotatedRect> segments = obs.getJeepWalkFreeLines();
            bool onASegment = false;
            float bestTopY = 0.0f;
            float bestDiff = std::numeric_limits<float>::max();

            for (const auto& seg : segments) {
                std::optional<float> topY = getTopEdgeYAtX(seg, playerCenterX);
                if (!topY.has_value()) continue;
                if (playerBottomY > *topY + landingTolerance) continue;

                float diff = std::abs(playerBottomY - *topY);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestTopY = *topY;
                    onASegment = true;
                }
            }

            if (onASegment) {
                effectiveGround = bestTopY;
                nowStandingOnJeep = true;
            } else {
                // Jeep has no separate front/back getter like Car does --
                // just a flat list of death zones. Same "front = higher
                // world-X than the vehicle's own center" rule as Car's
                // nose/trunk convention (see getCarDeathLine()) applies
                // here too, so whichever specific zone was actually
                // touched decides the knockback direction, not just
                // "any zone hit = generic death" like before.
                bool hitDeathZone = false;
                bool zoneIsFront = false;
                float bodyCenterX = obs.getBounds().position.x + obs.getBounds().size.x / 2.0f;
                for (const auto& zone : obs.getJeepDeathZones()) {
                    if (checkCollision(pBounds, zone)) {
                        hitDeathZone = true;
                        zoneIsFront = zone.center.x > bodyCenterX;
                        break;
                    }
                }
                if (hitDeathZone) {
                    triggerDeath(&obs, zoneIsFront);
                    return;
                }
            }
        }
        else if (obs.getType() == ObstacleType::DownlaneVan) {
            std::vector<RotatedRect> segments = obs.getVanWalkFreeLines();
            bool onASegment = false;
            float bestTopY = 0.0f;
            float bestDiff = std::numeric_limits<float>::max();

            for (const auto& seg : segments) {
                std::optional<float> topY = getTopEdgeYAtX(seg, playerCenterX);
                if (!topY.has_value()) continue;
                if (playerBottomY > *topY + landingTolerance) continue;

                float diff = std::abs(playerBottomY - *topY);
                if (diff < bestDiff) {
                    bestDiff = diff;
                    bestTopY = *topY;
                    onASegment = true;
                }
            }

            if (onASegment) {
                effectiveGround = bestTopY;
                nowStandingOnVan = true;
            } else {
                // Same per-zone front/back detection as Jeep above.
                bool hitDeathZone = false;
                bool zoneIsFront = false;
                float bodyCenterX = obs.getBounds().position.x + obs.getBounds().size.x / 2.0f;
                for (const auto& zone : obs.getVanDeathZones()) {
                    if (checkCollision(pBounds, zone)) {
                        hitDeathZone = true;
                        zoneIsFront = zone.center.x > bodyCenterX;
                        break;
                    }
                }
                if (hitDeathZone) {
                    triggerDeath(&obs, zoneIsFront);
                    return;
                }
            }
        }
    }

    player.setGroundLevel(effectiveGround);
    player.setStandingOnVan(nowStandingOnVan);
    player.setStandingOnCar(nowStandingOnCar);
    player.setStandingOnJeep(nowStandingOnJeep);
}

void Game::triggerDeath(const Obstacle* killer, bool hitFromFront) {
    state = GameState::Dying;

    // ---- DEATH KNOCKBACK BACKSTOP / FRONTSTOP ----
    // Default death knockback is backward (-x): moves the player AWAY
    // from whatever killed him, which is correct for every death type
    // except one: getting hit on a vehicle's FRONT (nose) side. Since
    // the nose is always the higher-X edge of the vehicle (see
    // getCarDeathLine()/getCarBackDeathLine(): front = right/high-X,
    // back = left/low-X, and the Jeep/Van zone check below uses the
    // same rule), the vehicle's body extends further in -x from a
    // front-hit point. Knocking backward from there sends the player
    // straight through the body he was just standing in front of --
    // hence hitFromFront flips the knockback to +x (forward, away from
    // the body, into open road) instead.
    //
    // Either direction can still be blocked early by a second obstacle
    // sitting close by in that direction (can't fly through it, or off
    // the back/front edge of one he's standing on): same idea as
    // before, just mirrored for the forward case, with a small visual
    // gap added so he stops just short of it instead of flush against it.
    //
    // `killer`, the specific obstacle whose collision triggered this
    // death, is skipped entirely in both directions. Backward: it's
    // always touching the player at the moment of death, so its own
    // trailing edge sits right at his position and would clamp the
    // knockback to near zero. Forward: the whole point of a front-hit
    // is to send him past the vehicle's nose into the open road ahead,
    // so the vehicle he's escaping shouldn't be able to stop that.
    sf::FloatRect pBoundsAtDeath = player.getBounds();
    float playerCenterX = pBoundsAtDeath.position.x + pBoundsAtDeath.size.x / 2.0f;
    constexpr float noLimitBack = -100000.0f;
    constexpr float noLimitFront = 100000.0f;
    constexpr float stopGap = 15.0f; // visual gap left in front of whatever obstacle stopped the knockback early

    float stopX = hitFromFront ? noLimitFront : noLimitBack;
    bool foundStop = false;

    auto considerX = [&](float edgeX) {
        if (hitFromFront) {
            if (edgeX >= playerCenterX && edgeX < stopX) { stopX = edgeX; foundStop = true; }
        } else {
            if (edgeX <= playerCenterX && edgeX > stopX) { stopX = edgeX; foundStop = true; }
        }
    };
    auto considerRotatedZones = [&](const std::vector<RotatedRect>& zones) {
        for (const auto& zone : zones) {
            for (const auto& corner : zone.getCorners()) {
                considerX(corner.x);
            }
        }
    };

    for (const auto& obs : obstacles) {
        if (&obs == killer) continue;

        if (obs.getType() == ObstacleType::Car) {
            considerX(obs.getCarBackDeathLine().position.x);
            considerX(obs.getCarDeathLine().position.x);
        }
        else if (obs.getType() == ObstacleType::Jeep) {
            considerRotatedZones(obs.getJeepDeathZones());
            considerRotatedZones(obs.getJeepWalkFreeLines());
        }
        else if (obs.getType() == ObstacleType::DownlaneVan) {
            considerRotatedZones(obs.getVanDeathZones());
            considerRotatedZones(obs.getVanWalkFreeLines());
        }
    }

    // Leave a small gap in front of whichever edge stopped the
    // knockback, so the player visibly stops just short of it instead
    // of ending up flush against it. Only applies when a real obstacle
    // was actually found: the ±100000 "no limit" sentinels are left
    // untouched so a death with nothing nearby still travels its full
    // normal distance.
    if (foundStop) {
        stopX += hitFromFront ? -stopGap : stopGap;
    }

    player.die(stopX, hitFromFront);

    // Tire/Box/Drone get their own distinct impact sound; everything
    // else (Car, Jeep, Van, ThrowingCar, etc.) uses the generic hitSound.
    ObstacleType killerType = killer->getType();
    std::optional<sf::Sound>* deathSound = &hitSound;
    bool deathSoundLoaded = hitLoaded;
    if (killerType == ObstacleType::Tire && tireHitLoaded) {
        deathSound = &tireHitSound;
        deathSoundLoaded = true;
    } else if (killerType == ObstacleType::Box && boxHitLoaded) {
        deathSound = &boxHitSound;
        deathSoundLoaded = true;
    } else if (killerType == ObstacleType::Drone && droneHitLoaded) {
        deathSound = &droneHitSound;
        deathSoundLoaded = true;
    }
    if (deathSoundLoaded) {
        (*deathSound)->setPitch(slowMoPitch);
        (*deathSound)->setVolume(80.0f * slowMoVolumeMultiplier);
        (*deathSound)->play();
    }

    if (musicLoaded) bgMusic.stop();
    if (heliLoaded) heliSound->stop();
    for (auto& o : obstacles) o.pauseSounds();
    background.pauseUpLaneSounds();
    if (droneSound) { droneSound->stop(); droneSound.reset(); }
    droneSoundActive = false;
    droneHasAppeared = false;
    droneOffscreenTimer = 0.0f;
    droneSoundElapsed = 0.0f;
    droneSpawnCountdown = -1.0f;
    tireSpawnCountdown = -1.0f;

    // Cancel any in-flight slow-motion too: otherwise it could carry
    // a leftover slow worldTimeScale into the death animation itself,
    // or still be ramping when the next run starts.
    slowMoPhase = SlowMoPhase::Inactive;
    slowMoTimer = 0.0f;
    worldTimeScale = 1.0f;
    if (slowMoSound) { slowMoSound->stop(); slowMoSound.reset(); }

    // update() returns early for Dying/GameOver, before the milestone-
    // blink code (Playing branch only) would reset it, so it's ended
    // here to avoid scoreText staying frozen mid-blink on death.
    milestoneBlinking = false;
    milestoneBlinkTimer = 0.0f;
    if (fontLoaded) {
        scoreText->setFillColor(GOLD_COLOR);
        scoreText->setScale({ 1.0f, 1.0f });
    }
}

void Game::playRandomMenuSong() {
    // Picks a track that is NEITHER the one that just played NOR the
    // one before that: with 10 songs, this always leaves at least 7
    // valid choices, so it can't loop forever. -1 (no history yet)
    // never matches a real index, so the first two calls just work
    // without needing special-casing.
    int chosen;
    do {
        chosen = std::rand() % NUM_MENU_SONGS;
    } while (chosen == menuMusicLastPlayed || chosen == menuMusicSecondLastPlayed);

    std::string path = "assets/MenuSong" + std::to_string(chosen + 1) + ".mp3";
    if (menuMusic.openFromFile(path)) {
        menuMusic.play();
    } else {
        std::cerr << "Warning: could not load " << path << ": menu music track skipped.\n";
    }

    menuMusicSecondLastPlayed = menuMusicLastPlayed;
    menuMusicLastPlayed = chosen;
}

void Game::checkCollisions() {
    // Was chain's death-only check: now handles Tire's circle
    // collision instead (also death-only, no landing).
    sf::FloatRect playerBounds = player.getBounds();

    for (auto& obs : obstacles) {
        if (obs.getType() != ObstacleType::Tire) continue;

        bool hit = checkCollision(playerBounds, obs.getTireCollisionCircle());

        if (hit) {
            triggerDeath(&obs);
            break;
        }
    }

    // Drone: same death-only pattern as Tire above (no landing
    // surface at all), now checking 4 small dots (one per corner)
    // instead of a single rotated rect: see getDroneDotZones() in
    // Obstacle.cpp/.h.
    for (auto& obs : obstacles) {
        if (obs.getType() != ObstacleType::Drone) continue;

        bool hit = false;
        for (const Circle& dot : obs.getDroneDotZones()) {
            if (checkCollision(playerBounds, dot)) { hit = true; break; }
        }

        if (hit) {
            triggerDeath(&obs);
            break;
        }
    }

    // Box: same death-only pattern as Tire/Drone above, now checking
    // 4 small dots (one per corner) instead of a single rotated rect --
    // see getBoxDotZones() in Obstacle.cpp/.h. Runs whether the box is
    // still mid-flight or already grounded and drifting: it's
    // lethal either way.
    for (auto& obs : obstacles) {
        if (obs.getType() != ObstacleType::Box) continue;

        bool hit = false;
        for (const Circle& dot : obs.getBoxDotZones()) {
            if (checkCollision(playerBounds, dot)) { hit = true; break; }
        }

        if (hit) {
            triggerDeath(&obs);
            break;
        }
    }
}

void Game::startGame() {
    // Menu music stops the moment Play is pressed: coming back to
    // the menu later (if that ever happens) starts a fresh random
    // pick, not a resume.
    menuMusic.stop();
    menuMusicStarted = false;
    menuMusicStartTimer = 0.0f;
    menuMusicWaitingGap = false;
    menuMusicGapTimer = 0.0f;

    obstacles.clear();
    spawnTimer = 0.0f;
    spawnInterval = 2.0f;

    // Drone spawn state must reset too: otherwise a leftover pending
    // flag or mid-count timer from the previous run carries into the
    // new one.
    droneSpawnTimer = 0.0f;
    droneSpawnPending = false;
    dronePostSpawnCarBlockTimer = 0.0f;
    droneSpawnCountdown = -1.0f;

    // Tire spawn state must reset too, same reasoning.
    tireSpawnTimer = 0.0f;
    tireSpawnPending = false;
    tirePostSpawnCarBlockTimer = 0.0f;
    tireSpawnCountdown = -1.0f;
    tireActive = false;

    // Box-throw spawn state must reset too, same reasoning.
    boxThrowSpawnTimer = 0.0f;
    boxThrowSpawnPending = false;
    boxThrowActive = false;
    boxThrowElapsedTimer = 0.0f;
    boxThrowPostSpawnCarBlockTimer = 0.0f;

    // Slow-motion state must reset too: otherwise a leftover
    // in-progress ramp (or a stuck slow worldTimeScale) from the
    // previous run would carry into the new one.
    slowMoPhase = SlowMoPhase::Inactive;
    slowMoTimer = 0.0f;
    worldTimeScale = 1.0f;
    if (slowMoSound) { slowMoSound->stop(); slowMoSound.reset(); }

    // Drone sound must be stopped/reset too: otherwise a leftover
    // hum from the previous run keeps playing (or its state carries
    // over) into the new one.
    if (droneSound) { droneSound->stop(); droneSound.reset(); }
    droneSoundActive = false;
    droneHasAppeared = false;
    droneOffscreenTimer = 0.0f;
    droneSoundElapsed = 0.0f;

    // Car-jump scene state must be reset here too: otherwise dying
    // mid-scene (e.g. after Car/Jeep spawned but before Van) leaves
    // carSceneState sitting at WaitingJeep/WaitingVan. obstacles.clear()
    // above wipes the actual sprites, but the state machine doesn't
    // know that, so on the next run it fires straight to spawning
    // Van/Jeep with nothing before it the moment its leftover timer
    // elapses: an unjumpable, out-of-nowhere obstacle.
    carSceneState = CarSceneState::Inactive;
    carSceneTimer = 0.0f;
    carSceneIncludesVan = true;
    carSceneCooldownTimer = 0.0f;

    currentGroundSpeed = GROUND_SPEED;
    speedSurvivalTime = 0.0f;

    player.reset();
    player.beginRun();
    background.reset();
    scoreManager.reset();

    milestoneBlinking = false;
    milestoneBlinkTimer = 0.0f;
    if (fontLoaded) {
        scoreText->setFillColor(GOLD_COLOR);
        scoreText->setScale({ 1.0f, 1.0f });
    }

    state = GameState::Playing;

    if (musicLoaded) {
        bgMusic.play();
    }

    // Post-intro agent cameo: fires every time (after the intro and
    // after every restart). Reuses the intro's agent sprite slots
    // since the cutscene is already over by this point.
    postIntroAgentActive = true;
    postIntroAgentTimer = 0.0f;
    agentPos = { postIntroAgentStartX, player.getGroundY() };
    agentFrame = 0;
    agentAnimTimer = 0.0f;
}

void Game::startIntro() {
    // Same menu-music-stop as startGame(): this is now the actual
    // entry point when Play is pressed, startGame() itself runs later
    // at the end of the cutscene as the handoff into real gameplay.
    menuMusic.stop();
    menuMusicStarted = false;
    menuMusicStartTimer = 0.0f;
    menuMusicWaitingGap = false;
    menuMusicGapTimer = 0.0f;

    state = GameState::Intro;
    introPhase = IntroPhase::TitleCard;
    introPhaseTimer = 0.0f;

    introPlayerPos = { introPlayerSpawnX, introGroundY };
    introPlayerFrame = 0;
    introPlayerAnimTimer = 0.0f;

    agentPos = { introAgentSpawnX, introGroundY };
    agentFrame = 0;
    agentAnimTimer = 0.0f;
    introAgentSpawned = false;

    introHeyStopTriggered = false;
    introBeepsStarted = false;
    introBeepCount = 0;
    introTitleSoundPlayed = false;

    introCurrentBg = 1;
    introFlickerTimer = 0.0f;

    introFlashAlpha = 0.0f;
    introFlashRisingUp = false;
    introFlashFadingOut = false;
    introFlashTimer = 0.0f;
    introBassDropPlayed = false;

    // Title sound plays mid-fade-in, once the fade crosses
    // introTitleSoundTriggerFraction (see updateIntro's TitleCard case).
}

void Game::updateIntro(float deltaTime) {
    introPhaseTimer += deltaTime;

    const float runFrameDuration = 0.08f; // seconds per frame: player and agent's run cycle

    switch (introPhase) {
        case IntroPhase::TitleCard: {
            // No movement, no animation: just the title art + sound
            // fading in, holding, then fading out on black.
            // Background1 is drawn underneath the WHOLE time by
            // renderIntro() so the fade-out reveals it as a proper
            // crossfade instead of a hard cut.
            float fadeInProgress = std::min(introPhaseTimer / introTitleFadeInDuration, 1.0f);

            // Title sound fires once the fade-in crosses
            // introTitleSoundTriggerFraction, not at time 0.
            if (!introTitleSoundPlayed && fadeInProgress >= introTitleSoundTriggerFraction) {
                introTitleSoundPlayed = true;
                if (introTitleSoundLoaded) introTitleSound->play();
            }

            if (introPhaseTimer >= introTitleDuration) {
                introPhase = IntroPhase::Chase;
                introPhaseTimer = 0.0f;
                introPlayerFrame = 0;
                introPlayerAnimTimer = 0.0f;

                // Burst-out sound fires the SAME frame he appears --
                // he's already running, no slide pose anymore.
                if (introWhooshLoaded) introWhooshSound->play();
            }
            break;
        }

        case IntroPhase::Chase: {
            introPlayerPos.x += introRunSpeed * deltaTime;

            introPlayerAnimTimer += deltaTime;
            if (introPlayerAnimTimer >= runFrameDuration) {
                introPlayerAnimTimer = 0.0f;
                introPlayerFrame = (introPlayerFrame + 1) % 8;
            }

            // "Hey stop!" is position-triggered, not timer-triggered:
            // fires once the player has run introHeyStopTriggerDistance
            // px past his own spawn point (introPlayerSpawnX), so
            // lowering it isn't swallowed by how far off-screen he
            // starts. Background stays calm (bg1) through the shout,
            // only switching once the shout finishes (see below).
            if (!introHeyStopTriggered && introPlayerPos.x >= introPlayerSpawnX + introHeyStopTriggerDistance) {
                introHeyStopTriggered = true;
                if (introHeyStopLoaded) introHeyStopSound->play();
                introFlickerTimer = 0.0f; // repurposed below as a countdown to the shout's real finish
            }

            // Wait out the shout's actual duration (read from the file
            // in init(), not guessed) before anything alarm-related
            // happens.
            if (introHeyStopTriggered && !introBeepsStarted) {
                introFlickerTimer += deltaTime;
                if (introFlickerTimer >= introHeyStopDurationSeconds) {
                    // Background switch and the first beep happen together here.
                    introBeepsStarted = true;
                    introFlickerTimer = 0.0f;
                    introCurrentBg = 3;
                    if (introBeepBeepLoaded) introBeepBeepSound->play();
                    introBeepCount = 1;
                }
            } else if (introBeepsStarted) {
                introFlickerTimer += deltaTime;
                // Single fixed beat for both on/off. The beep sound
                // plays whenever bg switches to 3, and can get cut
                // short if it's longer than introFlickerBeatDuration.
                if (introFlickerTimer >= introFlickerBeatDuration) {
                    introFlickerTimer = 0.0f;
                    introCurrentBg = (introCurrentBg == 2) ? 3 : 2;
                    if (introCurrentBg == 3) {
                        if (introBeepBeepLoaded) introBeepBeepSound->play();
                        introBeepCount++;
                        // Agent bursts out on the 2nd beep, not the 3rd.
                        if (introBeepCount == 2 && !introAgentSpawned) {
                            introAgentSpawned = true;
                            agentPos.x = introAgentSpawnX;
                            agentFrame = 0;
                            agentAnimTimer = 0.0f;
                        }
                    }
                }
            }

            // Agent stays completely inert (not drawn, not moved) until
            // introAgentSpawned flips true above.
            if (introAgentSpawned) {
                float desiredAgentX = introPlayerPos.x - introAgentGapX;
                if (agentPos.x < desiredAgentX) {
                    agentPos.x += introAgentCatchUpSpeed * deltaTime;
                    if (agentPos.x > desiredAgentX) agentPos.x = desiredAgentX;
                } else {
                    agentPos.x = desiredAgentX;
                }
                agentAnimTimer += deltaTime;
                if (agentAnimTimer >= runFrameDuration) {
                    agentAnimTimer = 0.0f;
                    agentFrame = (agentFrame + 1) % 8;
                }
            }

            if (introPlayerPos.x >= introGarageDoorX) {
                introPlayerPos.x = introGarageDoorX; // snap, no overshoot
                introPhase = IntroPhase::ExitTouch;
                introPhaseTimer = 0.0f;
                introFlashTimer = 0.0f;

                // Only the whip fires right here, at the trigger.
                // Bass drop comes LATER, partway through the flash --
                // see ExitTouch below.
                if (introWhipLoaded) introWhipSound->play();
                introFlashRisingUp = true;
            }
            break;
        }

        case IntroPhase::ExitTouch: {
            // Both frozen: no position updates at all here, just the
            // flash ramping on its own timer and, once it completes,
            // the handoff.
            if (introFlashRisingUp) {
                introFlashTimer += deltaTime;
                float progress = introFlashTimer / introFlashRiseDuration; // 0 -> 1, this IS the flash timer
                introFlashAlpha = 255.0f * std::min(progress, 1.0f);

                // Bass drop fires partway through the rise (NOT with
                // the whip): right before the screen goes fully
                // white, then the screen clears immediately after.
                if (!introBassDropPlayed && progress >= introBassDropTriggerFraction) {
                    introBassDropPlayed = true;
                    if (introBassDropLoaded) introBassDropSound->play();
                }

                if (progress >= 1.0f) {
                    introFlashAlpha = 255.0f;
                    introFlashRisingUp = false;
                    introFlashTimer = 0.0f;

                    // ---- HANDOFF ---- fully white now, safe to swap to
                    // real gameplay underneath it: startGame() resets
                    // everything properly (obstacles, spawn timers, the
                    // real player's position/state), exactly the same
                    // as a normal Play press always has.
                    startGame();
                    introFlashFadingOut = true;
                }
            }
            break;
        }
    }
}

void Game::renderIntro() {
    // During TitleCard, the real background is completely HIDDEN (not
    // drawn at all, not even under a black overlay) until the fade-out
    // actually begins: keeps the warehouse a genuine surprise instead
    // of leaking through a semi-transparent black during the fade-in.
    // Once the fade-out starts, it's drawn and revealed as the black clears.
    bool titleCardHidingBackground = false;
    if (introPhase == IntroPhase::TitleCard) {
        float solidEndCheck = introTitleDuration - introTitleFadeOutDuration;
        titleCardHidingBackground = introPhaseTimer <= solidEndCheck;
    }

    if (introBgLoaded && !titleCardHidingBackground) {
        sf::Sprite* bg = (introCurrentBg == 1) ? &(*introBgSprite1) : (introCurrentBg == 2) ? &(*introBgSprite2) : &(*introBgSprite3);

        // Fit-to-800px-width baseline, then scale/position adjustments
        // on top (introBgScaleMultiplier/introBgPosX/Y, Game.h).
        sf::Vector2u bgTexSize = bg->getTexture().getSize();
        float bgScale = (800.0f / static_cast<float>(bgTexSize.x)) * introBgScaleMultiplier;
        bg->setScale({bgScale, bgScale});
        bg->setOrigin({0.0f, 0.0f});
        bg->setPosition({introBgPosX, introBgPosY});

        window.draw(*bg);
    }

    if (introPhase == IntroPhase::TitleCard) {
        // Full window size/view, NOT the zoomed gameplay view: this is
        // what makes the black actually cover the whole screen
        // regardless of any zoom setting, instead of leaving edges
        // exposed.
        window.setView(window.getDefaultView());
        float winW = static_cast<float>(window.getSize().x);
        float winH = static_cast<float>(window.getSize().y);

        // Eased fade-in (first introTitleFadeInDuration seconds), then
        // solid hold, then eased fade-out (last introTitleFadeOutDuration
        // seconds): smoothstep instead of linear, for a smoother,
        // more cinematic ramp than a flat crossfade. During fade-in and
        // the hold, titleCardHidingBackground is true above, so there's
        // nothing behind this black to leak through regardless.
        float alpha = 255.0f;
        float titleDriftY = 0.0f; // extra upward drift, fade-out only
        float solidStart = introTitleFadeInDuration;
        float solidEnd = introTitleDuration - introTitleFadeOutDuration;

        auto smoothstep = [](float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); };

        if (introPhaseTimer < solidStart) {
            float t = smoothstep(introPhaseTimer / introTitleFadeInDuration);
            alpha = 255.0f * t;
        } else if (introPhaseTimer > solidEnd) {
            float t = smoothstep((introPhaseTimer - solidEnd) / introTitleFadeOutDuration);
            alpha = 255.0f * (1.0f - t);
            titleDriftY = -introTitleFadeOutRiseY * t; // lifts away as it fades
        }

        sf::RectangleShape blackOverlay({winW, winH});
        blackOverlay.setPosition({0.0f, 0.0f});
        blackOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha)));
        window.draw(blackOverlay);

        if (introTitleTexLoaded && introTitleSprite.has_value()) {
            sf::Vector2u texSize = introTitleTexture.getSize();
            // Fraction-based anchor (Game.h): stays correctly placed
            // regardless of window size, instead of a fixed pixel offset.
            float scale = (winW / static_cast<float>(texSize.x)) * introTitleScaleMultiplier;
            introTitleSprite->setScale({scale, scale});
            introTitleSprite->setOrigin({texSize.x / 2.0f, texSize.y / 2.0f});
            introTitleSprite->setPosition({
                winW * introTitleAnchorXFraction,
                winH * introTitleAnchorYFraction + titleDriftY
            });
            introTitleSprite->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
            window.draw(*introTitleSprite);
        }
        // Skip-intro prompt draws in every intro phase, TitleCard
        // included: default view is already active here, so no
        // extra setView() needed for this one.
        if (fontLoaded && skipIntroText.has_value()) {
            window.draw(*skipIntroText);
        }

        return; // nothing else to draw during the title card
    }

    const float introCharTargetHeight = 125.0f; // matches the real Player's own height

    if (introPlayerTexturesLoaded && introPlayerSprite.has_value()) {
        const sf::Texture& tex = introPlayerRunTextures[introPlayerFrame];
        introPlayerSprite->setTexture(tex, true);
        sf::Vector2u texSize = tex.getSize();
        float scale = introCharTargetHeight / texSize.y;
        introPlayerSprite->setScale({scale, scale});
        introPlayerSprite->setOrigin({texSize.x / 2.0f, static_cast<float>(texSize.y)}); // bottom-center: feet sit at introPlayerPos
        introPlayerSprite->setPosition(introPlayerPos);
        window.draw(*introPlayerSprite);
    }

    if (introAgentSpawned && agentTexturesLoaded && agentSprite.has_value()) {
        const sf::Texture& tex = agentRunTextures[agentFrame];
        agentSprite->setTexture(tex, true);
        sf::Vector2u texSize = tex.getSize();
        // Agent drawn introAgentSizeMultiplier bigger than the player --
        // more visual presence as the chaser.
        float scale = (introCharTargetHeight * introAgentSizeMultiplier) / texSize.y;
        agentSprite->setScale({scale, scale});
        agentSprite->setOrigin({texSize.x / 2.0f, static_cast<float>(texSize.y)});
        agentSprite->setPosition(agentPos);
        window.draw(*agentSprite);
    }

    // gameView (zoomed) is active for Chase/ExitTouch (set once per
    // frame in render() before dispatching here): switch back to the
    // window's default view so the skip prompt sits at a fixed,
    // unzoomed screen position, same idea as the HUD text elsewhere.
    if (fontLoaded && skipIntroText.has_value()) {
        window.setView(window.getDefaultView());
        window.draw(*skipIntroText);
    }
}

void Game::restartGame() {
    startGame();
}

void Game::returnToMenu() {
    // Mirrors startGame()'s menu-music reset, just in the opposite
    // direction: coming back to the menu means a fresh 5s delay and
    // a new random track, not resuming whatever was left over from
    // last time (there won't be anything left over anyway, since
    // startGame() already fully stops menuMusic when Play is pressed,
    // but resetting the timers here is what actually makes the 5s
    // delay fire again instead of instantly picking a track).
    menuMusic.stop();
    menuMusicStarted = false;
    menuMusicStartTimer = 0.0f;
    menuMusicWaitingGap = false;
    menuMusicGapTimer = 0.0f;

    state = GameState::Menu;
}

void Game::update(float deltaTime) {
    // Intro flash fade-out: runs regardless of state, since it needs
    // to keep decaying during the first moments of real gameplay too,
    // not just while state == Intro (see introFlashFadingOut, Game.h).
    if (introFlashFadingOut) {
        introFlashAlpha -= (255.0f / introFlashFadeDuration) * deltaTime;
        if (introFlashAlpha <= 0.0f) {
            introFlashAlpha = 0.0f;
            introFlashFadingOut = false;
        }
    }

    if (state == GameState::Menu) {
        // Menu music: first track starts 5s after the menu appears.
        // After that, each track is followed by a 3s silence before
        // the next one picks up: not an instant back-to-back
        // transition. See playRandomMenuSong() for the "no repeats
        // within the last 2" logic.
        if (!menuMusicStarted) {
            menuMusicStartTimer += deltaTime;
            if (menuMusicStartTimer >= 5.0f) {
                menuMusicStarted = true;
                playRandomMenuSong();
            }
        } else if (menuMusicWaitingGap) {
            menuMusicGapTimer += deltaTime;
            if (menuMusicGapTimer >= 3.0f) {
                menuMusicWaitingGap = false;
                menuMusicGapTimer = 0.0f;
                playRandomMenuSong();
            }
        } else if (menuMusic.getStatus() == sf::Music::Status::Stopped) {
            // Track just finished: start the 3s gap instead of
            // jumping straight to the next song.
            menuMusicWaitingGap = true;
            menuMusicGapTimer = 0.0f;
        }

        updateMenuParticles(deltaTime);
        return;
    }

    if (state == GameState::Intro) {
        updateIntro(deltaTime);
        return;
    }

    if (state == GameState::Paused) {
        return; // fully frozen, waiting on resume/quit input
    }

    if (state == GameState::GameOver) {
        return;
    }

    if (state == GameState::Dying) {
        player.update(deltaTime);

        if (player.consumeDeathFinalFrameEvent() && bodyfallLoaded) {
            bodyfallSound->play();
        }

        if (player.isDeathAnimationFinished()) {
            scoreManager.checkAndUpdateHighScore();
            state = GameState::GameOver;

            if (fontLoaded) {
                float cx = window.getSize().x / 2.0f;
                const float gap = 8.0f; // space between a label and its value

                // Centers a label+value pair as ONE unit on a given
                // line: label first (white), value right after it
                // (gold): since the two texts' combined width
                // changes with the digit count, this has to be
                // recomputed every time the score actually changes.
                auto layoutPair = [&](sf::Text& label, sf::Text& value, float lineY) {
                    sf::FloatRect lBounds = label.getLocalBounds();
                    sf::FloatRect vBounds = value.getLocalBounds();
                    float totalW = lBounds.size.x + gap + vBounds.size.x;
                    float startX = cx - totalW / 2.0f;

                    label.setOrigin({ 0.0f, lBounds.position.y + lBounds.size.y / 2.0f });
                    label.setPosition({ startX, lineY });

                    value.setOrigin({ 0.0f, vBounds.position.y + vBounds.size.y / 2.0f });
                    value.setPosition({ startX + lBounds.size.x + gap, lineY });
                };

                scoreValueText->setString(std::to_string(scoreManager.getScore()));
                highScoreValueText->setString(std::to_string(scoreManager.getHighScore()));

                layoutPair(*scoreLabelText, *scoreValueText, gameOverScoreLineY);
                layoutPair(*highScoreLabelText, *highScoreValueText, gameOverHighScoreLineY);
            }
        }
        return;
    }

    // state == Playing

    // VAN FLIP SLOW-MOTION: check for a fresh trigger from Player::jump()
    // (only starts a new one if nothing's already running), then advance
    // the timeline using REAL time: it must NOT be scaled by its own
    // output, or it'd take forever to ramp back to normal.
    if (player.consumeVanFlipSlowMoEvent() && slowMoPhase == SlowMoPhase::Inactive) {
        slowMoPhase = SlowMoPhase::PreDelay;
        slowMoTimer = 0.0f;
    }
    updateSlowMo(deltaTime);
    // Push the current slow-motion audio values out to every car's
    // engine/horn sound: downlane (Obstacle's static, shared by all
    // instances) and up-lane (Background's own single-instance copy).
    Obstacle::setSlowMoAudio(slowMoPitch, slowMoVolumeMultiplier);
    background.setSlowMoAudio(slowMoPitch, slowMoVolumeMultiplier);
    // Scaled time is what actually gets handed to the systems that
    // should visibly slow down (world scroll, obstacles, player) --
    // everything else below (score, spawn timers, UI blink) stays on
    // the real, unscaled deltaTime on purpose.
    float scaledDeltaTime = deltaTime * worldTimeScale;

    // GAME SPEED SCALING: grows the longer you survive, capped at
    // maxGroundSpeed. Passed into Background/Obstacle every frame
    // instead of them reading a fixed constant, so everything already
    // on screen speeds up together, not just newly spawned obstacles.
    speedSurvivalTime += deltaTime;
    const float speedGrowthRate = 0.2648f;    // was 0.5296: halved to match halved growth range, still reaches max at ~score 100,000
    const float maxGroundSpeed = GROUND_SPEED + 160.0f; // was 870 flat: halved the growth amount, not the raw cap (raw halved would be below base speed)
    currentGroundSpeed = std::min(GROUND_SPEED + speedGrowthRate * speedSurvivalTime, maxGroundSpeed);

    // Run animation speeds up slightly as the game speeds up over
    // time, matching the increasing pace visually. Kept subtle (small
    // growth rate, modest cap) so it's cosmetic only, not frantic.
    const float runAnimGrowthRate = 0.15f;     // multiplier gained per second survived
    const float maxRunAnimMultiplier = 1.4f;   // cap: 1.4 = 40% faster animation, max
    float runAnimMultiplier = std::min(1.0f + runAnimGrowthRate * speedSurvivalTime, maxRunAnimMultiplier);
    player.setRunAnimSpeedMultiplier(runAnimMultiplier);

    background.setScore(scoreManager.getScore());
    background.update(scaledDeltaTime, currentGroundSpeed, true); // true = Playing right now

    // Helicopter flyby sound: starts the instant it spawns. Playing at
    // a fixed volume since the clip itself already fades in/out to
    // match distance: applying our own live fade on top would
    // double up against that.
    if (background.consumeHelicopterSpawnEvent() && heliLoaded) {
        heliSound->play();
    }

    // Keeps the helicopter sound's playback rate in sync with how much
    // faster the helicopter is actually moving this run, so the sound
    // doesn't keep going after the helicopter has really passed by.
    // heliPitchGrowthDampening: 1.0 = pitch grows exactly as fast as
    // groundSpeed; lower it if the pitch swing sounds too extreme.
    // Both pitch and volume also get the slow-mo multiplier layered
    // on top, on top of whatever this was already doing.
    if (heliLoaded && background.isHelicopterActive()) {
        const float heliPitchGrowthDampening = 0.5f;
        float heliSpeedRatio = currentGroundSpeed / GROUND_SPEED;
        heliSound->setPitch((1.0f + (heliSpeedRatio - 1.0f) * heliPitchGrowthDampening) * slowMoPitch);
        heliSound->setVolume(100.0f * slowMoVolumeMultiplier);
    }
    scoreManager.update(deltaTime);

    // ---- MILESTONE: score just crossed the previous high score ----
    if (scoreManager.checkMilestoneCrossing()) {
        if (milestoneLoaded) {
            milestoneSound->setPitch(slowMoPitch);
            milestoneSound->setVolume(100.0f * slowMoVolumeMultiplier);
            milestoneSound->play();
        }
        milestoneBlinking = true;
        milestoneBlinkTimer = 0.0f;
        milestoneColorTimer = 0.0f;
        milestoneColorIndex = 0;
    }

    if (milestoneBlinking && fontLoaded) {
        milestoneBlinkTimer += deltaTime;
        milestoneColorTimer += deltaTime;

        // Cycle through milestoneColors every 0.15s for a rainbow-ish flash.
        if (milestoneColorTimer >= 0.15f) {
            milestoneColorTimer = 0.0f;
            milestoneColorIndex = (milestoneColorIndex + 1) % milestoneColors.size();
        }
        scoreText->setFillColor(milestoneColors[milestoneColorIndex]);

        // Pulse the text size smoothly between 1.0x and 2.0x over the
        // whole 5-second window using a sine wave.
        float pulse = 1.2f + 0.1f * std::sin(milestoneBlinkTimer * 6.0f);
        scoreText->setScale({ pulse, pulse });

        const float milestoneBlinkDuration = 5.0f;
        if (milestoneBlinkTimer >= milestoneBlinkDuration) {
            milestoneBlinking = false;
            scoreText->setFillColor(GOLD_COLOR);
            scoreText->setScale({ 1.0f, 1.0f });
        }
    }

    if (fontLoaded) {
        scoreText->setString("Score: " + std::to_string(scoreManager.getScore()));
    }

    updateLandableObstacles();
    if (state == GameState::Dying) {
        player.update(deltaTime); // just died this frame: real time, not scaled by any leftover slowmo
        return;
    }

    player.update(scaledDeltaTime);

    // Landing sound: only for a real jump landing (front-flip or
    // classic), never for roll: see consumeJumpLandedEvent() in Player.
    if (player.consumeJumpLandedEvent() && landingLoaded) {
        landingSound->setPitch(slowMoPitch);
        landingSound->setVolume(75.0f * slowMoVolumeMultiplier);
        landingSound->play();
    }

    // Footstep sounds: only fires while actually Running (Player.cpp
    // gates the events on that), so never during jump/roll/slide.
    // Picks the Car set or Ground set based on isStandingOnVehicle()
    // (true for Car, Van, OR Jeep: one combined flag).
    bool onVehicleForFootstep = player.isStandingOnVehicle();
    if (player.consumeFootstepLeftEvent()) {
        if (onVehicleForFootstep && carFootstepLeftLoaded) {
            carFootstepLeftSound->play();
        } else if (!onVehicleForFootstep && groundFootstepLeftLoaded) {
            groundFootstepLeftSound->play();
        }
    }
    if (player.consumeFootstepRightEvent()) {
        if (onVehicleForFootstep && carFootstepRightLoaded) {
            carFootstepRightSound->play();
        } else if (!onVehicleForFootstep && groundFootstepRightLoaded) {
            groundFootstepRightSound->play();
        }
    }

    // ---- Post-intro agent cameo: purely cosmetic, no collision.
    // Holds in place until postIntroAgentSlowdownStart (just animating,
    // since the player himself doesn't move screen-X either: the
    // world scrolls under both of them). After that, an EASED curve
    // (fast at first, visibly decelerating toward the end: ease-out,
    // not a flat constant speed) carries him from his hold position to
    // a safely off-screen point, timed to land exactly at
    // postIntroAgentGoneBy. This is what makes him read as tiring out
    // and fading back rather than just popping off mid-screen.
    if (postIntroAgentActive) {
        postIntroAgentTimer += deltaTime;

        const float agentRunFrameDuration = 0.08f;
        agentAnimTimer += deltaTime;
        if (agentAnimTimer >= agentRunFrameDuration) {
            agentAnimTimer = 0.0f;
            agentFrame = (agentFrame + 1) % 8;
        }

        if (postIntroAgentTimer > postIntroAgentSlowdownStart) {
            float slowdownDuration = postIntroAgentGoneBy - postIntroAgentSlowdownStart;
            float t = std::clamp((postIntroAgentTimer - postIntroAgentSlowdownStart) / slowdownDuration, 0.0f, 1.0f);
            float eased = t * (2.0f - t); // ease-out: fast start, visibly slowing as t -> 1

            float offScreenX = VIEW_LEFT_EDGE - 80.0f; // comfortably past the edge, not just barely off
            agentPos.x = postIntroAgentStartX + (offScreenX - postIntroAgentStartX) * eased;
        }

        if (postIntroAgentTimer >= postIntroAgentGoneBy) {
            postIntroAgentActive = false;
        }
    }

    // ---- CAR SPEED: deliberately NOT tied 1:1 to currentGroundSpeed.
    // Cars drive the SAME direction as the player (see carOwnForwardSpeed
    // in Obstacle.cpp): what matters for fairness is the CLOSING
    // speed, not the raw number here. If this grew all the way to
    // currentGroundSpeed's cap, cars would close the gap so fast at
    // high score they'd barely look like they're moving before
    // they're already on top of the player. So this only lets a SMALL,
    // capped fraction of the world's growth pass through: enough
    // that it's not perfectly static forever, not enough to meaningfully
    // change the reaction-time math. The visual "getting faster" feel
    // comes from the wheel-spin boost in Obstacle.cpp instead, which
    // can be dialed up freely without touching fairness at all.
    const float obstacleSpeedBase = 750.0f;
    const float obstacleSpeedGrowthPassThrough = 0.15f; // only 15% of the world's growth passes through
    const float obstacleSpeedCap = 800.0f; // hard ceiling regardless of score
    float obstacleSpeed = std::min(
        obstacleSpeedBase + (currentGroundSpeed - GROUND_SPEED) * obstacleSpeedGrowthPassThrough,
        obstacleSpeedCap
    );
    // Cooldown ticks down regardless of scene state: once a scene
    // finishes it's set to 10.0f, and no new scene can roll until
    // this reaches 0.
    if (carSceneCooldownTimer > 0.0f) {
        carSceneCooldownTimer -= deltaTime;
        if (carSceneCooldownTimer < 0.0f) carSceneCooldownTimer = 0.0f;
    }

    // ---- DRONE/BOX/TIRE START THRESHOLD ----
    // None of these three start trying to spawn until the score
    // crosses 8000: keeps the opening stretch of every run as plain
    // Car/Jeep/Van traffic, and shortens how much of the run these
    // three end up occupying overall. Their own interval timers
    // (droneSpawnTimer etc.) also don't START accumulating until this
    // point, so the first one doesn't fire the instant the threshold
    // is crossed: it still has to wait out its own normal interval
    // first, same as any other spawn.
    if (scoreManager.getScore() >= 5000.0f) {

    // ---- DRONE SPAWN TIMER ----
    // Runs independently of the Car spawnTimer below. Once the
    // interval elapses, it goes "pending" and just waits (checked
    // every frame) until isDownlaneClearForDrone() says the screen's
    // clear enough AND no regular Car spawn is imminent (see the
    // headroom guard below). Only then does a 1-second countdown
    // start (droneSpawnCountdown). The sound itself starts later, in
    // spawnDrone(), once the drone actually exists: NOT here at arm
    // time, since starting it this early gave away every spawn check
    // even on attempts that got cancelled by the backstop below before
    // a drone ever appeared.
    //
    // Tire/Box guards below still check droneSpawnCountdown < 0.0f
    // (not just !droneSoundActive) so they correctly treat the drone as
    // "claimed" for its whole 1s countdown, not just once it's on
    // screen: this part is unchanged. Same reasoning in reverse for
    // Drone/Box checking tireSpawnCountdown.
    droneSpawnTimer += deltaTime;
    if (!droneSpawnPending && droneSpawnTimer >= droneSpawnInterval) {
        droneSpawnPending = true;
    }
    if (droneSpawnPending && droneSpawnCountdown < 0.0f && isDownlaneClearForDrone() && !boxThrowActive && !tireActive && tireSpawnCountdown < 0.0f) {
        // FAIRNESS: without this, Drone (checked first, every frame)
        // would win every single tie against Box/Tire whenever more
        // than one is simultaneously ready to go: which becomes the
        // NORMAL case at high speed once clear windows get rare enough
        // that all three end up waiting on the same ones. Doesn't touch
        // the mutual-exclusion guards above (still fully in effect,
        // still what actually prevents two of these from committing in
        // the same frame): just adds a fair random chance to skip
        // this frame's turn if someone else is also currently ready,
        // instead of Drone automatically taking it every time.
        bool boxAlsoReady = boxThrowSpawnPending && !boxThrowActive && isDownlaneClearFor(boxThrowClearScreenFraction) && !droneSoundActive && droneSpawnCountdown < 0.0f && !tireActive && tireSpawnCountdown < 0.0f;
        bool tireAlsoReady = tireSpawnPending && tireSpawnCountdown < 0.0f && isDownlaneClearFor(tireClearScreenFraction) && !boxThrowActive && !droneSoundActive && droneSpawnCountdown < 0.0f;
        int contenders = 1 + (boxAlsoReady ? 1 : 0) + (tireAlsoReady ? 1 : 0);

        if (contenders == 1 || (std::rand() % contenders) == 0) {
        // Extra guard before committing: isDownlaneClearForDrone() only
        // checks EXISTING obstacles, not whether a brand new Car is
        // about to spawn from the regular timer below. Without this
        // check, the cue could start right as a fresh Car spawns a
        // moment later and immediately re-occupies the lane: which
        // sounded like the drone sound flickering on and off. Only
        // commit once there's enough headroom before the next
        // scheduled Car spawn that one landing mid-countdown is very
        // unlikely.
        float timeToNextCarSpawn = spawnInterval - spawnTimer;
        bool regularSpawningActive = (carSceneState == CarSceneState::Inactive) && (dronePostSpawnCarBlockTimer <= 0.0f);
        bool carSpawnImminent = regularSpawningActive && (timeToNextCarSpawn < 0.4f);

        if (!carSpawnImminent) {
            droneSpawnCountdown = 1.0f;
        }
        }
    }
    if (droneSpawnCountdown >= 0.0f) {
        // With the headroom guard above, the lane should stay clear
        // for the full 1s in the vast majority of cases. This is just
        // a backstop for the rare case something still interrupts it
        // (e.g. a scripted car-jump scene kicking in mid-countdown).
        if (isDownlaneClearForDrone()) {
            droneSpawnCountdown -= deltaTime;
            if (droneSpawnCountdown <= 0.0f) {
                spawnDrone();
                droneSpawnPending = false;
                droneSpawnTimer = 0.0f;
                droneSpawnCountdown = -1.0f;
                // Re-roll the next interval +/-5s so drones don't arrive on a
                // perfectly predictable metronome.
                droneSpawnInterval = 12.0f + (std::rand() % 9); // 18-28s
            }
        } else {
            droneSpawnCountdown = -1.0f;
            // Countdown cancelled before the drone appeared. Safety net
            // so a cancelled countdown can never leave the sound
            // playing with nothing on screen.
            if (droneSoundActive && !droneHasAppeared) {
                if (droneSound) { droneSound->stop(); droneSound.reset(); }
                droneSoundActive = false;
                droneOffscreenTimer = 0.0f;
                droneSoundElapsed = 0.0f;
            }
        }
    }

    // ---- BOX THROW SPAWN TIMER ----
    // Same timer/pending/clear-screen shape as Drone above, without a
    // sound-cue countdown. Once triggered, spawnBoxThrow() sets
    // boxThrowActive=true, which also pauses normal Car spawning (see
    // the `&& !boxThrowActive` guard above) so a Car can't spawn into
    // the same lane before the paired ThrowingCar does.
    boxThrowSpawnTimer += deltaTime;
    if (!boxThrowSpawnPending && boxThrowSpawnTimer >= boxThrowSpawnInterval) {
        boxThrowSpawnPending = true;
    }
    if (boxThrowSpawnPending && !boxThrowActive && isDownlaneClearFor(boxThrowClearScreenFraction) && !droneSoundActive && droneSpawnCountdown < 0.0f && !tireActive && tireSpawnCountdown < 0.0f) {
        // Same reasoning as Drone's block above: fair random
        // chance to skip this frame's turn if Drone/Tire are also
        // currently ready, instead of Box always winning just because
        // it happens to be checked before Tire in the source.
        bool droneAlsoReady = droneSpawnPending && droneSpawnCountdown < 0.0f && isDownlaneClearForDrone() && !boxThrowActive && !tireActive && tireSpawnCountdown < 0.0f;
        bool tireAlsoReady = tireSpawnPending && tireSpawnCountdown < 0.0f && isDownlaneClearFor(tireClearScreenFraction) && !boxThrowActive && !droneSoundActive && droneSpawnCountdown < 0.0f;
        int contenders = 1 + (droneAlsoReady ? 1 : 0) + (tireAlsoReady ? 1 : 0);

        if (contenders == 1 || (std::rand() % contenders) == 0) {
        spawnBoxThrow();
        boxThrowSpawnPending = false;
        boxThrowSpawnTimer = 0.0f;
        // Re-roll the next interval +/-5s, same reasoning as Drone's.
        boxThrowSpawnInterval = 24.0f + (std::rand() % 11); // 24-34s
        }
    }

    // Once the box has been in flight for boxThrowCarSpawnFraction of
    // its ACTUAL flight duration (computed per-throw in spawnBoxThrow(),
    // since flight time now depends on distance: see BOX_SPEED in
    // GameConfig.h), spawn its paired ThrowingCar from the normal far
    // spawn point. Base value is 0.2 (20%, see Game.h): fires early
    // in the box's flight, which is what makes it read as "thrown FROM
    // this car" despite the car spawning fresh off-screen.
    //
    // GROWTH MISMATCH COMPENSATION: Box's own speed grows +30% by the
    // real speed cap, but the ThrowingCar it's paired with only grows
    // ~6% (shares Car's normal closing-speed formula): so at high
    // speed the box visually outpaces its own source car more and
    // more, breaking the "thrown from this car" illusion. Compensates
    // by shrinking the spawn-trigger fraction slightly as speed climbs
    // (car spawns a bit EARLIER relative to the box's now-faster
    // flight, to make up for its own slower relative approach): ZERO
    // change at run start, ~18% reduction by max speed, same
    // start-at-zero growth-fraction shape as Box's own speed boost
    // above so this never applies at normal/low speed, only high.
    const float boxCarSpawnFractionCompensation = 0.182f; // derived from the real ratio between Box's +30% growth and Car's own ~6.3% growth at the current 160.0f speed range: recalculate if either growth number changes
    const float maxGroundSpeedGrowthRangeForBoxTiming = 160.0f; // must match the +160.0f in maxGroundSpeed above (this function has its own local copy since it's a different scope from spawnBoxThrow()'s)
    float boxGrowthFractionForTiming = std::clamp((currentGroundSpeed - GROUND_SPEED) / maxGroundSpeedGrowthRangeForBoxTiming, 0.0f, 1.0f);
    float effectiveBoxThrowCarSpawnFraction = boxThrowCarSpawnFraction * (1.0f - boxCarSpawnFractionCompensation * boxGrowthFractionForTiming);

    if (boxThrowActive) {
        boxThrowElapsedTimer += deltaTime;
        if (boxThrowElapsedTimer >= boxThrowActualFlightDuration * effectiveBoxThrowCarSpawnFraction) {
            obstacles.emplace_back(ObstacleType::ThrowingCar, computeSpawnX());
            boxThrowActive = false;
            // Blocks a normal Car from spawning the same frame, which
            // would land on top of the ThrowingCar just spawned above.
            // Same reasoning as dronePostSpawnCarBlockTimer below.
            boxThrowPostSpawnCarBlockTimer = boxThrowPostSpawnCarBlockSeconds;
        }
    }

    // ---- TIRE SPAWN TIMER: exact same shape as Drone's above,
    // rolling instead of flying. Mutual exclusion both ways with
    // Drone/Box-throw is already applied on their commit conditions
    // above (tireSpawnCountdown < 0.0f); this side checks the same
    // back at them.
    tireSpawnTimer += deltaTime;
    if (!tireSpawnPending && tireSpawnTimer >= tireSpawnInterval) {
        tireSpawnPending = true;
    }
    if (tireSpawnPending && tireSpawnCountdown < 0.0f && isDownlaneClearFor(tireClearScreenFraction) && !boxThrowActive && !droneSoundActive && droneSpawnCountdown < 0.0f) {
        // Same reasoning as Drone/Box's blocks above: fair
        // random chance to skip this frame's turn if Drone/Box are also
        // currently ready, instead of Tire always losing simply because
        // it's checked last of the three in the source.
        bool droneAlsoReady = droneSpawnPending && droneSpawnCountdown < 0.0f && isDownlaneClearForDrone() && !boxThrowActive && !tireActive && tireSpawnCountdown < 0.0f;
        bool boxAlsoReady = boxThrowSpawnPending && !boxThrowActive && isDownlaneClearFor(boxThrowClearScreenFraction) && !droneSoundActive && droneSpawnCountdown < 0.0f && !tireActive && tireSpawnCountdown < 0.0f;
        int contenders = 1 + (droneAlsoReady ? 1 : 0) + (boxAlsoReady ? 1 : 0);

        if (contenders == 1 || (std::rand() % contenders) == 0) {
        float timeToNextCarSpawn = spawnInterval - spawnTimer;
        bool regularSpawningActive = (carSceneState == CarSceneState::Inactive) && (tirePostSpawnCarBlockTimer <= 0.0f);
        bool carSpawnImminent = regularSpawningActive && (timeToNextCarSpawn < 0.4f);

        if (!carSpawnImminent) {
            tireSpawnCountdown = 1.0f;
        }
        }
    }
    if (tireSpawnCountdown >= 0.0f) {
        if (isDownlaneClearFor(tireClearScreenFraction)) {
            tireSpawnCountdown -= deltaTime;
            if (tireSpawnCountdown <= 0.0f) {
                spawnTire();
                tireSpawnPending = false;
                tireSpawnTimer = 0.0f;
                tireSpawnCountdown = -1.0f;
                tireSpawnInterval = 12.0f + (std::rand() % 9);
            }
        } else {
            tireSpawnCountdown = -1.0f;
        }
    }

    } // end of the score >= 8000 gate

    // Post-drone Car spawn block ticks down regardless of anything
    // else: while it's active, the normal spawnTimer below still
    // accumulates (so a Car spawn isn't just delayed by a fixed
    // amount, it triggers the instant the block clears if the timer's
    // already past its interval).
    if (dronePostSpawnCarBlockTimer > 0.0f) {
        dronePostSpawnCarBlockTimer -= scaledDeltaTime;
        if (dronePostSpawnCarBlockTimer < 0.0f) dronePostSpawnCarBlockTimer = 0.0f;
    }

    // Same idea, for the ThrowingCar that just spawned above.
    if (boxThrowPostSpawnCarBlockTimer > 0.0f) {
        boxThrowPostSpawnCarBlockTimer -= scaledDeltaTime;
        if (boxThrowPostSpawnCarBlockTimer < 0.0f) boxThrowPostSpawnCarBlockTimer = 0.0f;
    }

    if (tirePostSpawnCarBlockTimer > 0.0f) {
        tirePostSpawnCarBlockTimer -= scaledDeltaTime;
        if (tirePostSpawnCarBlockTimer < 0.0f) tirePostSpawnCarBlockTimer = 0.0f;
    }

    // Normal random spawning is completely paused while a scripted
    // car-jump scene is running: it drives its own spawns via
    // updateCarJumpScene() below instead.
    if (carSceneState == CarSceneState::Inactive) {
        spawnTimer += scaledDeltaTime;
        if (spawnTimer >= spawnInterval && dronePostSpawnCarBlockTimer <= 0.0f && !boxThrowActive && boxThrowPostSpawnCarBlockTimer <= 0.0f && tirePostSpawnCarBlockTimer <= 0.0f) {
            spawnTimer = 0.0f;
            // Random PIXEL gap (not a flat time range): converts to a
            // time interval using the car's CURRENT speed, so the actual
            // on-screen distance between cars stays in this range no
            // matter how fast/slow cars are moving right now. This is
            // what stops the gap from growing as the game speeds up.
            float targetGapPixels = 1100.0f + (std::rand() % 2100); // random 1100-3199px (max doubled from ~1600, min unchanged so nothing spawns too close)
            spawnInterval = targetGapPixels / obstacleSpeed;

            // ---- SCRIPTED SCENE ROLL: 15% chance INSTEAD OF a
            // normal spawn, only once score >= 5000, only if the 10s
            // cooldown from the last scene has fully expired. Randomly
            // picks the 3-vehicle or 2-vehicle chain each time. ----
            bool sceneEligible = (scoreManager.getScore() >= 5000.0f) && (carSceneCooldownTimer <= 0.0f);
            if (sceneEligible && (std::rand() % 100) < 15) {
                if (std::rand() % 2 == 0) triggerCarJumpScene3();
                else triggerCarJumpScene2();
            } else {
                spawnObstacle();
            }
        }
    } else {
        updateCarJumpScene(scaledDeltaTime, obstacleSpeed);
    }

    for (auto& obs : obstacles) {
        obs.update(scaledDeltaTime, obstacleSpeed);
    }

    // ---- DRONE SOUND: offscreen cutoff (no volume envelope here --
    // the sound file handles its own fade in/out) ----
    if (droneSoundActive && droneSound) {
        droneSound->setVolume(droneSoundVolume * slowMoVolumeMultiplier);
        droneSound->setPitch(slowMoPitch);

        Obstacle* activeDrone = nullptr;
        for (auto& obs : obstacles) {
            if (obs.getType() == ObstacleType::Drone) { activeDrone = &obs; break; }
        }

        if (activeDrone) {
            droneHasAppeared = true;
            if (activeDrone->isOffScreen()) droneOffscreenTimer += deltaTime;
            else droneOffscreenTimer = 0.0f;
        } else if (droneHasAppeared) {
            // Already appeared onscreen and is now gone from the
            // obstacle list (removed the instant it crossed the edge)
            //: keep counting toward the 0.5s cutoff from here.
            droneOffscreenTimer += deltaTime;
        }

        if (droneHasAppeared && droneOffscreenTimer >= 0.5f) {
            droneSound->stop();
            droneSound.reset();
            droneSoundActive = false;
            droneHasAppeared = false;
            droneOffscreenTimer = 0.0f;
            droneSoundElapsed = 0.0f;
        }
    }

    // pop_front instead of remove_if+erase: all downlane obstacles
    // share the same obstacleSpeed, so they never overtake each other
    //: the oldest (front) one always leaves the screen first. Using
    // remove_if+erase here would move-assign every SURVIVING car into
    // the gap left by a removed one, disturbing its live sf::Sound
    // engine/horn mid-playback. pop_front() is guaranteed by the
    // standard to never touch any other element in the deque.
    while (!obstacles.empty() && obstacles.front().canBeRemoved()) {
        // Tire's lane-block flag only covers its actual on-screen
        // lifetime: clear it right as the tire itself is the one
        // being removed, same "confirmed gone" moment droneSoundActive
        // uses for the drone.
        if (obstacles.front().getType() == ObstacleType::Tire) {
            tireActive = false;
        }
        obstacles.pop_front();
    }

    checkCollisions();
}

void Game::renderMenu() {
    // Entirely static UI now: no live gameplay scene behind it, so
    // this only ever needs the default (unzoomed) view. Switching
    // immediately, before drawing anything.
    window.setView(window.getDefaultView());

    if (menuBackgroundLoaded) {
        window.draw(*menuBackgroundSprite);
    }

    // Drawn behind the buttons/text but in front of the background --
    // ambient glow, not something that should interfere with reading
    // the UI.
    renderMenuParticles();

    if (menuPlayLoaded) {
        float baseScale = 145.0f / static_cast<float>(menuPlayTexture.getSize().x);
        float scale = playHovered ? baseScale * 1.05f : baseScale;
        menuPlaySprite->setScale({ scale, scale });
        menuPlaySprite->setColor(playHovered ? LAVENDER_HOVER_COLOR : sf::Color::White);
        window.draw(*menuPlaySprite);
    }

    if (menuAboutLoaded) {
        float baseScale = 145.0f / static_cast<float>(menuAboutTexture.getSize().x);
        float scale = aboutHovered ? baseScale * 1.05f : baseScale;
        menuAboutSprite->setScale({ scale, scale });
        menuAboutSprite->setColor(aboutHovered ? LAVENDER_HOVER_COLOR : sf::Color::White);
        window.draw(*menuAboutSprite);
    }

    if (menuQuitLoaded) {
        float baseScale = 145.0f / static_cast<float>(menuQuitTexture.getSize().x);
        float scale = quitHovered ? baseScale * 1.05f : baseScale;
        menuQuitSprite->setScale({ scale, scale });
        menuQuitSprite->setColor(quitHovered ? LAVENDER_HOVER_COLOR : sf::Color::White);
        window.draw(*menuQuitSprite);
    }

    if (fontLoaded) {
        std::string scoreStr = std::to_string(scoreManager.getHighScore());
        menuHighScoreText->setString("High Score: " + scoreStr);
        sf::FloatRect hBounds = menuHighScoreText->getLocalBounds();

        // Reserve room for 2 more digits than it currently has, so it
        // doesn't visibly creep further right (toward the laptop art)
        // the next time the score gains 2 digits. Measures the ACTUAL
        // width using the real font/size (not a guessed pixel number)
        // by building the same text with "00" appended and comparing.
        // Text is center-anchored, so half of that extra width is what
        // gets added to the right edge: shifting the anchor left by
        // that same half-amount keeps the right edge exactly where it
        // is right now even after 2 more digits appear.
        float leftShift = 23.0f;

        // buttonCenterX isn't in scope here (it's local to the resize
        // layout code above): recomputed the same way it's defined
        // there (0.221f * winW) so this stays in sync with it. Y is
        // left exactly as it already was (untouched, no scope issue)
        // by reading it back off the text itself rather than
        // recalculating: only X is being adjusted here.
        float winW = static_cast<float>(window.getSize().x);
        float buttonCenterX = 0.221f * winW;
        float currentY = menuHighScoreText->getPosition().y;

        menuHighScoreText->setOrigin({ hBounds.size.x / 2.0f, hBounds.size.y / 2.0f });
        menuHighScoreText->setPosition({ buttonCenterX - leftShift, currentY });
        window.draw(*menuHighScoreText);
    }

    // ---- About overlay: empty panel, no content yet ----
    if (showAboutOverlay) {
        sf::RectangleShape dim({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
        dim.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(dim);

        float panelW = 500.0f, panelH = 350.0f;
        sf::RectangleShape panel({ panelW, panelH });
        panel.setFillColor(sf::Color(15, 15, 30, 235));
        panel.setOutlineColor(sf::Color(106, 153, 211));
        panel.setOutlineThickness(2.0f);
        panel.setOrigin({ panelW / 2.0f, panelH / 2.0f });
        panel.setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f });
        window.draw(panel);

        if (fontLoaded && aboutTitleText.has_value() && aboutBodyText.has_value()) {
            float panelCenterX = window.getSize().x / 2.0f;
            float panelTopY = window.getSize().y / 2.0f - panelH / 2.0f;
            float padding = 30.0f;

            sf::FloatRect titleBounds = aboutTitleText->getLocalBounds();
            aboutTitleText->setOrigin({ titleBounds.size.x / 2.0f, 0.0f });
            aboutTitleText->setPosition({ panelCenterX, panelTopY + padding });
            window.draw(*aboutTitleText);

            // Body left-aligned, starting below the title with a gap.
            aboutBodyText->setOrigin({ 0.0f, 0.0f });
            aboutBodyText->setPosition({ panelCenterX - panelW / 2.0f + padding, panelTopY + padding + titleBounds.size.y + 25.0f });
            window.draw(*aboutBodyText);
        }
    }
}

void Game::renderGameplay() {
    background.draw(window);

    for (auto& obs : obstacles) {
        obs.draw(window);
    }

    player.draw(window);

    // Post-intro agent cameo: see the update-side logic in update()
    // for the timing. Same sprite/scale/origin convention as the intro.
    if (postIntroAgentActive && agentTexturesLoaded && agentSprite.has_value()) {
        const float agentCharTargetHeight = 125.0f; // matches the real Player's own height
        const sf::Texture& tex = agentRunTextures[agentFrame];
        agentSprite->setTexture(tex, true);
        sf::Vector2u texSize = tex.getSize();
        float scale = (agentCharTargetHeight * postIntroAgentSizeMultiplier) / texSize.y;
        agentSprite->setScale({scale, scale});
        agentSprite->setOrigin({texSize.x / 2.0f, static_cast<float>(texSize.y)});
        agentSprite->setPosition(agentPos);

        // Shadow first, underneath the body: same Shade.png/anchoring
        // convention as the real Player's shadow, but flat (no jump-
        // height shrink/fade) since this agent always stays on the
        // ground.
        if (agentShadeLoaded && agentShadowSprite.has_value()) {
            sf::Vector2u shadeTexSize = agentShadeTexture.getSize();
            if (shadeTexSize.x > 0) {
                sf::FloatRect agentBounds = agentSprite->getGlobalBounds();
                const float shadowWidthFrac = 0.55f; // same flat fraction the player's shadow uses
                float shadowWidth = agentBounds.size.x * shadowWidthFrac;
                float shadowScale = shadowWidth / shadeTexSize.x;
                agentShadowSprite->setScale({shadowScale, shadowScale});
                agentShadowSprite->setOrigin({shadeTexSize.x / 2.0f, shadeTexSize.y / 2.0f});
                agentShadowSprite->setColor(sf::Color(0, 0, 0, 255));
                agentShadowSprite->setPosition({
                    agentBounds.position.x + agentBounds.size.x / 2.0f,
                    agentPos.y - 5.0f
                });
                window.draw(*agentShadowSprite);
            }
        }

        window.draw(*agentSprite);
    }

    // Debug collision-line drawing (all types) removed: no longer needed.

    // HUD/UI below this point uses the window's default, unzoomed
    // view. gameView (set by render() before this function runs) is
    // wider than the window (the zoom-out effect), so anything drawn
    // under it maps fixed coordinates like scoreText's {20,20} to the
    // wrong spot. Switching to the default view here means every UI
    // element below (including in the Paused/Game Over screens) draws
    // in real window pixels and lines up with its hitbox rect.
    window.setView(window.getDefaultView());

    if (fontLoaded) {
        window.draw(*scoreText);
    }

    // ---- Pause icon (top-right): only while actively Playing ----
    if (state == GameState::Playing && pauseIconLoaded) {
        window.draw(*pauseIconSprite);
    }
}

void Game::renderPaused() {
    renderGameplay(); // frozen scene behind the pause overlay

    if (!fontLoaded) return;

    // Dim overlay
    sf::RectangleShape dim({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
    dim.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dim);

    window.draw(*pausedTitleText);

    resumeText->setFillColor(resumeHovered ? sf::Color::Yellow : sf::Color::White);
    pausedQuitText->setFillColor(pausedQuitHovered ? sf::Color::Yellow : sf::Color::White);
    window.draw(*resumeText);
    window.draw(*pausedQuitText);
}

void Game::renderGameOver() {
    renderGameplay(); // frozen scene behind the game-over overlay

    if (bustedLogoLoaded) {
        window.draw(*bustedLogoSprite);
    }

    if (restartTextureLoaded) {
        // Sprites can't change color like text via setFillColor, but
        // setColor() tints the whole texture by multiplying its
        // pixels against the given color: close enough to a
        // "hover glow" to match the buttons elsewhere. Combined with
        // the scale bump so hover is unmistakable, not subtle.
        float baseScale = 340.0f / static_cast<float>(restartTexture.getSize().x);
        float hoverScale = restartHovered ? baseScale * 1.05f : baseScale;
        restartSprite->setScale({ hoverScale, hoverScale });
        restartSprite->setColor(restartHovered ? LAVENDER_HOVER_COLOR : sf::Color::White);
        window.draw(*restartSprite);
    }

    if (gameOverMenuLoaded) {
        float restartRenderedHeight = 135.0f * (340.0f / static_cast<float>(restartTexture.getSize().x));
        float baseScale = restartRenderedHeight / static_cast<float>(gameOverMenuTexture.getSize().y);
        float hoverScale = gameOverMenuHovered ? baseScale * 1.05f : baseScale;
        gameOverMenuSprite->setScale({ hoverScale, hoverScale });
        gameOverMenuSprite->setColor(gameOverMenuHovered ? LAVENDER_HOVER_COLOR : sf::Color::White);
        window.draw(*gameOverMenuSprite);
    }

    if (gameOverQuitLoaded) {
        float restartRenderedHeight = 135.0f * (340.0f / static_cast<float>(restartTexture.getSize().x));
        float baseScale = restartRenderedHeight / static_cast<float>(gameOverQuitTexture.getSize().y);
        float hoverScale = gameOverQuitHovered ? baseScale * 1.05f : baseScale;
        gameOverQuitSprite->setScale({ hoverScale, hoverScale });
        gameOverQuitSprite->setColor(gameOverQuitHovered ? LAVENDER_HOVER_COLOR : sf::Color::White);
        window.draw(*gameOverQuitSprite);
    }

    if (fontLoaded) {
        window.draw(*scoreLabelText);
        window.draw(*scoreValueText);
        window.draw(*highScoreLabelText);
        window.draw(*highScoreValueText);
    }
}

void Game::render() {
    window.clear(sf::Color(20, 10, 40));

    // Re-applied every frame in case viewWidth/viewHeight get tuned at
    // runtime while testing. Applies to the world (background,
    // obstacles, player): HUD/UI elements switch back to the
    // window's default view partway through renderGameplay(), so
    // they're unaffected by this zoom. See the note there.
    gameView.setSize({viewWidth, viewHeight});
    gameView.setCenter({400.0f, 300.0f});
    window.setView(gameView);

    if (state == GameState::Menu) {
        renderMenu();
    } else if (state == GameState::Intro) {
        renderIntro();
    } else if (state == GameState::Paused) {
        renderPaused();
    } else if (state == GameState::GameOver) {
        renderGameOver();
    } else {
        renderGameplay(); // Playing or Dying
    }

    // ---- Intro white flash overlay: deliberately OUTSIDE the state
    // dispatch above, drawn regardless of state and gated only by its
    // own alpha, since it needs to keep fading during the first
    // moments of real GameState::Playing too (see introFlashFadingOut
    // in Game.h). Drawn in the window's default view (full 800x600,
    // not the zoomed gameView) so it always covers the whole screen
    // exactly, regardless of the current zoom/letterbox setup.
    if (introFlashAlpha > 0.0f) {
        window.setView(window.getDefaultView());
        sf::RectangleShape flashOverlay({static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)});
        flashOverlay.setPosition({0.0f, 0.0f});
        flashOverlay.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(introFlashAlpha)));
        window.draw(flashOverlay);
    }

    window.display();
}
