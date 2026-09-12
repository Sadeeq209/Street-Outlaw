#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

class ScoreManager {
public:
    ScoreManager();

    void reset();               // call at the start of each run
    void update(float deltaTime);

    int getScore() const;
    int getHighScore() const;

    // Call once, right when the game actually ends. Returns true if
    // this run beat the previous high score (and saves it to disk).
    bool checkAndUpdateHighScore();

    // Call every frame during gameplay. Returns true exactly once per
    // run, the instant the current score first crosses the existing
    // high score, used to trigger the milestone sound/blink.
    bool checkMilestoneCrossing();

private:
    float score;             // raw accumulator (float for smooth ticking)
    float scoreMultiplier;   // grows over survival time, see update()
    float survivalTime;
    int highScore;
    bool milestoneTriggered; // prevents checkMilestoneCrossing() from firing more than once per run

    void loadHighScore();
    void saveHighScore();
};

#endif