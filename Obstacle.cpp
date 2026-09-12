#include "Obstacle.h"
#include "GameConfig.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>

sf::Texture Obstacle::shadeTexture;
sf::Texture Obstacle::tireTexture;
std::vector<sf::Texture> Obstacle::carBodyTextures;
std::vector<float> Obstacle::carTargetHeights;
std::vector<float> Obstacle::carGroundYs;
std::vector<sf::Texture> Obstacle::carWheelTextures;
std::vector<Obstacle::CarWheelLayout> Obstacle::carWheelLayouts;
std::vector<Obstacle::CarCollisionLayout> Obstacle::carCollisionLayouts;
std::vector<sf::Texture> Obstacle::jeepBodyTextures;
std::vector<float> Obstacle::jeepTargetHeights;
std::vector<float> Obstacle::jeepGroundYs;
std::vector<sf::Texture> Obstacle::jeepWheelTextures;
std::vector<Obstacle::CarWheelLayout> Obstacle::jeepWheelLayouts;
std::vector<Obstacle::VehicleCollisionLayout> Obstacle::jeepCollisionLayouts;
std::vector<sf::Texture> Obstacle::vanBodyTextures;
std::vector<float> Obstacle::vanTargetHeights;
std::vector<float> Obstacle::vanGroundYs;
std::vector<sf::Texture> Obstacle::vanWheelTextures;
std::vector<Obstacle::CarWheelLayout> Obstacle::vanWheelLayouts;
std::vector<Obstacle::VehicleCollisionLayout> Obstacle::vanCollisionLayouts;
std::vector<sf::Texture> Obstacle::droneBodyTextures;
std::vector<float> Obstacle::droneTargetHeights;
std::vector<Obstacle::SegmentSpec> Obstacle::droneCollisionSpecs;
std::vector<Obstacle::DotCollisionSpec> Obstacle::droneDotSpecs;
std::vector<sf::Texture> Obstacle::throwingCarBodyTextures;
std::vector<sf::Texture> Obstacle::throwingCarWheelTextures;
std::vector<Obstacle::CarWheelLayout> Obstacle::throwingCarWheelLayouts;
std::vector<Obstacle::CarCollisionLayout> Obstacle::throwingCarCollisionLayouts;
std::vector<sf::Texture> Obstacle::boxTextures;
std::vector<Obstacle::SegmentSpec> Obstacle::boxCollisionSpecs;
std::vector<Obstacle::DotCollisionSpec> Obstacle::boxDotSpecs;
bool Obstacle::texturesLoaded = false;
int Obstacle::lastCarVariant = -1;
int Obstacle::lastJeepVariant = -1;
int Obstacle::lastVanVariant = -1;
int Obstacle::lastDroneVariant = -1;
int Obstacle::lastThrowingCarVariant = -1;
int Obstacle::lastBoxVariant = -1;
sf::SoundBuffer Obstacle::droneSoundBuffer;
bool Obstacle::droneSoundBufferLoaded = false;
float Obstacle::droneSoundVolume = 100.0f;

sf::SoundBuffer Obstacle::carEngineBuffer;
bool Obstacle::carEngineBufferLoaded = false;
sf::SoundBuffer Obstacle::carHornBuffers[Obstacle::NUM_CAR_HORN_VARIANTS];
bool Obstacle::carHornBuffersLoaded[Obstacle::NUM_CAR_HORN_VARIANTS] = { false, false };
bool Obstacle::carSoundsLoaded = false;
float Obstacle::carEngineVolume = 80.0f;
float Obstacle::carHornVolume = 80.0f;
float Obstacle::slowMoAudioPitch = 1.0f;
float Obstacle::slowMoAudioVolumeMult = 1.0f;

