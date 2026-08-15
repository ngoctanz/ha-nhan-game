# Funny Game — 2D Cartoon Game

Một game 2D cartoon nhỏ tập trung vào **animation, movement, combat vui nhộn và tương tác vật lý đơn giản**.

Tech stack:

```text id="zhuy8k"
C++20
+
raylib
+
CMake
+
Ninja
```

Target platforms:

```text id="j5x5md"
Linux / Fedora
macOS
Windows
```

Asset source:

```text id="8yyhlx"
Adobe Animate (.fla)
        ↓
PNG Sprite Sheet
        ↓
raylib
```

---

# 1. Goals

Prototype đầu tiên cần:

* Character 2D
* Idle animation
* Walk / Run
* Jump
* Attack
* Hit
* NPC
* Knockback
* Simple collision
* Camera
* Một map nhỏ
* Một vài interaction vui

Không cần:

* Backend
* Database
* Authentication
* Multiplayer
* Save system
* Complex AI
* Inventory

Mục tiêu là:

> Có một character hoạt hình chạy quanh map, thực hiện action và tương tác với NPC/object một cách vui nhộn.

---

# 2. Architecture

High-level architecture:

```text id="7i6fcp"
┌──────────────────────────────┐
│          Application         │
│                              │
│          main.cpp            │
└──────────────┬───────────────┘
               │
               ▼
        ┌─────────────┐
        │    Game     │
        └──────┬──────┘
               │
        ┌──────┴──────┐
        │  Game Loop  │
        └──────┬──────┘
               │
     ┌─────────┼─────────┐
     ▼         ▼         ▼

   Input     Update     Render
     │         │          │
     ▼         ▼          ▼
  Player    Entities    raylib
              │
        ┌─────┼─────┐
        ▼     ▼     ▼
       NPC  Physics Animation
```

Main loop:

```text id="ytadkt"
INPUT
  ↓
UPDATE
  ↓
COLLISION
  ↓
ANIMATION
  ↓
RENDER
  ↓
repeat
```

Target:

```text id="k7hjv5"
60 FPS
```

Character animation có thể chạy độc lập ở:

```text id="bshp8f"
8–12 FPS
```

---

# 3. Project Structure

Ban đầu:

```text id="3rpe77"
funny-game/
│
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── assets/
│   ├── characters/
│   │   ├── player/
│   │   └── npc/
│   │
│   ├── environment/
│   ├── effects/
│   ├── sounds/
│   └── music/
│
└── src/
    └── main.cpp
```

Khi project lớn hơn:

```text id="2n0tp9"
funny-game/
│
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── assets/
│
├── include/
│   └── game/
│       ├── Game.hpp
│       ├── Player.hpp
│       ├── NPC.hpp
│       └── Animation.hpp
│
└── src/
    ├── main.cpp
    ├── Game.cpp
    ├── Player.cpp
    ├── NPC.cpp
    └── Animation.cpp
```

Không dựng toàn bộ architecture ngay từ đầu.

Prototype đầu tiên chỉ cần:

```text id="53r3e3"
main.cpp
```

Sau đó mới refactor.

---

# 4. Dependencies

System dependencies:

```text id="uz32xq"
gcc / g++
CMake
Ninja
Git
```

Fedora:

```bash id="23pofx"
sudo dnf install gcc-c++ cmake ninja-build git
```

Verify:

```bash id="o2v1xl"
g++ --version
cmake --version
ninja --version
git --version
```

Không cần cài raylib global.

raylib được CMake tải riêng cho project.

---

# 5. CMake

`CMakeLists.txt`:

```cmake id="z9drc8"
cmake_minimum_required(VERSION 3.20)

project(
    funny_game
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(
    funny-game
    src/main.cpp
)

target_link_libraries(
    funny-game
    PRIVATE
    raylib
)
```

Dependency sẽ nằm trong build directory.

Không commit dependency vào Git.

---

# 6. Build

Configure:

```bash id="ll9fj3"
cmake -S . -B build -G Ninja
```

Build:

```bash id="23zkdh"
cmake --build build
```

Run:

```bash id="98m9ve"
./build/funny-game
```

Development loop:

```text id="o23apn"
VS Code
   ↓
edit C++
   ↓
cmake --build build
   ↓
./build/funny-game
   ↓
test
```

