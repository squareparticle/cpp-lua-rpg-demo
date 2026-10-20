# RPG2002

RPG2002 is a modern reconstruction of a C++ RPG prototype originally developed around 2002. The original project began while learning DirectX from contemporary books and tutorials, then grew into custom RPG and game-engine code.

[![Watch the RPG2002 gameplay demo on YouTube](https://img.youtube.com/vi/_nqrkBS6GLw/hqdefault.jpg)](https://youtu.be/_nqrkBS6GLw)

[Watch the gameplay demo on YouTube](https://youtu.be/_nqrkBS6GLw).

## Documentation

Read the project's architecture and implementation documentation on
[GitHub Pages](https://squareparticle.github.io/cpp-lua-rpg-demo/site-docs/).

## Build and run

RPG2002-Ship currently targets 64-bit Windows. To build it from source, install:

- Visual Studio 2022 or later with **Desktop development with C++**
- The **C++ CMake tools for Windows** component, including CMake and Ninja
- Git

An internet connection is required for the first configuration. CMake fetches
the pinned Lua 5.5.1 and yaml-cpp 0.8.0 dependencies automatically; they do not
need to be installed separately.

Open **Developer PowerShell for Visual Studio**, change to the repository root,
and configure a release build once:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

Build the game with the repository's build script:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
```

Then run:

```powershell
.\build\RPG2002-Ship.exe
```

The build copies the required game configuration and assets beside the
executable. The controls are:

- **WASD** or **arrow keys**: move and navigate menus
- **E**: interact and advance dialogue
- **Escape**: open or close the main menu
- **I**: open the inventory directly
- **Enter**: confirm a menu or battle selection

Tutorial-derived platform code and unverified third-party assets have been excluded from this public repository. The surviving historical custom source is preserved under [`legacy/`](legacy/) for historical and reference purposes and is not expected to compile in its current form.

The new version is being rebuilt incrementally in modern C++ with a lightweight architecture. Lua is intended to replace the original custom scripting system. The current goal is a small, playable ship-based vertical slice rather than a general-purpose engine.

## What I changed

I architected and redesigned the engine topology, then hand-wrote the framework
to deepen my understanding of C++20. I designed the Lua integration and the
binding rules between Lua and C++, as well as the separation between the
platform, framework, and game layers.

I also wrote the story, gameplay, and cutscenes, drawing on the intent of the
original 2002 version. To move the project forward more quickly, I used AI to
help wire in dependencies and commodity boilerplate, including Direct2D,
audio, and application-launcher code. I created the artwork and enhanced it
using AI, used AI-generated sounds, and collaborated with AI on the automated
tests.

## License

The code and technical documentation, including my retained historical code
under `legacy/`, use the [MIT License](LICENSE). Artwork, audio, and narrative
content are excluded from MIT; rights are reserved to the extent applicable.
You may download, build, and play the demo for personal learning and evaluation,
including recruiter review. See [Licensing](LICENSING.md) for the full scope,
evaluation permission, and AI-generated material notes.

## Display configuration

Edit `config.yaml` before starting the game. `window.mode` accepts:

- `windowed` for a normal resizable window
- `fullscreen` for a borderless window covering the primary monitor
- `borderless` for a centered frameless window using the configured width and height

The `render` section defines the game's fixed internal resolution independently
of the window size. `scaling: fit` preserves its aspect ratio and uses black
letterbox or pillarbox bars; `scaling: stretch` fills the window. `filtering`
accepts `nearest` for crisp pixel-art scaling or `linear` for smooth scaling.
Window dimensions are logical 96-DPI units; Windows scales them for the active
display DPI. Render dimensions are always the exact pixel size of the fixed
game surface.

The `dialog.typewriterCharactersPerSecond` value controls how quickly dialogue
text is revealed. It accepts any finite number greater than zero; `42` is the
current default used by `config.yaml`.

## World camera

A scene map may configure the camera with a world-space window and an optional
object target:

```yaml
camera:
  target: playerObj
  window: { x: 0, y: 0, width: 1254, height: 1254 }
```

The camera derives its viewport size from the fixed render surface rather than
duplicating `config.yaml`. When assigned, it tries to center the target but
clamps its viewport to the configured world window. Near a boundary the camera
stops while the target continues moving on screen. Omitting `target` leaves the
camera free at its configured origin; omitting the entire section preserves the
default free camera at `(0, 0)`.

The player defines a scaled `cameraPoint` in its `.obj.yaml`. This visual
tracking point remains independent from collision probes, `interactionPoint`,
and `sortPointY`.

## Object templates

Scene object defaults are `.obj.yaml` files colocated with the character or
scene object they define and contain an `init` map. A scene makes a template
available through `resources.objects`, where it may override those defaults. A
compact entry in an Object layer then references the template with `obj` and
has final say over its properties.

A resolved `sprite` value may be either an ID from `resources.sprites` or a
direct path to an `.anim.yaml` file. The inheritance order is therefore:

```text
.obj.yaml init < resources.objects entry < layer objects entry
```

## Color-mask collision

Walking scenes load a CPU-readable PNG through `resources.collisionMasks`.
Black pixels block movement, white pixels allow movement, and other colors are
reserved as future trigger identifiers. Coordinates outside the mask are
blocked.

Movable objects define `collisionProbes` in their `.obj.yaml` defaults. Probe
coordinates are relative to the object's top-left position in unscaled sprite
frame pixels; the object's `scale` is applied before the mask is sampled.

Players and interactable objects separately define an `interactionPoint` in
the same unscaled sprite-frame coordinate system. Proximity is measured between
those authored points after each object's scale is applied. This keeps the
gameplay anchor (usually the character's feet) independent from collision
probes, which exist only to decide whether movement is legal.

## Object depth and world overlays

An Object layer may opt into top-down depth sorting with `sort: y`. Each
sortable object defines `sortPointY` in unscaled sprite-frame coordinates,
normally at its feet. The object scale is applied, and objects with smaller
world Y values render first. Equal values preserve their YAML placement order.

Interaction prompts remain owned by their interactable objects but are not
rendered with the sorted body sprites. `WorldScene` renders them in a final
world-overlay pass after all map layers, including foreground artwork. Future
dialog boxes and HUD elements will use their own GUI/HUD layer rather than this
world interaction-overlay pass.

## Hooks, rules, and Lua

`GameController` owns the persistent boolean `GlobalState` tags and the single
embedded Lua 5.5.1 state. World-scene interactable objects may define the fixed
hooks `INIT`, `DUTIES`, `INTERACT`, and `DESTROY`. A hook either names a Lua
script directly or an ordered `.rules.yaml` table.

Rule tables are evaluated from top to bottom, and the first match wins. A
`when` map may combine `all`, `any`, and `none` tag lists; a rule without
`when` is an unconditional fallback. C++ owns proximity, the E prompt, and the
edge-triggered input. Lua receives only game-level functions (`SetTag`,
`GetTag`, `DebugMessage`, and `Say`) and never polls the keyboard.
`Say(text[, portraitImageId[, exitSoundId]])`
uses `CurrentObjectId` as the speaker for diagnostic output, opens the modal
dialog window, and yields the interaction coroutine until the player closes
the dialog. The optional exit sound replaces the normal dialog-close sound for
that page, allowing a final page to end on a content-specific effect without
playing two sounds at once.

`DUTIES` resolves once when the object begins updating, then runs as a
long-lived Lua coroutine. `WaitForTrigger("name")` yields without polling;
later, an object-local C++ `FireDutiesTrigger("name")` resumes only the object
waiting for that exact trigger. Continuous movement, collision, animation, and
trigger detection remain C++ responsibilities. `DESTROY` fires through
explicit scene shutdown while Lua is still alive, never from a C++ destructor.

The current proof waits for `mop_right` and then `mop_left`; no NPC movement or
waypoint system fires those triggers yet. A future state-driven duties change
can stop the active coroutine, re-evaluate its existing rule table, and start
the newly selected script without changing the coroutine boundary.

## Menu and dialogue UI

`GameController` owns one screen-space `MenuWindow` and renders it after the
world scene as the current GUI layer. While it is modal, the world continues
updating with an empty `InputState`, so animation continues without moving the
player or firing another interaction.

The menu is a deliberately small RPG window rather than a general widget
framework. It supports wrapped dialogue, an optional image-resource portrait,
a vertical opening animation, typewriter reveal, and a minimal selectable
list. Table/column menus remain a follow-up for when the ship demo needs one.

DirectWrite creation and text layout stay inside `Graphics`. Each frame lays
out the complete string, then applies a transparent drawing effect to the
unrevealed character range. Final word wrapping therefore remains stable as
the typewriter advances. Direct2D draws the procedural blue window, borders,
portrait, clipping, and list highlight.

During dialogue, E first reveals all remaining characters. The bottom-right
`[E] Continue` prompt then appears, and a later E resumes the exact interaction
coroutine suspended by `Say`. If Lua immediately reaches another `Say`, its new
page replaces the old text without closing or replaying the open animation. If
Lua finishes or waits for something else, the dialog window closes.