void Obstacle::loadTextures() {
    if (texturesLoaded) return;
    shadeTexture.loadFromFile("assets/Shade.png");
    if (shadeTexture.getSize().x == 0) {
        std::cerr << "Warning: could not load assets/Shade.png -- vehicle shadows will be invisible/broken until this file exists.\n";
    }
    tireTexture.loadFromFile("assets/Tire.png");

    // Car variants. Variant 0 is the BMW M4. Each variant needs a body
    // image, a wheel image, and a CarWheelLayout giving wheel position
    // on that body as fractions, measured from the body art's
    // wheel-well cutouts since placement varies car to car.
    carBodyTextures.resize(NUM_CAR_VARIANTS);
    carTargetHeights.resize(NUM_CAR_VARIANTS);
    carTargetHeights[0] = 94.0f; // Car1
    carTargetHeights[1] = 94.0f; // Car2
    carTargetHeights[2] = 94.0f; // Car3
    carTargetHeights[3] = 94.0f; // Car4
    carTargetHeights[4] = 94.0f; // Car5
    carTargetHeights[5] = 94.0f; // Car6
    carTargetHeights[6] = 94.0f; // Car7
    carTargetHeights[7] = 94.0f; // Car8
    carTargetHeights[8] = 94.0f; // Car9
    carTargetHeights[9] = 100.0f; // Car10
    carTargetHeights[10] = 94.0f; // Car11
    carTargetHeights[11] = 94.0f; // Car12
    carTargetHeights[12] = 94.0f; // Car13
    carTargetHeights[13] = 94.0f; // Car14
    carTargetHeights[14] = 94.0f; // Car15
    carTargetHeights[15] = 94.0f; // Car16
    carTargetHeights[16] = 94.0f; // Car17
    carTargetHeights[17] = 94.0f; // Car18
    carTargetHeights[18] = 94.0f; // Car19
    carTargetHeights[19] = 94.0f; // Car20
    carTargetHeights[20] = 94.0f; // Car21
    carTargetHeights[21] = 94.0f; // Car22
    carTargetHeights[22] = 94.0f; // Car23
    carTargetHeights[23] = 94.0f; // Car24
    carTargetHeights[24] = 94.0f; // Car25
    carTargetHeights[25] = 94.0f; // Car26
    carTargetHeights[26] = 94.0f; // Car27
    carTargetHeights[27] = 94.0f; // Car28
    carTargetHeights[28] = 94.0f; // Car29
    carTargetHeights[29] = 94.0f; // Car30
    carTargetHeights[30] = 94.0f; // Car31
    carTargetHeights[31] = 94.0f; // Car32
    carTargetHeights[32] = 94.0f; // Car33
    carTargetHeights[33] = 94.0f; // Car34
    carTargetHeights[34] = 94.0f; // Car35
    carTargetHeights[35] = 94.0f; // Car36
    carTargetHeights[36] = 94.0f; // Car37
    carTargetHeights[37] = 94.0f; // Car38
    carTargetHeights[38] = 94.0f; // Car39
    carTargetHeights[39] = 94.0f; // Car40

    // Per-variant ground line, defaults to 665.0f for every car. Set
    // carGroundYs[N] individually to give one car its own ground line,
    // same pattern as carTargetHeights.
    carGroundYs.resize(NUM_CAR_VARIANTS);
    for (int i = 0; i < NUM_CAR_VARIANTS; i++) carGroundYs[i] = 665.0f;

    carWheelTextures.resize(NUM_CAR_VARIANTS);
    carWheelLayouts.resize(NUM_CAR_VARIANTS);

    carBodyTextures[0].loadFromFile("assets/CarBody1.png");
    if (carBodyTextures[0].getSize().x == 0) {
        std::cerr << "Warning: could not load assets/CarBody1.png -- car will be invisible/broken until this file exists.\n";
    }
    carWheelTextures[0].loadFromFile("assets/CarWheel1.png");
    if (carWheelTextures[0].getSize().x == 0) {
        std::cerr << "Warning: could not load assets/CarWheel1.png -- car wheels will be invisible/broken until this file exists.\n";
    }
    // Measured from the M4 body art (627x176 canvas): front wheel well
    // center (107, 155), diameter ~96px; rear well center (492, 159),
    // diameter ~90px. Expressed as fractions of the body's own
    // width/height so it stays correct at any render size. The car's
    // nose points right, so the X fractions are mirrored (1 - value)
    // from a left-facing measurement; Y fractions are unchanged since
    // mirroring is horizontal only. Approximate values pending direct
    // measurement of the final art.
    carWheelLayouts[0] = { 0.824f, 0.818f, 0.215f, 0.808f, 0.168f };

    // Files for variants 2-5 load in this loop, same idea as variant 1.
    for (int i = 1; i < NUM_CAR_VARIANTS; i++) {
        std::string bodyPath = "assets/CarBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/CarWheel" + std::to_string(i + 1) + ".png";
        carBodyTextures[i].loadFromFile(bodyPath);
        if (carBodyTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- this car variant will be invisible/broken until this file exists.\n";
        }
        carWheelTextures[i].loadFromFile(wheelPath);
        if (carWheelTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << " -- this car variant's wheels will be invisible/broken until this file exists.\n";
        }
    }

    // Car wheel placements.
    carWheelLayouts[1] = { 0.818f, 0.818f, 0.213f, 0.822f, 0.168f };
    carWheelLayouts[2] = { 0.802f, 0.792f, 0.226f, 0.792f, 0.177f };
    carWheelLayouts[3] = { 0.802f, 0.873f, 0.211f, 0.877f, 0.166f };
    carWheelLayouts[4] = { 0.807f, 0.812f, 0.223f, 0.801f, 0.160f };
    carWheelLayouts[5] = { 0.791f, 0.844f, 0.215f, 0.845f, 0.167f };
    carWheelLayouts[6] = { 0.791f, 0.801f, 0.204f, 0.808f, 0.178f };
    carWheelLayouts[7] = { 0.772f, 0.857f, 0.219f, 0.851f, 0.149f };
    carWheelLayouts[8] = { 0.797f, 0.853f, 0.200f, 0.849f, 0.148f };
    carWheelLayouts[9] = { 0.779f, 0.830f, 0.191f, 0.828f, 0.170f };
    carWheelLayouts[10] = { 0.809f, 0.830f, 0.201f, 0.820f, 0.164f };
    carWheelLayouts[11] = { 0.781f, 0.838f, 0.212f, 0.823f, 0.173f };
    carWheelLayouts[12] = { 0.761f, 0.832f, 0.208f, 0.839f, 0.161f };
    carWheelLayouts[13] = { 0.820f, 0.877f, 0.233f, 0.873f, 0.148f };
    carWheelLayouts[14] = { 0.786f, 0.848f, 0.217f, 0.849f, 0.158f };
    carWheelLayouts[15] = { 0.780f, 0.830f, 0.211f, 0.841f, 0.178f };
    carWheelLayouts[16] = { 0.753f, 0.876f, 0.195f, 0.877f, 0.148f };
    carWheelLayouts[17] = { 0.801f, 0.837f, 0.203f, 0.835f, 0.157f };
    carWheelLayouts[18] = { 0.807f, 0.859f, 0.219f, 0.855f, 0.157f };
    carWheelLayouts[19] = { 0.802f, 0.843f, 0.212f, 0.820f, 0.158f };
    carWheelLayouts[20] = { 0.810f, 0.864f, 0.211f, 0.864f, 0.157f };
    carWheelLayouts[21] = { 0.807f, 0.870f, 0.211f, 0.867f, 0.156f };
    carWheelLayouts[22] = { 0.799f, 0.902f, 0.218f, 0.898f, 0.154f };
    carWheelLayouts[23] = { 0.810f, 0.878f, 0.202f, 0.881f, 0.159f };
    carWheelLayouts[24] = { 0.809f, 0.861f, 0.205f, 0.859f, 0.151f };
    carWheelLayouts[25] = { 0.765f, 0.843f, 0.212f, 0.836f, 0.153f };
    carWheelLayouts[26] = { 0.750f, 0.867f, 0.210f, 0.866f, 0.148f };
    carWheelLayouts[27] = { 0.784f, 0.811f, 0.204f, 0.808f, 0.178f };
    carWheelLayouts[28] = { 0.763f, 0.855f, 0.197f, 0.852f, 0.158f };
    carWheelLayouts[29] = { 0.795f, 0.854f, 0.206f, 0.842f, 0.149f };
    carWheelLayouts[30] = { 0.793f, 0.828f, 0.192f, 0.828f, 0.164f }; // Car31
    carWheelLayouts[31] = { 0.802f, 0.845f, 0.201f, 0.845f, 0.160f }; // Car32
    carWheelLayouts[32] = { 0.812f, 0.838f, 0.221f, 0.827f, 0.160f }; // Car33
    carWheelLayouts[33] = { 0.803f, 0.840f, 0.214f, 0.839f, 0.164f }; // Car34
    carWheelLayouts[34] = { 0.809f, 0.856f, 0.208f, 0.840f, 0.173f }; // Car35
    carWheelLayouts[35] = { 0.816f, 0.850f, 0.211f, 0.848f, 0.171f }; // Car36
    carWheelLayouts[36] = { 0.815f, 0.865f, 0.215f, 0.845f, 0.173f }; // Car37
    carWheelLayouts[37] = { 0.808f, 0.821f, 0.204f, 0.823f, 0.166f }; // Car38
    carWheelLayouts[38] = { 0.834f, 0.867f, 0.223f, 0.867f, 0.165f }; // Car39
    carWheelLayouts[39] = { 0.801f, 0.865f, 0.211f, 0.866f, 0.167f }; // Car40

    // Car collision lines.
    carCollisionLayouts.resize(NUM_CAR_VARIANTS);

    carCollisionLayouts[0] = { // Car 1
        0.838f, 0.462f, 0.334f, 5.0f, 9.9f,    // hood
        0.598f, 0.240f, 0.191f, 4.7f, 26.4f,   // windshield
        0.392f, 0.139f, 0.281f, 5.3f, -4.7f,    // roof
        0.144f, 0.294f, 0.279f, 4.0f, -16.1f,  // trunk
        6.1f, 70.0f, 4.4f, 66.0f,             // front death line
        9.7f, 49.0f, 3.9f, 44.5f              // back death line
    };
    carCollisionLayouts[1] = {
        0.812f, 0.412f, 0.354f, 4.5f, 6.9f,    // hood
        0.569f, 0.208f, 0.191f, 3.7f, 26.4f,   // windshield
        0.388f, 0.086f, 0.200f, 4.0f, -2.0f,    // roof
        0.156f, 0.260f, 0.287f, 4.0f, -20.0f,  // trunk
        8.5f, 68.2f, 4.4f, 66.0f,             // front death line
        8.0f, 48.2f, 3.9f, 44.5f              // back death line
    };
    carCollisionLayouts[2] = {
        0.837f, 0.456f, 0.312f, 4.5f, 8.5f,    // hood
        0.621f, 0.258f, 0.164f, 5.3f, 29.7f,   // windshield
        0.432f, 0.130f, 0.277f, 5.9f, -1.0f,    // roof
        0.170f, 0.286f, 0.287f, 5.8f, -17.9f,  // trunk
        8.5f, 68.2f, 4.4f, 66.0f,             // front death line
        8.0f, 48.2f, 3.9f, 44.5f              // back death line
    };
    carCollisionLayouts[3] = {
        0.840f, 0.477f, 0.310f, 4.5f, 8.1f,    // hood
        0.626f, 0.270f, 0.164f, 6.7f, 30.4f,   // windshield
        0.417f, 0.117f, 0.304f, 5.9f, 0.3f,    // roof
        0.160f, 0.285f, 0.277f, 5.8f, -21.8f,  // trunk
        5.1f, 69.8f, 4.4f, 66.0f,             // front death line
        11.2f, 51.4f, 3.9f, 44.5f              // back death line
    };
    carCollisionLayouts[4] = {
        0.842f, 0.437f, 0.312f, 4.5f, 5.2f,    // hood
        0.623f, 0.260f, 0.192f, 5.7f, 30.4f,   // windshield
        0.420f, 0.134f, 0.278f, 6.6f, -3.3f,    // roof
        0.160f, 0.285f, 0.277f, 5.8f, -17.4f,  // trunk
        12.0f, 64.2f, 4.4f, 66.0f,             // front death line
        8.4f, 54.0f, 3.9f, 110.5f              // back death line
    };
    carCollisionLayouts[5] = {
        0.842f, 0.437f, 0.312f, 4.5f, 5.2f,    // hood
        0.623f, 0.260f, 0.192f, 5.7f, 30.4f,   // windshield
        0.420f, 0.134f, 0.278f, 6.6f, -3.3f,    // roof
        0.160f, 0.285f, 0.277f, 5.8f, -17.4f,  // trunk
        8.3f, 71.7f, 4.4f, 66.0f,             // front death line
        7.3f, 50.8f, 3.9f, 110.5f              // back death line
    };
    carCollisionLayouts[6] = {
        0.850f, 0.399f, 0.312f, 4.5f, 5.2f,    // hood
        0.640f, 0.233f, 0.192f, 5.7f, 27.8f,   // windshield
        0.429f, 0.083f, 0.281f, 6.0f, -1.0f,    // roof
        0.166f, 0.245f, 0.321f, 4.9f, -18.4f,  // trunk
        8.6f, 65.2f, 4.4f, 66.0f,             // front death line
        5.4f, 51.2f, 3.9f, 110.5f              // back death line
    };
    carCollisionLayouts[7] = {
        0.857f, 0.472f, 0.312f, 4.5f, 6.8f,    // hood
        0.637f, 0.249f, 0.192f, 5.7f, 32.2f,   // windshield
        0.429f, 0.083f, 0.281f, 6.0f, -1.0f,    // roof
        0.166f, 0.241f, 0.291f, 4.9f, -16.3f,  // trunk
        7.0f, 66.3f, 4.4f, 66.0f,             // front death line
        12.0f, 52.1f, 3.9f, 110.5f              // back death line
    };
    carCollisionLayouts[8] = {
        0.864f, 0.478f, 0.284f, 4.3f, 10.3f,    // hood
        0.658f, 0.236f, 0.179f, 4.2f, 29.8f,   // windshield
        0.462f, 0.068f, 0.256f, 4.4f, 1.2f,    // roof
        0.187f, 0.220f, 0.334f, 4.3f, -15.8f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[9] = {
        0.875f, 0.444f, 0.257f, 4.8f, 8.8f,    // hood
        0.671, 0.247f, 0.196f, 4.6f, 29.0f,   // windshield
        0.441f, 0.109f, 0.310f, 4.7f, -1.3f,    // roof
        0.166f, 0.266f, 0.276f, 3.7f, -20.6f,  // trunk
        7.2f, 60.2f, 6.0f, 70.0f,             // front death line
        12.1f, 45.3f, 3.4f, 40.0f              // back death line
    };
    carCollisionLayouts[10] = {
        0.842f, 0.440f, 0.319f, 4.3f, 7.8f,    // hood
        0.618f, 0.230f, 0.179f, 4.7f, 29.5f,   // windshield
        0.419f, 0.100f, 0.279f, 4.4f, -1.0f,    // roof
        0.159f, 0.241f, 0.282f, 4.4f, -16.4f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[11] = {
        0.829f, 0.414f, 0.344f, 4.3f, 7.8f,    // hood
        0.610f, 0.217f, 0.161f, 4.7f, 29.5f,   // windshield
        0.419f, 0.100f, 0.279f, 4.4f, -1.0f,    // roof
        0.153f, 0.253f, 0.282f, 4.4f, -16.4f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[12] = {
        0.829f, 0.414f, 0.344f, 4.3f, 7.8f,    // hood
        0.610f, 0.217f, 0.161f, 4.7f, 29.5f,   // windshield
        0.419f, 0.100f, 0.279f, 4.4f, -1.0f,    // roof
        0.153f, 0.253f, 0.282f, 4.4f, -16.4f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[13] = {
        0.855f, 0.432f, 0.318f, 4.3f, 10.4f,    // hood
        0.641f, 0.222f, 0.161f, 4.7f, 27.8f,   // windshield
        0.429f, 0.109f, 0.299f, 4.4f, -1.0f,    // roof
        0.153f, 0.253f, 0.282f, 4.4f, -17.9f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[14] = {
        0.827f, 0.396f, 0.356f, 4.3f, 6.1f,    // hood
        0.587f, 0.198f, 0.161f, 4.7f, 27.8f,   // windshield
        0.403f, 0.097f, 0.249f, 4.9f, -4.6f,    // roof
        0.153f, 0.238f, 0.285f, 4.4f, -13.2f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[15] = {
        0.822f, 0.392f, 0.356f, 4.3f, 6.1f,    // hood
        0.587f, 0.198f, 0.161f, 4.7f, 27.8f,   // windshield
        0.407f, 0.087f, 0.249f, 4.9f, -1.2f,    // roof
        0.160f, 0.250f, 0.285f, 4.4f, -19.0f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[16] = {
        0.849f, 0.490f, 0.300f, 5.2f, 9.1f,    // hood
        0.628f, 0.247f, 0.192f, 4.7f, 27.8f,   // windshield
        0.420f, 0.097f, 0.272f, 4.9f, -1.2f,    // roof
        0.164f, 0.231f, 0.285f, 4.4f, -14.6f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        6.2f, 47.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[17] = {
        0.854f, 0.474f, 0.300f, 5.2f, 9.1f,    // hood
        0.633f, 0.250f, 0.192f, 4.7f, 27.8f,   // windshield
        0.427f, 0.113f, 0.272f, 4.9f, 0.2f,    // roof
        0.164f, 0.231f, 0.285f, 4.4f, -14.8f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        10.3f, 46.0f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[18] = {
        0.844f, 0.485f, 0.345f, 4.9f, 6.5f,    // hood
        0.614f, 0.272f, 0.177f, 4.7f, 32.8f,   // windshield
        0.439f, 0.115f, 0.220f, 4.9f, 0.2f,    // roof
        0.186f, 0.262f, 0.330f, 4.4f, -16.4f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        9.6f, 48.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[19] = {
        0.853f, 0.446f, 0.286f, 4.9f, 11.5f,    // hood
        0.645f, 0.228f, 0.177f, 4.7f, 28.2f,   // windshield
        0.441f, 0.085f, 0.273f, 4.9f, 0.2f,    // roof
        0.180f, 0.228f, 0.298f, 4.4f, -18.3f,  // trunk
        4.7f, 65.6f, 6.0f, 70.0f,             // front death line
        9.6f, 48.1f, 6.0f, 43.0f              // back death line
    };
    carCollisionLayouts[20] = {
        0.867f, 0.464f, 0.285f, 5.0f, 9.8f,    // hood
        0.661f, 0.240f, 0.188f, 5.6f, 27.6f,   // windshield
        0.432f, 0.099f, 0.325f, 5.0f, -0.1f,    // roof
        0.153f, 0.251f, 0.272f, 4.8f, -19.2f,  // trunk
        7.3f, 67.1f, 6.0f, 70.0f,             // front death line
        10.7f, 48.0f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[21] = {
        0.866f, 0.496f, 0.275f, 5.0f, 9.8f,    // hood
        0.661f, 0.265f, 0.188f, 5.6f, 27.6f,   // windshield
        0.440f, 0.123f, 0.325f, 5.0f, -0.1f,    // roof
        0.164f, 0.269f, 0.272f, 4.8f, -19.2f,  // trunk
        7.3f, 67.1f, 6.0f, 70.0f,             // front death line
        10.7f, 48.0f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[22] = {
        0.858f, 0.493f, 0.275f, 5.0f, 9.8f,    // hood
        0.657f, 0.263f, 0.188f, 5.6f, 27.6f,   // windshield
        0.429f, 0.105f, 0.306f, 5.0f, -0.1f,    // roof
        0.154f, 0.278f, 0.283f, 4.8f, -19.2f,  // trunk
        7.3f, 67.1f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[23] = {
        0.862f, 0.468f, 0.275f, 5.6f, 8.5f,    // hood
        0.652f, 0.254f, 0.188f, 5.6f, 27.6f,   // windshield
        0.429f, 0.105f, 0.306f, 5.0f, -0.1f,    // roof
        0.157f, 0.258f, 0.283f, 4.8f, -17.5f,  // trunk
        7.3f, 67.1f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[24] = {
        0.862f, 0.450f, 0.275f, 5.6f, 7.0f,    // hood
        0.652f, 0.254f, 0.188f, 5.0f, 27.6f,   // windshield
        0.429f, 0.105f, 0.306f, 5.0f, -0.1f,    // roof
        0.157f, 0.258f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[25] = {
        0.834f, 0.427f, 0.327f, 5.3f, 7.2f,    // hood
        0.613f, 0.235f, 0.159f, 5.0f, 27.6f,   // windshield
        0.423f, 0.109f, 0.265f, 5.0f, -0.1f,    // roof
        0.164f, 0.257f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[26] = {
        0.832f, 0.465f, 0.327f, 5.3f, 7.2f,    // hood
        0.610f, 0.252f, 0.159f, 5.0f, 31.2f,   // windshield
        0.423f, 0.109f, 0.265f, 5.0f, -0.1f,    // roof
        0.164f, 0.257f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[27] = {
        0.833f, 0.408f, 0.336f, 5.3f, 4.4f,    // hood
        0.611f, 0.237f, 0.159f, 5.0f, 31.2f,   // windshield
        0.422f, 0.102f, 0.265f, 5.0f, -0.1f,    // roof
        0.163f, 0.248f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[28] = {
        0.835f, 0.464f, 0.336f, 4.8f, 8.7f,    // hood
        0.611f, 0.237f, 0.159f, 5.0f, 31.2f,   // windshield
        0.422f, 0.102f, 0.265f, 5.0f, -0.1f,    // roof
        0.163f, 0.248f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[29] = {
        0.873f, 0.490f, 0.266f, 4.8f, 9.5f,    // hood
        0.649f, 0.264f, 0.232f, 5.8f, 24.8f,   // windshield
        0.422f, 0.102f, 0.265f, 5.0f, -0.1f,    // roof
        0.163f, 0.248f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[30] = {
        0.820f, 0.408f, 0.357f, 4.8f, 6.5f,    // hood
        0.598f, 0.219f, 0.134f, 5.8f, 32.7f,   // windshield
        0.422f, 0.102f, 0.265f, 5.0f, -0.1f,    // roof
        0.163f, 0.248f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[31] = {
        0.854f, 0.471f, 0.306f, 4.8f, 9.5f,    // hood
        0.642f, 0.236f, 0.163f, 5.8f, 32.7f,   // windshield
        0.428f, 0.094f, 0.317f, 5.0f, -0.1f,    // roof
        0.153f, 0.236f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[32] = {
        0.839f, 0.423f, 0.334f, 4.8f, 9.3f,    // hood
        0.632f, 0.209f, 0.131f, 5.8f, 32.5f,   // windshield
        0.428f, 0.094f, 0.318f, 5.0f, -0.1f,    // roof
        0.153f, 0.236f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[33] = {
        0.839f, 0.400f, 0.334f, 4.8f, 9.3f,    // hood
        0.605f, 0.181f, 0.181f, 5.8f, 25.0f,   // windshield
        0.408f, 0.085f, 0.256f, 5.0f, -4.0f,    // roof
        0.157f, 0.259f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[34] = {
        0.842f, 0.405f, 0.334f, 4.8f, 9.3f,    // hood
        0.605f, 0.181f, 0.181f, 5.8f, 25.0f,   // windshield
        0.408f, 0.085f, 0.256f, 5.0f, -4.0f,    // roof
        0.157f, 0.259f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[35] = {
        0.842f, 0.405f, 0.334f, 4.8f, 9.3f,    // hood
        0.605f, 0.181f, 0.181f, 5.8f, 25.0f,   // windshield
        0.408f, 0.085f, 0.256f, 5.0f, -4.0f,    // roof
        0.157f, 0.259f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[36] = {
        0.864f, 0.435f, 0.309f, 4.8f, 11.7f,    // hood
        0.645f, 0.216f, 0.181f, 5.8f, 25.0f,   // windshield
        0.431f, 0.119f, 0.293f, 4.7f, -4.4f,    // roof
        0.161f, 0.282f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[37] = {
        0.857f, 0.429f, 0.296f, 4.8f, 10.0f,    // hood
        0.647f, 0.221f, 0.181f, 5.8f, 25.0f,   // windshield
        0.432f, 0.101f, 0.293f, 4.7f, -0.5f,    // roof
        0.161f, 0.235f, 0.283f, 4.8f, -17.5f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[38] = {
        0.859f, 0.474f, 0.300f, 4.8f, 8.7f,    // hood
        0.647f, 0.245f, 0.181f, 5.4f, 31.9f,   // windshield
        0.440f, 0.102f, 0.293f, 4.7f, -0.5f,    // roof
        0.165f, 0.252f, 0.300f, 4.8f, -15.1f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };
    carCollisionLayouts[39] = {
        0.856f, 0.454f, 0.296f, 5.0f, 8.3f,    // hood
        0.642f, 0.241f, 0.181f, 5.4f, 27.1f,   // windshield
        0.427f, 0.101f, 0.293f, 4.7f, -0.5f,    // roof
        0.163f, 0.230f, 0.274f, 4.8f, -15.1f,  // trunk
        3.2f, 65.0f, 6.0f, 70.0f,             // front death line
        6.5f, 46.8f, 3.7f, 41.5f              // back death line
    };

    // Jeep variants: body image, wheel image, wheel layout, same
    // pattern as the sedans. Canvas size 900x281 (vs the sedan's
    // 627x176).
    jeepBodyTextures.resize(NUM_JEEP_VARIANTS);
    jeepTargetHeights.resize(NUM_JEEP_VARIANTS);
    jeepTargetHeights[0] = 140.0f; // Jeep1
    jeepTargetHeights[1] = 140.0f; // Jeep2
    jeepTargetHeights[2] = 140.0f; // Jeep3
    jeepTargetHeights[3] = 140.0f; // Jeep4
    jeepTargetHeights[4] = 140.0f; // Jeep5
    jeepTargetHeights[5] = 140.0f; // Jeep6
    jeepTargetHeights[6] = 140.0f; // Jeep7
    jeepTargetHeights[7] = 140.0f; // Jeep8
    jeepTargetHeights[8] = 140.0f; // Jeep9
    jeepTargetHeights[9] = 140.0f; // Jeep10

    // Per-variant ground line, defaults to 658.0f for every jeep. Set
    // jeepGroundYs[N] individually to give one jeep its own ground
    // line.
    jeepGroundYs.resize(NUM_JEEP_VARIANTS);
    for (int i = 0; i < NUM_JEEP_VARIANTS; i++) jeepGroundYs[i] = 658.0f;

    jeepWheelTextures.resize(NUM_JEEP_VARIANTS);
    jeepWheelLayouts.resize(NUM_JEEP_VARIANTS);
    jeepCollisionLayouts.resize(NUM_JEEP_VARIANTS);

    for (int i = 0; i < NUM_JEEP_VARIANTS; i++) {
        std::string bodyPath = "assets/JeepBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/JeepWheel" + std::to_string(i + 1) + ".png";
        jeepBodyTextures[i].loadFromFile(bodyPath);
        if (jeepBodyTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- this jeep variant will be invisible/broken until this file exists.\n";
        }
        jeepWheelTextures[i].loadFromFile(wheelPath);
        if (jeepWheelTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << " -- this jeep variant's wheels will be invisible/broken until this file exists.\n";
        }
    }

    // Wheel layouts, canvas 900x281.
    jeepWheelLayouts[0] = { 0.819f, 0.953f, 0.206f, 0.956f, 0.188f }; // Jeep1
    jeepWheelLayouts[1] = { 0.794f, 0.902f, 0.186f, 0.899f, 0.163f }; // Jeep2
    jeepWheelLayouts[2] = { 0.804f, 0.850f, 0.193f, 0.844f, 0.185f }; // Jeep3
    jeepWheelLayouts[3] = { 0.794f, 0.908f, 0.192f, 0.903f, 0.163f }; // Jeep4
    jeepWheelLayouts[4] = { 0.849f, 0.926f, 0.213f, 0.927f, 0.177f }; // Jeep5
    jeepWheelLayouts[5] = { 0.805f, 0.903f, 0.202f, 0.901f, 0.168f }; // Jeep6
    jeepWheelLayouts[6] = { 0.797f, 0.908f, 0.195f, 0.905f, 0.168f }; // Jeep7
    jeepWheelLayouts[7] = { 0.816f, 0.927f, 0.222f, 0.907f, 0.180f }; // Jeep8
    jeepWheelLayouts[8] = { 0.792f, 0.927f, 0.186f, 0.928f, 0.168f }; // Jeep9
    jeepWheelLayouts[9] = { 0.799f, 0.896f, 0.191f, 0.894f, 0.170f }; // Jeep10

    // Jeep collision lines.

    jeepCollisionLayouts[0].walkFreeSegments = { // Jeep1
        { 0.364f, 0.160f, 0.561f, 5.0f, 0.6f }, // rear roof / hatch
        { 0.675f, 0.339f, 0.160f, 6.0f, 56.0f }, // windshield / A-pillar
        { 0.844f, 0.537f, 0.270f, 5.0f, 4.1f }, // hood / front roofline
    };
    jeepCollisionLayouts[0].deathZones = {
        { 0.082f, 0.288f, 0.050f, 5.0f, -35.0f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.970f, 0.780f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[1].walkFreeSegments = {
        { 0.332f, 0.097f, 0.528f, 5.0f, -2.4f }, // rear roof / hatch
        { 0.652f, 0.239f, 0.189f, 5.3f, 41.0f }, // windshield / A-pillar
        { 0.857f, 0.469f, 0.270f, 5.0f, 7.9f }, // hood / front roofline
    };
    jeepCollisionLayouts[1].deathZones = {
        { 0.071f, 0.270f, 0.081f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.980f, 0.782f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[2].walkFreeSegments = {
        { 0.310f, 0.112f, 0.545f, 5.8f, 0.8f }, // rear roof / hatch
        { 0.644f, 0.239f, 0.171f, 6.1f, 33.3f }, // windshield / A-pillar
        { 0.852f, 0.432f, 0.301f, 6.2f, 8.6f }, // hood / front roofline
    };
    jeepCollisionLayouts[2].deathZones = {
        { 0.059f, 0.255f, 0.077f, 5.0f, -58.5f }, // rear roof pillar
        { 0.030f, 0.470f, 0.114f, 6.1f, 98.0f }, // taillight
        { 0.020f, 0.850f, 0.122f, 6.0f, 90.0f }, // rear bumper
        { 0.974f, 0.790f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[3].walkFreeSegments = {
        { 0.327f, 0.121f, 0.516f, 5.9f, -2.4f }, // rear roof / hatch
        { 0.657f, 0.260f, 0.207f, 7.7f, 34.5f }, // windshield / A-pillar
        { 0.869f, 0.466f, 0.270f, 6.2f, 7.9f }, // hood / front roofline
    };
    jeepCollisionLayouts[3].deathZones = {
        { 0.052f, 0.279f, 0.081f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.980f, 0.782f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[4].walkFreeSegments = {
        { 0.355f, 0.148f, 0.539f, 5.0f, 0.8f }, // rear roof / hatch
        { 0.647f, 0.297f, 0.123f, 5.3f, 61.1f }, // windshield / A-pillar
        { 0.816f, 0.491f, 0.306f, 5.0f, 8.0f }, // hood / front roofline
    };
    jeepCollisionLayouts[4].deathZones = {
        { 0.070f, 0.282f, 0.050f, 5.0f, -35.0f }, // rear roof pillar
        { 0.023f, 0.506f, 0.136f, 6.0f, 100.3f }, // taillight
        { 0.021f, 0.904f, 0.104f, 6.0f, 90.0f }, // rear bumper
        { 0.950f, 0.859f, 0.119f, 6.0f, 88.4f }, // front bumper
    };
    jeepCollisionLayouts[5].walkFreeSegments = {
        { 0.334f, 0.111f, 0.524f, 6.0f, -1.5f }, // rear roof / hatch
        { 0.648f, 0.247f, 0.170f, 7.7f, 41.8f }, // windshield / A-pillar
        { 0.848f, 0.456f, 0.299f, 6.2f, 7.9f }, // hood / front roofline
    };
    jeepCollisionLayouts[5].deathZones = {
        { 0.051f, 0.279f, 0.052f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.961f, 0.863f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[6].walkFreeSegments = {
        { 0.334f, 0.111f, 0.524f, 6.0f, -1.5f }, // rear roof / hatch
        { 0.648f, 0.247f, 0.170f, 7.7f, 41.8f }, // windshield / A-pillar
        { 0.848f, 0.456f, 0.299f, 6.2f, 7.9f }, // hood / front roofline
    };
    jeepCollisionLayouts[6].deathZones = {
        { 0.051f, 0.279f, 0.052f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.961f, 0.863f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[7].walkFreeSegments = {
        { 0.365f, 0.141f, 0.559f, 7.1f, 0.2f }, // rear roof / hatch
        { 0.685f, 0.302f, 0.156f, 5.8f, 51.7f }, // windshield / A-pillar
        { 0.855f, 0.500f, 0.265f, 6.2f, 7.1f }, // hood / front roofline
    };
    jeepCollisionLayouts[7].deathZones = {
        { 0.081f, 0.276f, 0.052f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.971f, 0.861f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[8].walkFreeSegments = {
        { 0.330f, 0.096f, 0.524f, 5.8f, 0.2f }, // rear roof / hatch
        { 0.661f, 0.285f, 0.204f, 5.8f, 39.3f }, // windshield / A-pillar
        { 0.861f, 0.495f, 0.265f, 6.2f, 7.1f }, // hood / front roofline
    };
    jeepCollisionLayouts[8].deathZones = {
        { 0.051f, 0.279f, 0.052f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.971f, 0.861f, 0.070f, 6.0f, 90.0f }, // front bumper
    };
    jeepCollisionLayouts[9].walkFreeSegments = {
        { 0.330f, 0.096f, 0.524f, 6.0f, -1.1f }, // rear roof / hatch
        { 0.660f, 0.264f, 0.204f, 5.8f, 39.3f }, // windshield / A-pillar
        { 0.859f, 0.471f, 0.265f, 6.2f, 7.1f }, // hood / front roofline
    };
    jeepCollisionLayouts[9].deathZones = {
        { 0.051f, 0.279f, 0.052f, 5.0f, -55.2f }, // rear roof pillar
        { 0.030f, 0.470f, 0.105f, 6.0f, 99.8f }, // taillight
        { 0.020f, 0.850f, 0.143f, 6.0f, 90.0f }, // rear bumper
        { 0.961f, 0.863f, 0.070f, 6.0f, 90.0f }, // front bumper
    };

    // Downlane van variants. Canvas size 1100x386, the biggest of the
    // three downlane vehicle kinds. Separate from the up-lane Van in
    // Background.cpp: own textures, own lane, own collision.
    vanBodyTextures.resize(NUM_DOWNLANE_VAN_VARIANTS);
    vanTargetHeights.resize(NUM_DOWNLANE_VAN_VARIANTS);
    vanTargetHeights[0] = 160.0f; // Van1
    vanTargetHeights[1] = 160.0f; // Van2
    vanTargetHeights[2] = 160.0f; // Van3
    vanTargetHeights[3] = 160.0f; // Van4
    vanTargetHeights[4] = 160.0f; // Van5

    // Per-variant ground line, defaults to 660.0f for every van. Set
    // vanGroundYs[N] individually to give one van its own ground line.
    vanGroundYs.resize(NUM_DOWNLANE_VAN_VARIANTS);
    for (int i = 0; i < NUM_DOWNLANE_VAN_VARIANTS; i++) vanGroundYs[i] = 660.0f;

    vanWheelTextures.resize(NUM_DOWNLANE_VAN_VARIANTS);
    vanWheelLayouts.resize(NUM_DOWNLANE_VAN_VARIANTS);
    vanCollisionLayouts.resize(NUM_DOWNLANE_VAN_VARIANTS);

    for (int i = 0; i < NUM_DOWNLANE_VAN_VARIANTS; i++) {
        std::string bodyPath = "assets/VanBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/VanWheel" + std::to_string(i + 1) + ".png";
        vanBodyTextures[i].loadFromFile(bodyPath);
        if (vanBodyTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- this van variant will be invisible/broken until this file exists.\n";
        }
        vanWheelTextures[i].loadFromFile(wheelPath);
        if (vanWheelTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << " -- this van variant's wheels will be invisible/broken until this file exists.\n";
        }
    }

    // Wheel layouts, canvas 1100x386.
    vanWheelLayouts[0] = { 0.824f, 0.908f, 0.200f, 0.907f, 0.130f }; // Van1
    vanWheelLayouts[1] = { 0.782f, 0.884f, 0.189f, 0.889f, 0.135f }; // Van2
    // Van3-5 are placeholders copied from Van1, pending measurement of
    // their real art.
    vanWheelLayouts[2] = { 0.843f, 0.877f, 0.201f, 0.882f, 0.135f }; // Van3, placeholder
    vanWheelLayouts[3] = { 0.790f, 0.912f, 0.142f, 0.910f, 0.135f }; // Van4, placeholder
    vanWheelLayouts[4] = { 0.790f, 0.912f, 0.142f, 0.910f, 0.135f }; // Van5, placeholder

    // Each DownlaneVan variant is its own separate array entry, so
    // editing one variant's numbers doesn't affect any other.
    vanCollisionLayouts[0].walkFreeSegments = {
        { 0.374f, 0.043f, 0.656f, 7.3f, 0.4f },   // box roof, flat
        { 0.775f, 0.243f, 0.213f, 6.0f, 38.2f },  // windshield slope
        { 0.908f, 0.483f, 0.154f, 5.0f, 22.4f },  // hood slope
    };
    vanCollisionLayouts[0].deathZones = {
        { 0.037f, 0.448f, 0.280f, 6.0f, 93.6f },  // back edge: one long near-vertical line down the box's rear face
        { 0.970f, 0.750f, 0.100f, 6.0f, 90.0f },  // front bumper
    };

    vanCollisionLayouts[1].walkFreeSegments = {
        { 0.323f, 0.072f, 0.597f, 7.5f, 0.6f },   // box roof, flat
        { 0.687f, 0.253f, 0.190f, 8.1f, 39.2f },  // windshield slope
        { 0.875f, 0.455f, 0.243f, 7.7f, 7.4f },  // hood slope
    };
    vanCollisionLayouts[1].deathZones = {
        { 0.026f, 0.600f, 0.329f, 6.4f, 93.2},  // back edge: one long near-vertical line down the box's rear face
        { 0.970f, 0.750f, 0.100f, 6.0f, 90.0f },  // front bumper
    };

    vanCollisionLayouts[2].walkFreeSegments = {

        { 0.390f, 0.049f, 0.684f, 7.3f, 0.6f},

        { 0.780f, 0.226f, 0.159f, 6.0f, 52.2f},

        { 0.900f, 0.454f, 0.167f, 6.9f, 13.3f},

    };

    vanCollisionLayouts[2].deathZones = {

        { 0.033f, 0.454f, 0.280f, 5.8f, 95.7f},

        { 0.971f, 0.776f, 0.074f, 7.3f, -90.6f},

    };

    vanCollisionLayouts[3].walkFreeSegments = {

        { 0.350f, 0.046f, 0.630f, 6.6f, 0.6f},

        { 0.718f, 0.230f, 0.159f, 6.0f, 47.6f},

        { 0.873f, 0.476f, 0.220f, 6.3f, 14.3f},

    };

    vanCollisionLayouts[3].deathZones = {

        { 0.029f, 0.504f, 0.233f, 6.3f, -85.2f},

        { 0.972f, 0.765f, 0.074f, 7.3f, -93.6f},

    };

    vanCollisionLayouts[4].walkFreeSegments = {

        { 0.340f, 0.050f, 0.595f, 5.7f, 0.6f},

        { 0.696f, 0.247f, 0.183f, 6.0f, 44.1f},

        { 0.865f, 0.506f, 0.243f, 6.3f, 13.7f},

    };

    vanCollisionLayouts[4].deathZones = {

        { 0.029f, 0.532f, 0.252f, 5.9f, 96.8f},

        { 0.972f, 0.765f, 0.074f, 7.3f, 84.2f},

    };

    // Drone variants: single sprite each, no wheels. Flies toward the
    // player, facing and moving left (opposite of the background
    // Helicopter's left-to-right flyby in Background.cpp).
    droneBodyTextures.resize(NUM_DRONE_VARIANTS);
    droneCollisionSpecs.resize(NUM_DRONE_VARIANTS);
    droneTargetHeights.resize(NUM_DRONE_VARIANTS);

    for (int i = 0; i < NUM_DRONE_VARIANTS; i++) {
        std::string bodyPath = "assets/Drone" + std::to_string(i + 1) + ".png";
        droneBodyTextures[i].loadFromFile(bodyPath);
        if (droneBodyTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- this drone variant will be invisible/broken until this file exists.\n";
        }
    }

    // Per-variant target height.
    droneTargetHeights[0] = 35.0f; // Drone1
    droneTargetHeights[1] = 35.0f; // Drone2
    droneTargetHeights[2] = 50.0f; // Drone3

    // Nose death zone, measured from each drone's cropped silhouette
    // (fractions measured against the uncropped canvas would be thrown
    // off by the transparent padding around the propellers in the
    // source images).
    //
    // Drone1 (516x190) and Drone2 (507x190) share the same rig with the
    // nose camera low, near the bottom-left, where the front landing
    // leg meets the body. Drone3 (577x267) is a chunkier design with a
    // bigger lens housing, so its nose camera cluster sits higher and
    // further right on a proportionally taller body.
    //
    // Starting placement from reference art rather than the in-game
    // render; may need adjustment once seen on screen.
    droneCollisionSpecs[0] = { 0.150f, 0.76f, 0.045f, 10.0f, 0.0f }; // Drone1
    droneCollisionSpecs[1] = { 0.162f, 0.745f, 0.045f, 10.0f, 0.0f }; // Drone2
    droneCollisionSpecs[2] = { 0.247f, 0.465f, 0.060f, 13.0f, 0.0f }; // Drone3

    // 4-dot hitbox, placeholder: all 3 variants start with the same
    // corner placement (top-left, top-right, bottom-left, bottom-right,
    // each 15% in from its edge).
    droneDotSpecs.resize(NUM_DRONE_VARIANTS * 4);
    for (int i = 0; i < NUM_DRONE_VARIANTS; i++) {
        droneDotSpecs[i * 4 + 0] = { 0.15f, 0.15f, 10.0f }; // top-left
        droneDotSpecs[i * 4 + 1] = { 0.85f, 0.15f, 10.0f }; // top-right
        droneDotSpecs[i * 4 + 2] = { 0.15f, 0.85f, 10.0f }; // bottom-left
        droneDotSpecs[i * 4 + 3] = { 0.85f, 0.85f, 10.0f }; // bottom-right
    }

    // Drone sound loading happens in Game::init(), which owns sound
    // centrally.

    // Throwing car variants, trunk popped open. Visually and
    // collision-wise this is a Car, so it starts as a copy of
    // carWheelLayouts[0]/carCollisionLayouts[0], the base sedan
    // measurements, rather than a fresh set of measurements.
    throwingCarBodyTextures.resize(NUM_THROWING_CAR_VARIANTS);
    throwingCarWheelTextures.resize(NUM_THROWING_CAR_VARIANTS);
    throwingCarWheelLayouts.resize(NUM_THROWING_CAR_VARIANTS);
    throwingCarCollisionLayouts.resize(NUM_THROWING_CAR_VARIANTS);

    for (int i = 0; i < NUM_THROWING_CAR_VARIANTS; i++) {
        std::string bodyPath = "assets/ThrowingCarBody" + std::to_string(i + 1) + ".png";
        std::string wheelPath = "assets/ThrowingCarWheel" + std::to_string(i + 1) + ".png";
        throwingCarBodyTextures[i].loadFromFile(bodyPath);
        if (throwingCarBodyTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- this throwing-car variant will be invisible/broken until this file exists.\n";
        }
        throwingCarWheelTextures[i].loadFromFile(wheelPath);
        if (throwingCarWheelTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << wheelPath << " -- this throwing-car variant's wheels will be invisible/broken until this file exists.\n";
        }
    }

    throwingCarWheelLayouts[0] = { 0.809f, 0.858f, 0.204f, 0.860f, 0.153f };
    throwingCarWheelLayouts[1] = { 0.806f, 0.854f, 0.204f, 0.860f, 0.149f };
    throwingCarCollisionLayouts[0] = {



        0.842f, 0.489f, 0.291f, 5.0f, 11.4f,    // hood



        0.650f, 0.265f, 0.145f, 4.5f, 33.3f,   // windshield



        0.459f, 0.159f, 0.303f, 5.3f, 0.1f,    // roof



        0.178f, 0.324f, 0.291f, 4.0f, -18.5f,  // trunk



        6.3f, 61.1f, 4.8f, 69.0f,             // front death line



        9.5f, 7.4f, 5.4f, 7.5f



    };
    throwingCarCollisionLayouts[1] = {

        0.842f, 0.489f, 0.291f, 5.0f, 11.4f,    // hood

        0.650f, 0.265f, 0.145f, 4.5f, 33.3f,   // windshield

        0.459f, 0.159f, 0.303f, 5.3f, 0.1f,    // roof

        0.178f, 0.324f, 0.291f, 4.0f, -18.5f,  // trunk

        6.3f, 61.1f, 4.8f, 69.0f,             // front death line

        9.5f, 7.4f, 5.4f, 7.5f

    };

    // Box variants: single sprite each, no wheels. Uses a small "drop"
    // death zone rather than the full sprite bounds, same reasoning as
    // the drone's nose zone: a small hitbox is fairer to the player and
    // doesn't fully cover the box art when the debug collision overlay
    // draws it as a filled shape.
    boxTextures.resize(NUM_BOX_VARIANTS);
    boxCollisionSpecs.resize(NUM_BOX_VARIANTS);
    for (int i = 0; i < NUM_BOX_VARIANTS; i++) {
        std::string bodyPath = "assets/Box" + std::to_string(i + 1) + ".png";
        boxTextures[i].loadFromFile(bodyPath);
        if (boxTextures[i].getSize().x == 0) {
            std::cerr << "Warning: could not load " << bodyPath << " -- this box variant will be invisible/broken until this file exists.\n";
        }
    }

    // Centered, well inside the box's own edges: roughly a third of
    // its size. Both variants share the same starting spec.
    for (int i = 0; i < NUM_BOX_VARIANTS; i++) {
        boxCollisionSpecs[i] = { 0.5f, 0.5f, 0.35f, 16.0f, 0.0f };
    }

    // 4-dot hitbox, placeholder: same starting corner placement as
    // Drone above.
    boxDotSpecs.resize(NUM_BOX_VARIANTS * 4);
    boxDotSpecs[0 * 4 + 0] = {0.160f, 0.444f, 39.0f}; // top-left
    boxDotSpecs[0 * 4 + 1] = {0.479f, 0.240f, 37.0f}; // top-right
    boxDotSpecs[0 * 4 + 2] = {0.348f, 0.749f, 38.0f}; // bottom-left
    boxDotSpecs[0 * 4 + 3] = {0.686f, 0.592f, 38.0f}; // bottom-right
    boxDotSpecs[1 * 4 + 0] = {0.176f, 0.408f, 39.0f}; // top-left
    boxDotSpecs[1 * 4 + 1] = {0.428f, 0.269f, 37.0f}; // top-right
    boxDotSpecs[1 * 4 + 2] = {0.272f, 0.692f, 38.0f}; // bottom-left
    boxDotSpecs[1 * 4 + 3] = {0.599f, 0.508f, 38.0f}; // bottom-right

    texturesLoaded = true;
}

void Obstacle::loadCarSounds() {
    if (carSoundsLoaded) return;

    // Downlane car engine sound: one buffer shared by every car.
    carEngineBufferLoaded = carEngineBuffer.loadFromFile("assets/CarEngine1.wav");
    if (!carEngineBufferLoaded) std::cerr << "Warning: could not load assets/CarEngine1.wav\n";

    // Downlane car horn sounds: 3 variants. A 20% chance one of them
    // plays, picked at random, once a car reaches ~90% of its trip
    // toward the left edge (see the horn trigger in update()).
    carHornBuffersLoaded[0] = carHornBuffers[0].loadFromFile("assets/CarHorn1.wav");
    if (!carHornBuffersLoaded[0]) std::cerr << "Warning: could not load assets/CarHorn1.wav\n";
    carHornBuffersLoaded[1] = carHornBuffers[1].loadFromFile("assets/CarHorn2.wav");
    if (!carHornBuffersLoaded[1]) std::cerr << "Warning: could not load assets/CarHorn2.wav\n";
    carHornBuffersLoaded[2] = carHornBuffers[2].loadFromFile("assets/CarHorn3.wav");
    if (!carHornBuffersLoaded[2]) std::cerr << "Warning: could not load assets/CarHorn3.wav\n";

    carSoundsLoaded = true;
}

void Obstacle::setCarEngineVolume(float volume0to100) {
    carEngineVolume = volume0to100;
}

void Obstacle::setCarHornVolume(float volume0to100) {
    carHornVolume = volume0to100;
}

void Obstacle::setSlowMoAudio(float pitch, float volumeMultiplier) {
    slowMoAudioPitch = pitch;
    slowMoAudioVolumeMult = volumeMultiplier;
}

void Obstacle::setDroneSoundVolume(float volume0to100) {
    droneSoundVolume = volume0to100;
}

void Obstacle::loadDroneSound() {
    if (droneSoundBufferLoaded) return;
    droneSoundBufferLoaded = droneSoundBuffer.loadFromFile("assets/Drone.wav");
    if (!droneSoundBufferLoaded) std::cerr << "Warning: could not load assets/Drone.wav\n";
}

void Obstacle::pauseSounds() {
    if (engineSound && engineSound->getStatus() == sf::Sound::Status::Playing) engineSound->pause();
    if (droneSound && droneSound->getStatus() == sf::Sound::Status::Playing) droneSound->pause();
    if (hornSound && hornSound->getStatus() == sf::Sound::Status::Playing) hornSound->pause();
}

Obstacle::Obstacle(ObstacleType type, float x, float heightOverride, float groundYOverride, float boxTargetX, float boxTargetY, float boxSpeedOverride) : type(type), active(true), tireRotationDeg(0.0f),
    carVariant(0), carWheelRotationDeg(0.0f), hornRolled(false), spawnXPos(x),
    boxElapsedTime(0.0f), boxFlightDuration(0.9f), boxLaunchX(0.0f), boxLaunchY(0.0f), boxArcTargetX(0.0f), boxArcTargetY(0.0f), boxArcComplete(false),
    boxVelocityX(0.0f), boxVelocityY(0.0f), boxGrounded(false) {

    loadTextures();
    loadCarSounds();

    if (type == ObstacleType::Car) {
        // Random variant, never the same as the immediately preceding one.
        carVariant = std::rand() % NUM_CAR_VARIANTS;
        if (carVariant == lastCarVariant) {
            carVariant = (carVariant + 1) % NUM_CAR_VARIANTS; // never the same variant twice (or more) in a row
        }
        lastCarVariant = carVariant;

        sprite.emplace(carBodyTextures[carVariant]);

        // Target height: normally 95px. The scripted scene overrides
        // this to 176px for the dramatic Car->Jeep->Van height chain.
        float carTargetHeight = (heightOverride > 0.0f) ? heightOverride : carTargetHeights[carVariant];
        float scale = (carTargetHeight / carBodyTextures[carVariant].getSize().y) * OBSTACLE_SCALE_MULTIPLIER;
        sprite->setScale({scale, scale});

        // Ground line, per-variant (carGroundYs[carVariant]), same as
        // Jeep/Van below. Used for both normal spawns and the scripted
        // scene.
        float carGroundY = carGroundYs[carVariant];
        float carY = carGroundY - carTargetHeight;
        sprite->setPosition({x, carY});

        // Wheels: placed using this variant's measured layout
        // fractions, relative to the body's actual current bounds.
        const CarWheelLayout& layout = carWheelLayouts[carVariant];
        sf::FloatRect bodyBounds = sprite->getGlobalBounds();

        float wheelDiameter = bodyBounds.size.x * layout.wheelDiameterFrac;

        frontWheelSprite.emplace(carWheelTextures[carVariant]);
        rearWheelSprite.emplace(carWheelTextures[carVariant]);

        float wheelScale = wheelDiameter / carWheelTextures[carVariant].getSize().x;
        frontWheelSprite->setScale({wheelScale, wheelScale});
        rearWheelSprite->setScale({wheelScale, wheelScale});

        // Centered origin so each wheel spins around its own middle.
        sf::Vector2u wheelTexSize = carWheelTextures[carVariant].getSize();
        frontWheelSprite->setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});
        rearWheelSprite->setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});

        // Fraction-based position, then the fine-tune pixel nudges
        // from the layout added on top; see CarWheelLayout in
        // Obstacle.h.
        frontWheelSprite->setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.frontXFrac + layout.frontOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.frontYFrac + layout.frontOffsetY
        });
        rearWheelSprite->setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.rearXFrac + layout.rearOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.rearYFrac + layout.rearOffsetY
        });

        // Shadow: see the member comment in Obstacle.h. Computed once
        // here from this same layout data, reused every frame. Sized
        // off the full body width, not the wheelbase, so it stretches
        // close to bumper-to-bumper like a real 2D contact shadow
        // instead of stopping short at the wheel centers.
        shadowSprite.emplace(shadeTexture);
        shadowWidthFrac = 1.05f; // full body width + 5% overhang
        shadowCenterXFrac = 0.5f; // centered on the body, not the wheelbase midpoint

        // Engine sound: single shared buffer, own sf::Sound instance so
        // it can play independently alongside every other car's engine
        // sound. Not looping: the clip is authored to match roughly one
        // car's pass-by, with fade included in the file itself. Plays
        // once at flat volume, no code-side envelope.
        if (carEngineBufferLoaded) {
            engineSound.emplace(carEngineBuffer);
            engineSound->setLooping(false);
            engineSound->setVolume(carEngineVolume * slowMoAudioVolumeMult);
            engineSound->setPitch(slowMoAudioPitch);
            engineSound->play();
        }

    } else if (type == ObstacleType::Jeep || type == ObstacleType::DownlaneVan) {
        // Jeep and DownlaneVan share this branch: both use the same
        // wiring as Car above (variant roll, body+wheel sprites,
        // flat-top collision, engine sound). Only the texture/layout
        // tables and target size/ground differ, picked via isJeep.
        bool isJeep = (type == ObstacleType::Jeep);
        int numVariants = isJeep ? NUM_JEEP_VARIANTS : NUM_DOWNLANE_VAN_VARIANTS;
        int& lastVariant = isJeep ? lastJeepVariant : lastVanVariant;

        carVariant = std::rand() % numVariants;
        if (carVariant == lastVariant) {
            carVariant = (carVariant + 1) % numVariants; // never the same variant twice (or more) in a row
        }
        lastVariant = carVariant;

        const sf::Texture& bodyTex = isJeep ? jeepBodyTextures[carVariant] : vanBodyTextures[carVariant];
        const sf::Texture& wheelTex = isJeep ? jeepWheelTextures[carVariant] : vanWheelTextures[carVariant];
        sprite.emplace(bodyTex);

        // Jeep/Van target height, independent of the sedan (627x176,
        // target 95px tall). Jeep canvas 900x281, Van canvas 1100x386:
        // both bigger vehicles, so bigger target heights.
        float normalHeight = isJeep ? jeepTargetHeights[carVariant] : vanTargetHeights[carVariant];
        float targetHeight = (heightOverride > 0.0f) ? heightOverride : normalHeight;
        float scale = (targetHeight / bodyTex.getSize().y) * OBSTACLE_SCALE_MULTIPLIER;
        sprite->setScale({scale, scale});

        // Ground alignment: per-variant (jeepGroundYs/vanGroundYs, same
        // pattern as Car above), used for both normal spawns and the
        // scripted scene.
        const float downlaneGroundBottom = isJeep ? jeepGroundYs[carVariant] : vanGroundYs[carVariant];
        sprite->setPosition({x, downlaneGroundBottom - targetHeight});

        // Wheels: same fraction-based placement as Car.
        const CarWheelLayout& layout = isJeep ? jeepWheelLayouts[carVariant] : vanWheelLayouts[carVariant];
        sf::FloatRect bodyBounds = sprite->getGlobalBounds();

        float wheelDiameter = bodyBounds.size.x * layout.wheelDiameterFrac;

        frontWheelSprite.emplace(wheelTex);
        rearWheelSprite.emplace(wheelTex);

        float wheelScale = wheelDiameter / wheelTex.getSize().x;
        frontWheelSprite->setScale({wheelScale, wheelScale});
        rearWheelSprite->setScale({wheelScale, wheelScale});

        sf::Vector2u wheelTexSize = wheelTex.getSize();
        frontWheelSprite->setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});
        rearWheelSprite->setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});

        frontWheelSprite->setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.frontXFrac + layout.frontOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.frontYFrac + layout.frontOffsetY
        });
        rearWheelSprite->setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.rearXFrac + layout.rearOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.rearYFrac + layout.rearOffsetY
        });

        // Shadow: see the member comment in Obstacle.h. Computed once
        // here from this same layout data, reused every frame. Sized
        // off the full body width, not the wheelbase, so it stretches
        // close to bumper-to-bumper like a real 2D contact shadow
        // instead of stopping short at the wheel centers.
        shadowSprite.emplace(shadeTexture);
        shadowWidthFrac = 1.05f; // full body width + 5% overhang
        shadowCenterXFrac = 0.5f; // centered on the body, not the wheelbase midpoint

        // Engine sound: reuses the same shared downlane car engine
        // buffer as the sedans, with its own sf::Sound instance per
        // car, same as Car above.
        if (carEngineBufferLoaded) {
            engineSound.emplace(carEngineBuffer);
            engineSound->setLooping(false);
            engineSound->setVolume(carEngineVolume * slowMoAudioVolumeMult);
            engineSound->setPitch(slowMoAudioPitch);
            engineSound->play();
        }

    } else if (type == ObstacleType::Drone) {
        // Random variant, same "never repeat last" pattern as
        // everything else.
        carVariant = std::rand() % NUM_DRONE_VARIANTS;
        if (carVariant == lastDroneVariant) {
            carVariant = (carVariant + 1) % NUM_DRONE_VARIANTS;
        }
        lastDroneVariant = carVariant;

        sprite.emplace(droneBodyTextures[carVariant]);

        // Per-variant target height (droneTargetHeights above).
        float droneTargetHeight = droneTargetHeights[carVariant];
        float scale = (droneTargetHeight / droneBodyTextures[carVariant].getSize().y) * OBSTACLE_SCALE_MULTIPLIER;
        sprite->setScale({scale, scale});

        // Flight altitude: unlike ground vehicles, the drone is
        // airborne, so this is a flat Y position for the sprite's top
        // edge rather than a "bottom touches the ground" line.
        float droneGroundY = 550.0f;
        sprite->setPosition({x, droneGroundY});

        // Shadow: no wheelbase to measure since it's airborne, so this
        // uses a fixed fraction of the body width instead, centered
        // (shadowCenterXFrac 0.5). The Y position is handled separately
        // in draw() below: it's pinned to the ground line rather than
        // the body's own bottom edge, so the shadow stays on the road
        // while the drone flies above it.
        shadowSprite.emplace(shadeTexture);
        shadowWidthFrac = 0.6f;
        shadowCenterXFrac = 0.5f;

        // Drone audio is owned centrally by Game: started ~1s before
        // spawn, cut ~0.5s after leaving the screen, with a
        // 70%->100% volume envelope. See Game::update().

        // No wheels: frontWheelSprite/rearWheelSprite stay unset
        // (nullopt) for this type; update()/draw() below never touch
        // them for Drone.

    } else if (type == ObstacleType::Tire) {
        sprite.emplace(tireTexture);

        // Target height in pixels, also scaled by
        // OBSTACLE_SCALE_MULTIPLIER like everything else.
        float scale = (80.0f / tireTexture.getSize().y) * OBSTACLE_SCALE_MULTIPLIER;
        sprite->setScale({scale, scale});

        // Origin centered so it spins around its own middle instead of
        // orbiting around a corner (needed since it visually rotates).
        sf::Vector2u texSize = tireTexture.getSize();
        sprite->setOrigin({ texSize.x / 2.0f, texSize.y / 2.0f });

        // Ground level. The sprite's origin is centered (unlike Car,
        // which uses a top-left origin), so this Y value is the tire's
        // center height, not its top edge, and needs a larger number
        // than a top-edge value would to land in the same visual spot.
        float tireGroundLevel = 634.0f;
        sprite->setPosition({x, tireGroundLevel});

        // Shadow: same shade sprite everything else uses. Placement
        // (ground line, left offset, width) is set directly in draw()
        // below instead, for rotation stability since this sprite spins.
        shadowSprite.emplace(shadeTexture);

    } else if (type == ObstacleType::ThrowingCar) {
        // Identical to Car above: own variant table/arrays, same
        // wheel and collision math, same engine sound. Differs in the
        // art (trunk open) and in how Game.cpp decides when to spawn
        // one (once the paired Box is 50% through its flight, not the
        // normal car spawn timer).
        carVariant = std::rand() % NUM_THROWING_CAR_VARIANTS;
        if (carVariant == lastThrowingCarVariant) {
            carVariant = (carVariant + 1) % NUM_THROWING_CAR_VARIANTS;
        }
        lastThrowingCarVariant = carVariant;

        sprite.emplace(throwingCarBodyTextures[carVariant]);

        // Per-variant target height, not a shared number: the two
        // source images have different canvas proportions. One is a
        // tightly-cropped sedan silhouette like the normal cars; the
        // other's canvas is taller because the propped-open trunk lid
        // strut extends above the roofline, so it needs a bigger
        // target height to end up the same visual size once scaled.
        //
        // Index [0] = ThrowingCarBody1.png, [1] = ThrowingCarBody2.png.
        float throwingCarTargetHeights[NUM_THROWING_CAR_VARIANTS] = { 103.0f, 108.0f };
        float throwingCarTargetHeight = throwingCarTargetHeights[carVariant];
        float scale = (throwingCarTargetHeight / throwingCarBodyTextures[carVariant].getSize().y) * OBSTACLE_SCALE_MULTIPLIER;
        sprite->setScale({scale, scale});

        // Ground anchor: the two variants have different target
        // heights, so anchoring both at the same top-Y would sink the
        // taller one's wheels below the road (bottom edge = topY +
        // height). Anchoring off a shared bottom line instead (670 =
        // normal Car's own top(570) + height(100)) keeps both variants'
        // wheels on the same ground line regardless of target height.
        const float throwingCarGroundBottom = 670.0f;
        sprite->setPosition({x, throwingCarGroundBottom - throwingCarTargetHeight});

        const CarWheelLayout& layout = throwingCarWheelLayouts[carVariant];
        sf::FloatRect bodyBounds = sprite->getGlobalBounds();

        float wheelDiameter = bodyBounds.size.x * layout.wheelDiameterFrac;

        frontWheelSprite.emplace(throwingCarWheelTextures[carVariant]);
        rearWheelSprite.emplace(throwingCarWheelTextures[carVariant]);

        float wheelScale = wheelDiameter / throwingCarWheelTextures[carVariant].getSize().x;
        frontWheelSprite->setScale({wheelScale, wheelScale});
        rearWheelSprite->setScale({wheelScale, wheelScale});

        sf::Vector2u wheelTexSize = throwingCarWheelTextures[carVariant].getSize();
        frontWheelSprite->setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});
        rearWheelSprite->setOrigin({wheelTexSize.x / 2.0f, wheelTexSize.y / 2.0f});

        frontWheelSprite->setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.frontXFrac + layout.frontOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.frontYFrac + layout.frontOffsetY
        });
        rearWheelSprite->setPosition({
            bodyBounds.position.x + bodyBounds.size.x * layout.rearXFrac + layout.rearOffsetX,
            bodyBounds.position.y + bodyBounds.size.y * layout.rearYFrac + layout.rearOffsetY
        });

        // Shadow: see the member comment in Obstacle.h. Computed once
        // here from this same layout data, reused every frame. Sized
        // off the full body width, not the wheelbase, so it stretches
        // close to bumper-to-bumper like a real 2D contact shadow
        // instead of stopping short at the wheel centers.
        shadowSprite.emplace(shadeTexture);
        shadowWidthFrac = 1.05f; // full body width + 5% overhang
        shadowCenterXFrac = 0.5f; // centered on the body, not the wheelbase midpoint

        if (carEngineBufferLoaded) {
            engineSound.emplace(carEngineBuffer);
            engineSound->setLooping(false);
            engineSound->setVolume(carEngineVolume * slowMoAudioVolumeMult);
            engineSound->setPitch(slowMoAudioPitch);
            engineSound->play();
        }

    } else if (type == ObstacleType::Box) {
        // Random variant, same "never repeat last" pattern.
        carVariant = std::rand() % NUM_BOX_VARIANTS;
        if (carVariant == lastBoxVariant) {
            carVariant = (carVariant + 1) % NUM_BOX_VARIANTS;
        }
        lastBoxVariant = carVariant;

        sprite.emplace(boxTextures[carVariant]);

        // Target height in pixels.
        float boxTargetHeight = 65.0f;
        float scale = (boxTargetHeight / boxTextures[carVariant].getSize().y) * OBSTACLE_SCALE_MULTIPLIER;
        sprite->setScale({scale, scale});

        // Launch height, roughly trunk height on a sedan (cars render
        // at y=570 with 100px height, so the trunk/mid-body sits
        // around y=570+50=620).
        boxLaunchY = 520.0f;
        boxLaunchX = x;
        sprite->setPosition({x, boxLaunchY});

        // Stored so update() can keep computing the arc from the
        // original launch/target points every frame; see the
        // boxElapsedTime comment in Obstacle.h for the arc formula.
        boxArcTargetX = boxTargetX;
        boxArcTargetY = boxTargetY;
        boxElapsedTime = 0.0f;
        boxArcComplete = false;

        // This throw's flight time is derived from actual distance at
        // a constant speed, not a fixed duration, so a longer throw
        // naturally takes proportionally longer at the same speed
        // rather than covering more ground in the same fixed time.
        // boxSpeedOverride (Game.cpp's growth-scaled speed for this
        // throw) is used when provided; falls back to the flat
        // BOX_SPEED constant otherwise (sentinel -1.0f = not provided).
        float effectiveBoxSpeedForFlight = (boxSpeedOverride > 0.0f) ? boxSpeedOverride : BOX_SPEED;
        boxFlightDuration = std::abs(boxTargetX - x) / effectiveBoxSpeedForFlight;
        boxVelocityX = (boxTargetX - x) / boxFlightDuration; // constant throughout, arc phase and post-arc fall alike
        boxGrounded = false;

        // No wheels, no engine sound: stay unset for this type.
    }
}

void Obstacle::update(float deltaTime, float speed) {
    if (!active) return;

    if (type == ObstacleType::Tire) {
        // Tire speed is a multiplier of the 'speed' parameter, the
        // same speed value tiles and every other obstacle use, so it
        // grows over time along with everything else.
        //
        // 1.71 keeps the tire's speed roughly 20% above the base
        // car-to-tile speed ratio.
        float tireBaseSpeed = 680.0f;
        float tireGrowthPassThrough = 7.08f; // tuned for +25% growth at the speed cap (774, from the 160.0f growth range)
        float tireSpeedValue = tireBaseSpeed + (speed - 750.0f) * tireGrowthPassThrough;
        sprite->move({ -tireSpeedValue * deltaTime, 0.0f });

        // Rotation speed in degrees/sec. A real rolling tire's spin
        // rate relates to its speed and radius, but a simple constant
        // is easier to tune by eye than exact physics. Negative because
        // it rolls leftward, spinning the opposite way from rightward.
        float tireSpinSpeed = -650.0f;
        tireRotationDeg += tireSpinSpeed * deltaTime;
        if (tireRotationDeg > 360.0f) tireRotationDeg -= 360.0f;
        if (tireRotationDeg < -360.0f) tireRotationDeg += 360.0f;
        sprite->setRotation(sf::degrees(tireRotationDeg));

    } else if (type == ObstacleType::Car || type == ObstacleType::Jeep || type == ObstacleType::DownlaneVan || type == ObstacleType::ThrowingCar) {
        // Jeep/DownlaneVan/ThrowingCar drive exactly like Car: same
        // closing speed, wheel spin, and horn-trigger logic, reusing
        // the same per-instance members (carVariant/
        // carWheelRotationDeg/frontWheelSprite/rearWheelSprite/
        // engineSound/hornSound).
        //
        // These cars drive the same direction as the player (forward),
        // just slower, so the player catches up to them naturally.
        // carOwnForwardSpeed is the car's own real speed, in the same
        // units as the world's speed; what shows on screen is the
        // difference (world speed - car's own speed). Example: world
        // at 500, car's own speed at 300 gives 200 of relative closing
        // speed. Raising carOwnForwardSpeed gives a gentler catch-up;
        // lowering it gives a faster one. If this is ever set higher
        // than the world's speed, the car appears to move backward
        // (toward the player from ahead).
        float carOwnForwardSpeed = 370.0f;
        float carSpeedValue = speed - carOwnForwardSpeed;
        float moveAmount = -carSpeedValue * deltaTime;

        sprite->move({ moveAmount, 0.0f });
        frontWheelSprite->move({ moveAmount, 0.0f });
        rearWheelSprite->move({ moveAmount, 0.0f });

        // Wheel spin is tied directly to the car's actual current speed
        // (unlike Tire's flat constant), so wheels visibly spin faster
        // as the car speeds up over a run. carWheelRotationFactor is
        // degrees of spin per pixel traveled.
        //
        // The actual closing speed (carSpeedValue, derived from
        // `speed`) barely grows over a run, by design, for fairness,
        // but that makes tires look static for a long game. This
        // amplifies however far `speed` has grown above its own base
        // by wheelSpeedGrowthBoost before feeding it into the spin
        // calc, so wheels visibly spin faster over time even though
        // the car's real closing speed barely changed. Purely visual;
        // no effect on gameplay danger.
        const float speedBaseForWheelBoost = 750.0f; // matches obstacleSpeedBase in Game.cpp
        const float wheelSpeedGrowthBoost = 4.02f; // tuned so wheel spin is ~30% faster at max game speed than at run start
        float wheelBoostFactor = 1.0f + std::max(0.0f, (speed - speedBaseForWheelBoost) / speedBaseForWheelBoost) * wheelSpeedGrowthBoost;

        float carWheelRotationFactor = 2.9f;
        // Wheels spin opposite to the car's facing (nose points right)
        // so they look like they're rolling forward, not backward.
        carWheelRotationDeg += carWheelRotationFactor * wheelBoostFactor * carSpeedValue * deltaTime;
        if (carWheelRotationDeg > 360.0f) carWheelRotationDeg -= 360.0f;
        if (carWheelRotationDeg < -360.0f) carWheelRotationDeg += 360.0f;
        frontWheelSprite->setRotation(sf::degrees(carWheelRotationDeg));
        rearWheelSprite->setRotation(sf::degrees(carWheelRotationDeg));

        // Fraction of the trip from spawn to the left edge, used for
        // the horn trigger only.
        float traveled = spawnXPos - sprite->getPosition().x;
        float totalTrip = spawnXPos - VIEW_LEFT_EDGE;
        float fraction = (totalTrip > 0.0f) ? (traveled / totalTrip) : 0.0f;

        // Flat volume, live-updated so a slider change applies
        // immediately. The fade shape lives in the .wav file itself.
        // Slow-motion pitch/volume multiplier layered on top, same as
        // every other gameplay sound in Game.cpp.
        if (engineSound) {
            engineSound->setVolume(carEngineVolume * slowMoAudioVolumeMult);
            engineSound->setPitch(slowMoAudioPitch);
        }

        // Horn trigger: fires once per car, only if the 20% chance
        // passes, when this car has covered ~90% of the trip from its
        // spawn point toward the left edge.
        if (!hornRolled) {
            if (fraction >= 0.90f) {
                hornRolled = true;
                if ((std::rand() % 100) < 20) {
                    // Horn1 is rarer than Horn2/Horn3: Horn1 gets a 5%
                    // slice, Horn2/Horn3 split the rest evenly.
                    int roll = std::rand() % 100;
                    int hornVariant = (roll < 5) ? 0 : (roll < 52 ? 1 : 2);
                    if (carHornBuffersLoaded[hornVariant]) {
                        hornSound.emplace(carHornBuffers[hornVariant]);
                        hornSound->setVolume(carHornVolume * slowMoAudioVolumeMult);
                        hornSound->setPitch(slowMoAudioPitch);
                        hornSound->play();
                    }
                }
            }
        }

    } else if (type == ObstacleType::Drone) {
        // Moves left (nose-first, since it faces left), straight toward
        // the player, with no rise, fall, or rotation. Uses the same
        // relative-speed model as Car (own forward speed subtracted
        // from world speed), with its own separate number: a lower
        // droneOwnForwardSpeed than Car's 370.0f means a bigger (world
        // speed - own speed) gap, so the drone closes in on the player
        // faster than cars do.
        float droneOwnForwardSpeed = 80.0f;
        const float droneGrowthBoost = 6.98f; // tuned for +25% growth by the REAL speed cap (774, from the 160.0f growth range)
        float droneSpeedValue = (750.0f - droneOwnForwardSpeed) + (speed - 750.0f) * droneGrowthBoost;
        sprite->move({ -droneSpeedValue * deltaTime, 0.0f });

    } else if (type == ObstacleType::Box) {
        // Same downlane ground line every ground vehicle sits on (665
        // bottom edge). Once the box's own bottom edge reaches this
        // line, it stops falling for good.
        const float boxGroundBottom = 665.0f;

        if (!boxGrounded) {
            if (!boxArcComplete) {
                // Arc phase: a direct position formula, not velocity
                // accumulation. Straight-line interpolation from launch
                // to target, with a sine bump subtracted (screen Y
                // decreases upward) that peaks at exactly
                // BOX_ARC_HEIGHT pixels at the midpoint of the flight.
                // Doubling BOX_ARC_HEIGHT exactly doubles the visible
                // rise, regardless of launch/target distance or flight
                // duration.
                boxElapsedTime += deltaTime;
                float T = boxFlightDuration;
                if (boxElapsedTime >= T) {
                    boxElapsedTime = T;
                    boxArcComplete = true;
                }
                float frac = boxElapsedTime / T;
                const float PI = 3.14159265358979f;
                float newX = boxLaunchX + (boxArcTargetX - boxLaunchX) * frac;
                float newY = boxLaunchY + (boxArcTargetY - boxLaunchY) * frac - BOX_ARC_HEIGHT * std::sin(PI * frac);
                sprite->setPosition({ newX, newY });

                if (boxArcComplete) {
                    // Carries the curve's instantaneous vertical
                    // velocity at t=T into the post-arc fall below, so
                    // there's no visible pop in speed when it switches
                    // from following the curve to falling: matches the
                    // curve's own derivative exactly at the handoff.
                    boxVelocityY = (boxArcTargetY - boxLaunchY) / T + BOX_ARC_HEIGHT * (PI / T);
                }
            } else {
                // Post-arc phase: real gravity. Whether the player was
                // hit right at the aimed point or dodged and this is
                // now past it, the box keeps falling for real from here
                // until it reaches the ground.
                boxVelocityY += BOX_GRAVITY * deltaTime;
                sprite->move({ boxVelocityX * deltaTime, boxVelocityY * deltaTime });
            }

            sf::FloatRect afterMove = sprite->getGlobalBounds();
            if (afterMove.position.y + afterMove.size.y >= boxGroundBottom) {
                // Snap the bottom edge exactly onto the ground line
                // rather than letting it overshoot into the road, then
                // switch to simple horizontal drift for the rest of its
                // time on screen, same movement everything else uses.
                sprite->setPosition({ afterMove.position.x, boxGroundBottom - afterMove.size.y });
                boxGrounded = true;
            }
        } else {
            sprite->move({ -speed * deltaTime, 0.0f });
        }

    } else {
        sprite->move({ -speed * deltaTime, 0.0f });
    }
}

void Obstacle::draw(sf::RenderWindow& window) {
    if (!active) return;

    if (type == ObstacleType::Car || type == ObstacleType::Jeep || type == ObstacleType::DownlaneVan || type == ObstacleType::ThrowingCar) {
        // Shadow drawn first, underneath everything. Position/size are
        // recalculated fresh from the body's current bounds every
        // frame, so it tracks the vehicle without any separate
        // per-frame movement code of its own.
        if (shadowSprite.has_value()) {
            sf::FloatRect bodyBounds = sprite->getGlobalBounds();
            sf::Vector2u shadeTexSize = shadeTexture.getSize();
            if (shadeTexSize.x > 0) {
                float shadowWidth = bodyBounds.size.x * shadowWidthFrac;
                float shadowScale = shadowWidth / shadeTexSize.x;
                shadowSprite->setScale({shadowScale, shadowScale});
                shadowSprite->setOrigin({shadeTexSize.x / 2.0f, shadeTexSize.y / 2.0f});
                shadowSprite->setPosition({
                    bodyBounds.position.x + bodyBounds.size.x * shadowCenterXFrac,
                    bodyBounds.position.y + bodyBounds.size.y + 8.0f // ground line = body's own bottom edge
                });
                window.draw(*shadowSprite);
            }
        }

        // Wheels drawn after the body: drawing them underneath relied
        // on the body art having a transparent wheel-well cutout lined
        // up exactly right, which isn't reliable. Drawing on top
        // guarantees the wheel is always visible regardless of the
        // body art's cutout.
        window.draw(*sprite);
        window.draw(*frontWheelSprite);
        window.draw(*rearWheelSprite);
    } else if (type == ObstacleType::Drone) {
        // Same shadow mechanism as the wheeled vehicles above, but the
        // Y is a fixed ground line (665, the same one every ground
        // vehicle sits on) instead of the body's own bottom edge: the
        // drone flies well above that, so its own bottom edge would put
        // the shadow floating in mid-air instead of on the road.
        if (shadowSprite.has_value()) {
            sf::FloatRect bodyBounds = sprite->getGlobalBounds();
            sf::Vector2u shadeTexSize = shadeTexture.getSize();
            if (shadeTexSize.x > 0) {
                const float droneShadowGroundY = 665.0f;
                float shadowWidth = bodyBounds.size.x * shadowWidthFrac;
                float shadowScale = shadowWidth / shadeTexSize.x;
                shadowSprite->setScale({shadowScale, shadowScale});
                shadowSprite->setOrigin({shadeTexSize.x / 2.0f, shadeTexSize.y / 2.0f});
                shadowSprite->setPosition({
                    bodyBounds.position.x + bodyBounds.size.x * shadowCenterXFrac,
                    droneShadowGroundY
                });
                window.draw(*shadowSprite);
            }
        }
        window.draw(*sprite);
    } else if (type == ObstacleType::Tire) {
        // Not using bodyBounds.position.y + bodyBounds.size.y here like
        // Car does: Tire's whole sprite spins to look like it's
        // rolling, and getGlobalBounds() is the bounding box of the
        // rotated sprite. A rotated square canvas's bounding box grows
        // as it turns (up to ~1.4x at 45deg) even though the visible
        // circle inside it doesn't change size, so anchoring off that
        // box would make the shadow grow/shrink and drift as the tire
        // spun instead of staying locked under it.
        //
        // Uses a fixed ground line (tireShadowGroundY) and the sprite's
        // rotation-stable center (getPosition(), since Tire's origin is
        // centered) instead.
        if (shadowSprite.has_value()) {
            sf::Vector2u shadeTexSize = shadeTexture.getSize();
            if (shadeTexSize.x > 0) {
                // Y is derived from the tire's own live position plus
                // radius (same "sits on the shadow" pattern as
                // Player's shadow), so it can't drift out of sync if
                // tireGroundLevel changes. Uses getLocalBounds() +
                // getScale() (not getGlobalBounds()) for the radius,
                // since local bounds/scale don't change with rotation.
                sf::FloatRect tireLocalBounds = sprite->getLocalBounds();
                float tireVisualRadius = (tireLocalBounds.size.y / 2.0f) * sprite->getScale().y;
                float tireShadowGroundY = sprite->getPosition().y + tireVisualRadius - 5.0f;
                float tireShadowXOffset = -12.0f;
                float tireShadowWidth = 70.0f;

                float shadowScale = tireShadowWidth / shadeTexSize.x;
                shadowSprite->setScale({shadowScale, shadowScale});
                shadowSprite->setOrigin({shadeTexSize.x / 2.0f, shadeTexSize.y / 2.0f});
                shadowSprite->setPosition({
                    sprite->getPosition().x + tireShadowXOffset,
                    tireShadowGroundY
                });
                window.draw(*shadowSprite);
            }
        }
        window.draw(*sprite);
    } else {
        window.draw(*sprite);
    }
}

bool Obstacle::isOffScreen() const {
    // Every obstacle type moves left, so one shared check works for
    // everyone. Uses getGlobalBounds() (not getPosition()) since that
    // correctly accounts for each sprite's own origin: Car uses a
    // top-left origin, while Tire uses a centered origin (needed for
    // rotation), and mixing getPosition() with getGlobalBounds() would
    // give a wrong answer specifically for Tire.
    sf::FloatRect b = sprite->getGlobalBounds();
    float rightEdge = b.position.x + b.size.x;
    return rightEdge < VIEW_LEFT_EDGE;
}

bool Obstacle::canBeRemoved() const {
    if (type != ObstacleType::Car && type != ObstacleType::Jeep && type != ObstacleType::DownlaneVan && type != ObstacleType::ThrowingCar) return isOffScreen();
    if (!isOffScreen()) return false;
    // Don't destroy the object (and cut its engine sound dead) until
    // the clip has actually finished playing on its own.
    if (engineSound && engineSound->getStatus() == sf::Sound::Status::Playing) return false;
    return true;
}

std::vector<RotatedRect> Obstacle::getCarWalkFreeLines() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    std::vector<RotatedRect> lines;
    const CarCollisionLayout& c = carCollisionLayouts[carVariant];

    // Car walk-free surface (4 sloped segments). Numbers come from this
    // car's own carCollisionLayouts entry; see the struct comment in
    // Obstacle.h.
    RotatedRect hood;
    hood.center = { b.position.x + b.size.x * c.hoodXFrac, b.position.y + b.size.y * c.hoodYFrac };
    hood.width = b.size.x * c.hoodWidthFrac;
    hood.height = c.hoodHeight;
    hood.rotationDeg = c.hoodRotationDeg;
    lines.push_back(hood);

    RotatedRect windshield;
    windshield.center = { b.position.x + b.size.x * c.windshieldXFrac, b.position.y + b.size.y * c.windshieldYFrac };
    windshield.width = b.size.x * c.windshieldWidthFrac;
    windshield.height = c.windshieldHeight;
    windshield.rotationDeg = c.windshieldRotationDeg;
    lines.push_back(windshield);

    RotatedRect roof;
    roof.center = { b.position.x + b.size.x * c.roofXFrac, b.position.y + b.size.y * c.roofYFrac };
    roof.width = b.size.x * c.roofWidthFrac;
    roof.height = c.roofHeight;
    roof.rotationDeg = c.roofRotationDeg;
    lines.push_back(roof);

    RotatedRect trunk;
    trunk.center = { b.position.x + b.size.x * c.trunkXFrac, b.position.y + b.size.y * c.trunkYFrac };
    trunk.width = b.size.x * c.trunkWidthFrac;
    trunk.height = c.trunkHeight;
    trunk.rotationDeg = c.trunkRotationDeg;
    lines.push_back(trunk);

    return lines;
}

sf::FloatRect Obstacle::getCarDeathLine() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    const CarCollisionLayout& c = carCollisionLayouts[carVariant];

    // Car death line: a plain vertical line on the front-facing side
    // (the right side, since the car's nose points right).
    float height = b.size.y - c.deathLineHeightReduction;

    return sf::FloatRect(
        { b.position.x + b.size.x - c.deathLineXOffsetFromRight - c.deathLineThickness, b.position.y + c.deathLineYOffset },
        { c.deathLineThickness, height }
    );
}

sf::FloatRect Obstacle::getCarBackDeathLine() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    const CarCollisionLayout& c = carCollisionLayouts[carVariant];

    // Back death line, at the trunk end (left side). Fully independent
    // from getCarDeathLine() above: its own X offset (measured from the
    // left edge), Y offset, thickness, and height reduction, since real
    // cars aren't symmetric front-to-back.
    float height = b.size.y - c.backDeathLineHeightReduction;

    return sf::FloatRect(
        { b.position.x + c.backDeathLineXOffsetFromLeft, b.position.y + c.backDeathLineYOffset },
        { c.backDeathLineThickness, height }
    );
}

std::vector<RotatedRect> Obstacle::getThrowingCarWalkFreeLines() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    std::vector<RotatedRect> lines;
    const CarCollisionLayout& c = throwingCarCollisionLayouts[carVariant];

    // Identical math to getCarWalkFreeLines() above, reading from
    // throwingCarCollisionLayouts instead of carCollisionLayouts.
    RotatedRect hood;
    hood.center = { b.position.x + b.size.x * c.hoodXFrac, b.position.y + b.size.y * c.hoodYFrac };
    hood.width = b.size.x * c.hoodWidthFrac;
    hood.height = c.hoodHeight;
    hood.rotationDeg = c.hoodRotationDeg;
    lines.push_back(hood);

    RotatedRect windshield;
    windshield.center = { b.position.x + b.size.x * c.windshieldXFrac, b.position.y + b.size.y * c.windshieldYFrac };
    windshield.width = b.size.x * c.windshieldWidthFrac;
    windshield.height = c.windshieldHeight;
    windshield.rotationDeg = c.windshieldRotationDeg;
    lines.push_back(windshield);

    RotatedRect roof;
    roof.center = { b.position.x + b.size.x * c.roofXFrac, b.position.y + b.size.y * c.roofYFrac };
    roof.width = b.size.x * c.roofWidthFrac;
    roof.height = c.roofHeight;
    roof.rotationDeg = c.roofRotationDeg;
    lines.push_back(roof);

    RotatedRect trunk;
    trunk.center = { b.position.x + b.size.x * c.trunkXFrac, b.position.y + b.size.y * c.trunkYFrac };
    trunk.width = b.size.x * c.trunkWidthFrac;
    trunk.height = c.trunkHeight;
    trunk.rotationDeg = c.trunkRotationDeg;
    lines.push_back(trunk);

    return lines;
}

sf::FloatRect Obstacle::getThrowingCarDeathLine() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    const CarCollisionLayout& c = throwingCarCollisionLayouts[carVariant];

    float height = b.size.y - c.deathLineHeightReduction;
    return sf::FloatRect(
        { b.position.x + b.size.x - c.deathLineXOffsetFromRight - c.deathLineThickness, b.position.y + c.deathLineYOffset },
        { c.deathLineThickness, height }
    );
}

sf::FloatRect Obstacle::getThrowingCarBackDeathLine() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    const CarCollisionLayout& c = throwingCarCollisionLayouts[carVariant];

    float height = b.size.y - c.backDeathLineHeightReduction;
    return sf::FloatRect(
        { b.position.x + c.backDeathLineXOffsetFromLeft, b.position.y + c.backDeathLineYOffset },
        { c.backDeathLineThickness, height }
    );
}

std::vector<RotatedRect> Obstacle::buildSegments(const std::vector<SegmentSpec>& specs, const sf::FloatRect& body) {
    std::vector<RotatedRect> result;
    result.reserve(specs.size());
    for (const auto& s : specs) {
        RotatedRect r;
        r.center = {
            body.position.x + body.size.x * s.xFrac + s.xOffset,
            body.position.y + body.size.y * s.yFrac + s.yOffset
        };
        r.width = body.size.x * s.widthFrac;
        r.height = s.height;
        r.rotationDeg = s.rotationDeg;
        result.push_back(r);
    }
    return result;
}

std::vector<RotatedRect> Obstacle::getJeepWalkFreeLines() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    return buildSegments(jeepCollisionLayouts[carVariant].walkFreeSegments, b);
}

std::vector<RotatedRect> Obstacle::getJeepDeathZones() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    return buildSegments(jeepCollisionLayouts[carVariant].deathZones, b);
}

std::vector<RotatedRect> Obstacle::getVanWalkFreeLines() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    return buildSegments(vanCollisionLayouts[carVariant].walkFreeSegments, b);
}

std::vector<RotatedRect> Obstacle::getVanDeathZones() const {
    sf::FloatRect b = sprite->getGlobalBounds();
    return buildSegments(vanCollisionLayouts[carVariant].deathZones, b);
}

Circle Obstacle::getTireCollisionCircle() const {
    sf::FloatRect b = sprite->getGlobalBounds();

    // Tire collision circle (not a rectangle). The center tracks the
    // sprite's current bounds; the radius is set slightly smaller than
    // the visible tire's half-width so the hitbox hugs the rubber
    // rather than the full padded image bounds.
    sf::Vector2f center = { b.position.x + b.size.x / 2.0f, b.position.y + b.size.y / 2.0f };
    float radius = (b.size.x / 2.0f) - 14.0f;

    return Circle{ center, radius };
}

RotatedRect Obstacle::getDroneDeathZone() const {
    sf::FloatRect b = sprite->getGlobalBounds();

    // Drone nose death zone: a single small rotated rect.
    return buildSegments({ droneCollisionSpecs[carVariant] }, b)[0];
}

std::vector<Circle> Obstacle::getDroneDotZones() const {
    sf::FloatRect b = sprite->getGlobalBounds();

    // Drone 4-dot hitbox: each dot's center is derived live from the
    // sprite's current bounds (b.position + b.size * fraction), same
    // as every other fraction-based hitbox in this file.
    std::vector<Circle> dots;
    dots.reserve(4);
    for (int i = 0; i < 4; i++) {
        const DotCollisionSpec& spec = droneDotSpecs[carVariant * 4 + i];
        sf::Vector2f center = { b.position.x + b.size.x * spec.xFrac, b.position.y + b.size.y * spec.yFrac };
        dots.push_back(Circle{ center, spec.radius });
    }
    return dots;
}

RotatedRect Obstacle::getBoxDeathZone() const {
    sf::FloatRect b = sprite->getGlobalBounds();

    // Box drop death zone: small, centered.
    return buildSegments({ boxCollisionSpecs[carVariant] }, b)[0];
}

std::vector<Circle> Obstacle::getBoxDotZones() const {
    sf::FloatRect b = sprite->getGlobalBounds();

    // Box 4-dot hitbox: same live-tracked convention as Drone's above.
    std::vector<Circle> dots;
    dots.reserve(4);
    for (int i = 0; i < 4; i++) {
        const DotCollisionSpec& spec = boxDotSpecs[carVariant * 4 + i];
        sf::Vector2f center = { b.position.x + b.size.x * spec.xFrac, b.position.y + b.size.y * spec.yFrac };
        dots.push_back(Circle{ center, spec.radius });
    }
    return dots;
}