Sau lần configure đầu tiên, thường chỉ cần:

```bash id="3s6zrc"
cmake --build build && ./build/funny-game
```

---

# 7. Clean

Build artifacts và raylib dependency nằm trong:

```text id="qwe61g"
build/
```

Clean toàn bộ:

```bash id="xiyw3p"
rm -rf build
```

Configure lại:

```bash id="zzd3hj"
cmake -S . -B build -G Ninja
```

Không cần uninstall raylib khỏi Fedora vì raylib không được cài global.

---

# 8. Hello Game

Initial `src/main.cpp`:

```cpp id="l1ljkp"
#include <raylib.h>

int main()
{
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;

    InitWindow(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        "Funny Game"
    );

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText(
            "Funny Game",
            40,
            40,
            32,
            BLACK
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
```

Definition of Done:

```text id="vkixv1"
cmake build success
        ↓
./build/funny-game
        ↓
window opens
        ↓
"Funny Game"
```

---

# 9. Asset Pipeline

Original assets:

```text id="h4jmc4"
character.fla
```

Không load `.fla` trực tiếp trong game.

Pipeline:

```text id="j6e7o8"
character.fla
      ↓
Adobe Animate
      ↓
Export Sprite Sheet
      ↓
PNG
      ↓
assets/characters/player/
      ↓
raylib Texture2D
```

Preferred:

```text id="4nf4w5"
player.png
```

Sprite sheet:

```text id="vcn75q"
┌────┬────┬────┬────┐
│ I1 │ I2 │ I3 │ I4 │
├────┼────┼────┼────┤
│ W1 │ W2 │ W3 │ W4 │
├────┼────┼────┼────┤
│ A1 │ A2 │ A3 │ A4 │
└────┴────┴────┴────┘

I = Idle
W = Walk
A = Attack
```

Không dùng GIF làm runtime animation.

---

# 10. Sprite Loading

Load texture:

```cpp id="ux5chm"
Texture2D playerTexture =
    LoadTexture("assets/characters/player/player.png");
```

Unload khi game kết thúc:

```cpp id="5byoho"
UnloadTexture(playerTexture);
```

Lifecycle:

```text id="u2sv8d"
InitWindow()

LoadTexture()
LoadSound()
...

GAME LOOP

UnloadTexture()
UnloadSound()
...

CloseWindow()
```

---

# 11. Animation

Một animation bao gồm:

```cpp id="o3wjyo"
struct Animation
{
    int firstFrame;
    int frameCount;

    float frameDuration;
    float timer;

    int currentFrame;

    bool loop;
};
```

Update:

```text id="r8d7bs"
deltaTime
   ↓
timer += delta
   ↓
timer >= frameDuration?
   ↓
YES
   ↓
next frame
```

Sprite sheet source rectangle:

```cpp id="kgy96e"
Rectangle source{
    frameIndex * frameWidth,
    0.0f,
    frameWidth,
    frameHeight
};
```

Render:

```cpp id="mlnccn"
DrawTextureRec(
    texture,
    source,
    position,
    WHITE
);
```

---

# 12. Player States

Initial player states:

```cpp id="2ojppq"
enum class PlayerState
{
    Idle,
    Walk,
    Run,
    Jump,
    Fall,
    Attack,
    Hit,
    Dance
};
```

Flow:

```text id="g76kwk"
             IDLE
               │
        movement input
               ↓
             WALK
               │
             shift
               ↓
              RUN


IDLE / WALK / RUN
        │
       jump
        ↓
       JUMP
        ↓
       FALL
        ↓
       IDLE
```

Combat:

```text id="ss1z3u"
IDLE / WALK
     │
   attack
     ↓
   ATTACK
     │
animation complete
     ↓
    IDLE
```

---

# 13. Player

Future structure:

```cpp id="h80dtg"
class Player
{
public:

    void update(float deltaTime);

    void draw();

    void attack();

private:

    Vector2 position{};
    Vector2 velocity{};

    PlayerState state{
        PlayerState::Idle
    };

    Texture2D texture{};
};
```

Responsibilities:

```text id="3ljtba"
Player
├── position
├── velocity
├── state
├── animation
├── movement
└── attack
```

Player không quản lý toàn bộ game.

---

# 14. Input

