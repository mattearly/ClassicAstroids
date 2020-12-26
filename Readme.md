# ClassicAstroids

Demo Astroids Game implementation using my [Ancient Archer](https://github.com/mattearly/AncientArcher) C++ Game Engine Library.

![Classic Astroids gameplay](astroids_demo_0.gif)

This project in particular makes use of:

- Positional sound effects
- Buffer cycled background music
- 3D Model Loading + Textures
- Texture Shaders with light and no light calculations
- Boolean Sphere Colliding
- Top down Perspective Camera
- Geometry and Trig calculations for game movements flow
- Random enemy direction and spin using [rand](https://github.com/mattearly/rand) (which is basically my C++ dll library of mersenne twister + chrono frontend)

## Controls

- A - turn left  
- D - turn right  
- W - advance forward  
- Space/Left Click - fire lazer


## Goal

- Win condition: You can't truely win, sure kill all the astroids and then there is nothing left to do, just restart the program.

- Make changes to the code, run again. It's really just to test the AAEngine and get used to the Library. If you do something that you would consider an improvement and want to show off a little, feel free to make a pull request.

## How to run (Windows only right now)

### Prerequisites

1. Windows 10
2. [Microsoft Visual Studio](https://visualstudio.microsoft.com/) with C++ workflow
3. Install required libraries for [Ancient Archer](https://github.com/mattearly/AncientArcher) with [vcpkg](https://github.com/microsoft/vcpkg).

### Run

1. Clone this repo with `--recursive` (or use [Fork](https://git-fork.com/) (recommended)) to automatically include required submodules [Ancient Archer](https://github.com/mattearly/AncientArcher) and [rand](https://github.com/mattearly/rand).
2. Open the solution file (ClassicAstroids.sln) with [Microsoft Visual Studio](https://visualstudio.microsoft.com/).
3. Make sure ClassicAstroids is the the default startup project and press Run/Local Windows Debugger (F5 Hotkey)
