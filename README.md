# CG Robot Arm

Built with CMake and edited within Visual Studio on Windows.

## How to Build & Run
1. Download CMake and OpenGL.
2. Download tutorial code from [website]http://www.opengl-tutorial.org/download/.
3. Drag and drop `common` folder files from this repo to the downloaded `common` folder.
4. Drag and drop `source` folder files from this repo to the downloaded `misc05_picking` folder.
5. Move the `models` folder from this repo to `misc05_picking` folder.
6. Make a separate `build` folder to use for CMake.
7. Open CMake and add pathnames to respective folders.
8. Open Visual Studio code and build `misc05_picking_easy` project.
9. Open the executable.
10. Interact with program!

## Interaction Keys
1. Pen: Select the pen using key `p`. The pen rotates when the arrow keys are pressed, and `←`, `→`, `↑`, `↓` are longitude and latitude rotations, and `shift + ←` and `shift + →` should twist the pen around its axis.
2. Base: Select the base using key `b`. The whole model slides on the XZ plane according to the arrow keys.
3. Top: Select the top using key `t`. The top, arms and pen rotate around Y axis when using the left and right arrow keys.
4. Arm1: Select arm1 using key `1`. The arm (and the other connected arm and pen) rotates up and down when using the arrow keys.
5. Arm2: Select Arm2 using key `2`. The arm (and pen) rotate up and down when using the arrow keys.