Controls:

```text id="13z2ct"
A / Left

move left


D / Right

move right


Shift

run


Space

jump


J

attack


Q

dance
```

raylib:

```cpp id="ecg2l7"
if (IsKeyDown(KEY_A))
{
    // move left
}

if (IsKeyDown(KEY_D))
{
    // move right
}

if (IsKeyPressed(KEY_SPACE))
{
    // jump
}
```

---

# 15. Movement

Constants:

```cpp id="x4ndft"
constexpr float WALK_SPEED = 180.0f;

constexpr float RUN_SPEED = 280.0f;

constexpr float JUMP_FORCE = 450.0f;

constexpr float GRAVITY = 1000.0f;
```

Update:

```text id="wh7ahh"
velocity.y += gravity * delta

position += velocity * delta
```

Important:

Movement phải sử dụng delta time.

Không làm:

```cpp id="0gnt04"
position.x += 5;
```

Nên làm:

```cpp id="1wq3re"
position.x += velocity.x * deltaTime;
```

Gameplay khi đó không phụ thuộc FPS.

---

# 16. Collision

Prototype sử dụng simple rectangle collision.

```cpp id="2wktqh"
Rectangle playerCollider;
Rectangle groundCollider;
```

raylib:

```cpp id="4bl6a3"
CheckCollisionRecs(
    playerCollider,
    groundCollider
);
```

Ban đầu không cần physics engine riêng.

---

# 17. Combat

Architecture:

```text id="1snz78"
Player
   ↓
Attack
   ↓
Attack Animation
   ↓
Hitbox
   ↓
Collision
   ↓
NPC
   ↓
Hit
   ↓
Knockback
```

Attack hitbox:

```cpp id="fx6h7v"
Rectangle attackHitbox;
```

Check:

```cpp id="ey3m42"
if (
    CheckCollisionRecs(
        attackHitbox,
        npcCollider
    )
)
{
    npc.takeHit();
}
```

Hitbox chỉ active ở attack frames thích hợp.

---

# 18. NPC

Initial NPC states:

```cpp id="pt4e98"
enum class NPCState
{
    Idle,
    Walk,
    Hit,
    Down
};
```

Prototype NPC:

```text id="kh4x0h"
Idle
 ↓
Walk around

Player hits
 ↓
Hit
 ↓
Knockback
```

Không cần AI architecture phức tạp.

---

# 19. Knockback

Khi NPC bị hit:

```cpp id="poy2md"
velocity.x = direction * 350.0f;
velocity.y = -200.0f;
```

Flow:

```text id="qg8e8w"
        PUNCH

Player ─────────→ NPC

                  ↗
                NPC bay
```

Funny physics là một phần gameplay quan trọng.

---

# 20. Game Feel

Sau khi combat chạy ổn mới thêm:

```text id="5j8fbc"
Hit Stop
Screen Shake
Particles
Sound
Camera Punch
Funny animation
```

Attack feeling:

```text id="3nv4hy"
Attack
 ↓
Hit
 ↓
freeze 30–60 ms
 ↓
particle
 ↓
sound
 ↓
screen shake
 ↓
knockback
```

Không optimize game feel trước khi combat hoạt động.

---

# 21. Game Class

Khi `main.cpp` bắt đầu lớn, extract:

```text id="zjnjkp"
main.cpp
   ↓
Game
   ├── Player
   ├── NPC
   ├── World
   └── Camera
```

`main.cpp`:

```cpp id="1p61ad"
int main()
{
    Game game;

    game.run();

    return 0;
}
```

Game:

```cpp id="7ctj6c"
class Game
{
public:

    void run();

private:

    void update(float deltaTime);

    void draw();
};
```

---

# 22. Memory Strategy

Prototype ưu tiên RAII.

Tránh:

```cpp id="tg5ypr"
new Player();
```

nếu không cần.

Prefer:

```cpp id="g7plfl"
Player player;
```

hoặc container:

```cpp id="4e1mtg"
std::vector<NPC> npcs;
```

Nếu cần ownership động:

```cpp id="cyy11u"
std::unique_ptr<T>
```

Tránh raw owning pointer.

---

# 23. Performance Rules

Không optimize sớm.

Ưu tiên:

