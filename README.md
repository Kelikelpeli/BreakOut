# BreakOut

## Description

This project recreates a modernized version of the classic **BreakOut** video game, using **OpenGL** and an object-oriented architecture in **C++**. This project includes slight alterations to the original game's mechanics, aimed at providing a more balanced and engaging experience.

## Running the Game

To run the `.exe` file of the game outside of Visual Studio, make sure that the resources (images, sounds, textures) are in the same relative directory as the executable, as they are necessary for the correct functioning of the game. You can find the executables in the `bin` folder (both in Debug and Release mode).

## Instructions to Play

- **A/D**: Move the paddle left or right.
- **Space**: Launch the ball.
- **Enter**: Start the game.
- **W/S** (Menu Mode): Navigate through levels.
- Break all the blocks on the screen while managing lives and the timer. Use power-ups and extra lives to increase your chances of winning.

## Project Structure

- **`src` Folder**: Contains the core game classes, including mechanics for the paddle, ball, blocks, and power-ups.
- **`resources` Folder**: Includes assets such as textures, sounds, and fonts used in the game.
- **`shaders` Folder**: Stores OpenGL shader programs for graphical effects.
- **`Dependencies` Folder**: Includes third-party libraries essential for the project, such as irrKlang for audio.

## Implemented Features

- **Power-Ups**: Includes new mechanics like the extra ball power-up and the chaos modifier, providing unique challenges and opportunities.
- **Timer and Lives System**: A countdown timer and an extra life mechanic were added to balance gameplay.
- **Extra Lives**: Players can gain extra lives by breaking 10 consecutive blocks without losing a life.

## Additional Improvements

1. **Chaos Power-Up**: Adjusted functionality to reset upon losing a life, avoiding prolonged negative effects.
2. **Power-Up Generation**: Optimized logic to prevent overlaps and ensure clear gameplay.
3. **UI Enhancements**: Visible counters for time, lives, and special effects, ensuring clear and attractive visuals.

## References

- FontZone. (n.d.). OCR-A Extended. Retrieved from https://fontzone.net/font-details/ocr-a-extended
- LearnOpenGL. (n.d.). Breakout Game Tutorial. Retrieved from https://learnopengl.com/In-Practice/2D-Game/Breakout
- Professor-provided resources. (n.d.). Sound, Image, and Texture Resources [resourcesCAA3.rar].
- Ambiera. (n.d.). irrKlang Audio Library. Retrieved from https://www.ambiera.com/irrklang/downloads.html
- DeVries, J. (n.d.). LearnOpenGL Source Code. Retrieved from https://github.com/JoeyDeVries/LearnOpenGL/tree/master

## Credits

Developed by: **Raquel López Barceló**