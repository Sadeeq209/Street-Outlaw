#include "ScoreManager.h"
#include <fstream>
#include <algorithm>

ScoreManager::ScoreManager() : score(0.0f), scoreMultiplier(1.0f), survivalTime(0.0f), highScore(0), milestoneTriggered(false) {
    loadHighScore();
}

void ScoreManager::reset() {
    score = 0.0f;
    scoreMultiplier = 1.0f;
    survivalTime = 0.0f;
    milestoneTriggered = false;
}

void ScoreManager::update(float deltaTime) {
    survivalTime += deltaTime;

    // Multiplier grows the longer the run survives, so score ticks up
    // faster later in a run. growthRate controls how quickly it ramps;
    // maxMultiplier caps how fast it can ever get.
    const float growthRate = 0.05f;   // multiplier gained per second survived
    const float maxMultiplier = 5.0f; // hard cap
    scoreMultiplier = std::min(1.0f + survivalTime * growthRate, maxMultiplier);

    // Points per second at multiplier 1.0.
    const float baseRate = 30.0f;
    score += baseRate * scoreMultiplier * deltaTime;
}

int ScoreManager::getScore() const {
    return static_cast<int>(score);
}

int ScoreManager::getHighScore() const {
    return highScore;
}

bool ScoreManager::checkAndUpdateHighScore() {
    int finalScore = getScore();
    if (finalScore > highScore) {
        highScore = finalScore;
        saveHighScore();
        return true;
    }
    return false;
}

bool ScoreManager::checkMilestoneCrossing() {
    if (!milestoneTriggered && getScore() > highScore) {
        milestoneTriggered = true;
        return true;
    }
    return false;
}

void ScoreManager::loadHighScore() {
    // Plain text file with a single integer, sitting next to the .exe.
    // If it doesn't exist yet (first ever run), highScore stays 0.
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void ScoreManager::saveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}