```text id="7bd81w"
Correctness
   ↓
Gameplay
   ↓
Profiling
   ↓
Optimization
```

Một số rule:

* Load texture một lần.
* Không load asset trong game loop.
* Không allocate liên tục mỗi frame.
* Dùng sprite sheet.
* Update bằng delta time.
* Giữ collision đơn giản.
* Batch asset hợp lý.
* Profile trước khi optimize.

---

# 24. Development Workflow

Daily loop:

```text id="7vld1k"
VS Code
   ↓
Edit
   ↓
Build
   ↓
Run
   ↓
Test
```

Command:

```bash id="4qfve6"
cmake --build build && ./build/funny-game
```

Nếu thay đổi `CMakeLists.txt` lớn:

```bash id="ck7z13"
cmake -S . -B build -G Ninja
cmake --build build
```

---

# 25. VS Code

Recommended extensions:

```text id="eeyrdg"
C/C++
CMake Tools
```

VS Code chỉ là editor.

Compiler:

```text id="o0on0e"
g++
```

Build configuration:

```text id="p24ogj"
CMake
```

Build executor:

```text id="n7z87c"
Ninja
```

Game framework:

```text id="ysdgm9"
raylib
```

---

# 26. Git

`.gitignore`:

```gitignore id="fpc5ip"
build/

.vscode/

.DS_Store

*.swp
```

Commit:

```text id="9h15ap"
src/
include/
assets/
CMakeLists.txt
README.md
```

Không commit:

```text id="2zk8a3"
build/
```

---

# 27. macOS

Same source code:

```text id="3s4a17"
Git repository
      │
      ├── Fedora
      │     ↓
      │   g++
      │
      └── macOS
            ↓
          Clang
```

Build trên Mac:

```bash id="jlj3tf"
cmake -S . -B build -G Ninja
cmake --build build
```

Binary phải build riêng theo OS.

Không copy Linux binary sang macOS.

---

# 28. Milestones

## Milestone 0

Toolchain hoạt động.

```text id="m3q9yb"
C++
 ↓
CMake
 ↓
raylib
 ↓
window
```

Definition of Done:

```text id="vryp6m"
Funny Game window opens.
```

---

## Milestone 1

Asset pipeline.

```text id="nyw8yl"
FLA
 ↓
PNG Sprite Sheet
 ↓
raylib
```

Implement:

```text id="k5m8az"
Player render
Idle animation
```

---

## Milestone 2

Movement.

Implement:

```text id="j0br76"
Walk
Run
Flip
Ground
Camera
```

---

## Milestone 3

Platform movement.

Implement:

```text id="s21zuh"
Gravity
Jump
Fall
Collision
```

---

## Milestone 4

Combat.

Implement:

```text id="ev5x13"
Attack
Hitbox
NPC
Hit
Knockback
```

Definition of Done:

```text id="vv8dfz"
Player punches NPC
        ↓
NPC flies
```

---

## Milestone 5

Game Feel.

Implement:

```text id="u0pyt5"
Sound
Particles
Hit Stop
Screen Shake
Funny Actions
Dance
```

---

# 29. Definition of Done

Prototype hoàn thành khi:

* Build thành công trên Fedora
* Build thành công trên macOS
* Player load từ sprite sheet
* Idle animation
* Walk animation
* Run
* Jump
* Fall
* Attack
* NPC
* Hit detection
* Knockback
* Simple world collision
* Camera
* Sound
* Một funny interaction
* Game chạy ổn ở 60 FPS

---

# 30. First Steps

Không bắt đầu bằng architecture lớn.

Làm đúng thứ tự:

```text id="03skct"
1. Create repository

2. Create:
   CMakeLists.txt
   src/main.cpp

3. Build raylib

4. Open window

5. Commit

6. Export ONE idle animation

7. Load sprite sheet

8. Render player

9. Animate idle

10. Add movement
```

Target đầu tiên:

```text id="92ztc9"
┌─────────────────────────────────┐
│                                 │
│                                 │
│               O                 │
│              /|\                │
│              / \                │
│                                 │
│─────────────────────────────────│
└─────────────────────────────────┘

          Funny Game
             60 FPS
```

Sau khi window + sprite + idle animation hoạt động, asset pipeline và rendering pipeline đã được xác nhận.

Từ đó mới bắt đầu xây gameplay.

