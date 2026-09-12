# Street Outlaw

A 2D endless runner built from scratch in C++ using SFML 3. Play as an outlaw on the run through the city streets, dodging traffic, drones, and everything else in your way.

**[Play it now on itch.io](https://sadeeq209.itch.io/street-outlaw)**

<!-- Add a gameplay screenshot or GIF here once available, e.g.: -->
<!-- ![Gameplay](screenshots/gameplay.gif) -->

## About

Street Outlaw is a solo-developed endless runner focused on tight, readable gameplay and a level of production polish not typical of a first solo project: a scripted cinematic intro, six independently-timed hazard types, and a large library of vehicle variants with individually tuned collision and animation data.

## Features

- **Six hazard types**, each running on its own independent spawn timer with arm/countdown phases and mutual-exclusion logic so hazards never overlap unfairly
- **40+ car variants**, each with hand-tuned collision boxes and wheel positioning
- **A scripted cinematic intro** before every run: a phase-based state machine driving a title card, an NPC-triggered chase, and a timed alarm sequence, synchronized against real audio durations instead of guessed delays
- **A perceived-causality thrown-obstacle sequence**: a projectile that starts moving before its source vehicle even appears on screen, selling a convincing on-screen throw
- **Data-driven wheel-position mirroring**: a second traffic lane's wheel data is auto-generated from a single source of truth, eliminating manual duplication across every vehicle variant
- **Randomized jump variety**: jumps occasionally break into a front flip, back flip, or side flip
- Front/back knockback on death that reacts correctly to which side of a vehicle was hit

## Controls

| Key | Action |
|---|---|
| `UP` | Jump (sometimes a flip) |
| `DOWN` (in air) | Roll |
| `DOWN` (on ground) | Slide |
| `ESC` | Pause |

## Built With

- **C++**
- **SFML 3**
- Code::Blocks

## Building from Source

1. Install [SFML 3](https://www.sfml-dev.org/) and set it up with Code::Blocks (or your preferred C++ IDE/toolchain).
2. Clone this repository.
3. Open the project in Code::Blocks and build.
4. Make sure the `assets` folder sits in the same directory as the compiled executable before running.

## Status

Released and available now on [itch.io](https://sadeeq209.itch.io/street-outlaw). Actively maintained; updates and fixes are pushed as they're found.

## Author

**Abubakar Babandi**
[GitHub](https://github.com/Sadeeq209) | abubakarbabandi867@gmail.com
