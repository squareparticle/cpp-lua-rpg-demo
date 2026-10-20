const REPO = "https://github.com/squareparticle/RPG2002-Ship";

const pages = {
  overview: {
    title: "Welcome aboard",
    eyebrow: "RPG2002-Ship · Architecture Atlas",
    lede: "In 2002 this was an unfinished Win32 RPG prototype: a ship, a hero and a scrolling deck. Twenty-four years later it has been rebuilt from scratch as a small slice you can actually finish. Find a key, beat two goblins, save the ship from a sea of lava, and help a rain mage put out the fires. This atlas shows how that slice works under the hood.",
    body: `
      ${shot("overview-deck.png", "The player and crew standing on the ship's deck.", "The boat deck, where every playthrough starts.")}
      <section class="prose">
        <h2>What's in the slice</h2>
        <p>Three scenes (the <strong>boat deck</strong>, the <strong>storage room</strong> and a <strong>battle arena</strong>), an opening and a closing cutscene, one short quest, a turn-based fight, an inventory, and save/load. It takes a few minutes to play. Making a few minutes of game work end to end touches almost every system a bigger RPG would need, and that's the point.</p>
        <h2>How it's built</h2>
        <p>The engine is C++ on Windows: Win32 for the window, Direct2D for drawing, XAudio2 for sound. The world itself (maps, characters, items, battle moves) is described in YAML files. The story logic, like "what does this crew member say now?", lives in small Lua scripts. Everything is deliberately sized for this one ship. When a piece of code only exists because a future game might want it, it doesn't go in.</p>
      </section>
      ${diagram("system-topology.svg", "The four layers", "Who is allowed to depend on whom.", "Application, Game, Framework and Platform stacked top to bottom with arrows pointing down. YAML and Lua feed into Game. Game may also call Platform directly.", "Read it top to bottom. Only Game knows this is an RPG; everything underneath could draw any 2D scene.")}
      <section>
        <h2>Where to go next</h2>
        <div class="page-grid">
          ${pageCard("play", "Play the slice", "Controls, the quest step by step, and the bit of game state each step changes.", "Start here")}
          ${pageCard("responsibilities", "C++ · Lua · YAML", "Follow one press of the E key from a YAML file, through C++ and Lua, and back.", "Open the door")}
          ${pageCard("runtime", "Runtime lifecycle", "What happens at startup, and what happens every frame after that.", "Follow a frame")}
          ${pageCard("cutscenes", "Cutscenes", "How a Lua script walks the crew around, steers the camera, and waits without freezing the game.", "Roll the camera")}
          ${pageCard("world-rendering", "World & rendering", "Layers, the camera, collision masks, and why the player has four different 'centers'.", "Walk the deck")}
          ${pageCard("state-save-load", "State & save/load", "Why a save file is tiny, and how the boat rebuilds itself from a handful of facts.", "Open a save")}
          ${pageCard("battle-flow", "Battle flow", "Leaving the world, fighting goblins turn by turn, and landing back on the exact pixel you left.", "Fight the goblins")}
          ${pageCard("asset-dependencies", "Asset dependency map", "How map, sprite, object and script files point at each other.", "Trace the files")}
          ${pageCard("legacy-correspondence", "Legacy correspondence", "What survived from the 2002 prototype, what was rebuilt, and what is brand new.", "Meet the original")}
          ${pageCard("source-index", "Source index & glossary", "Where things live in the repo, the names you'll see everywhere, and the Lua API.", "Find a file")}
        </div>
      </section>
    `
  },

  play: {
    title: "Play the slice",
    eyebrow: "Start here · Controls and quest",
    lede: "The game is a native Windows app, so it can't run in your browser. Instead, here is a full playthrough step by step, with the piece of game state each step flips. Those flags (called tags) come up again on almost every other page.",
    body: `
      <section class="prose">
        <h2>Controls</h2>
      </section>
      <table class="boundary-table controls-table">
        <thead><tr><th>Do this</th><th>Press</th></tr></thead>
        <tbody>
          <tr><td>Walk</td><td><kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> or the arrow keys</td></tr>
          <tr><td>Talk, pick up, open</td><td><kbd>E</kbd> when the E prompt appears over something</td></tr>
          <tr><td>Next line of dialogue</td><td><kbd>E</kbd> (the first press finishes the typing)</td></tr>
          <tr><td>Inventory</td><td><kbd>I</kbd></td></tr>
          <tr><td>Pause menu: save, load, status, quit</td><td><kbd>Esc</kbd></td></tr>
          <tr><td>Cutscenes</td><td>Just watch. Walking, the inventory and the pause menu come back when the scene ends.</td></tr>
          <tr><td>Move through menus and battle commands</td><td><kbd>W</kbd> <kbd>S</kbd> or <kbd>↑</kbd> <kbd>↓</kbd>, then <kbd>Enter</kbd></td></tr>
        </tbody>
      </table>
      ${diagram("game-map.svg", "Quest map", "The real deck, storage room and arena, with each step pinned where it happens.", "Quest map: the boat deck, storage room and battle arena, with seven numbered steps and the tag each one sets.", "Red areas are doorways and green areas are special ground. Both are painted into each scene's collision mask; the World page explains how.")}
      <section class="prose">
        <h2>The walkthrough</h2>
      </section>
      <ol class="quest-list">
        <li>
          <div><strong>Watch the opening.</strong>
          <p>The game starts with a short cutscene. The lookout walks down from the helm to the red carpet and tells you the ship is entering Hell's Gate, the storage lantern has gone dark, and the storage room is locked. When they finish, a key appears somewhere on the deck and you're free to move.</p>
          <p class="tag-line">sets <code>mission_storageLantern</code></p></div>
          ${shot("play-01-opening.gif", "The opening cutscene: a crew member walks down from the helm and explains the storage lantern mission.", "Nobody presses anything. The opening plays itself.")}
        </li>
        <li>
          <div><strong>Pick up the key.</strong>
          <p>Walk over to it; the E prompt pops up when you're close enough. The key goes into your inventory.</p>
          <p class="tag-line">sets <code>storageLantern_has_doorKey</code> · adds item <code>storage_door_key</code></p></div>
          ${shot("play-02-key.gif", "Walking to the key and picking it up.", "The prompt only shows for the nearest thing you can reach.")}
        </li>
        <li>
          <div><strong>Unlock the storage door.</strong>
          <p>Without the key you get a locked rattle. With it, the key is used up, the door disappears, and the dark doorway behind it now takes you into the storage room.</p>
          <p class="tag-line">sets <code>storageLantern_door_open</code> · removes the key</p></div>
          ${shot("play-03-door.gif", "Opening the storage door with the key and walking through.", "The doorway was a wall a moment ago. Now it's a warp.")}
        </li>
        <li>
          <div><strong>Get ambushed by goblins.</strong>
          <p>Halfway up the room, the floor triggers a fight with two goblins. Pick Attack and a target, and keep swinging until both are down.</p>
          <p class="tag-line">sets <code>storageBattle_triggered</code> (so it never happens twice)</p></div>
          ${shot("play-04-ambush.gif", "Stepping on the trigger and fighting two goblins.", "The storage room is thrown away while you fight, and rebuilt when you win.")}
        </li>
        <li>
          <div><strong>Take the Guiding Lantern.</strong>
          <p>You're back exactly where the fight started. The lantern sits at the far end of the room.</p>
          <p class="tag-line">sets <code>storageLantern_has_lantern</code> and <code>storageLantern_flames_active</code> · adds item <code>guiding_lantern</code></p></div>
          ${shot("play-05-lantern.gif", "Picking up the Guiding Lantern.", "Picking up the lantern quietly sets the deck on fire for later.")}
        </li>
        <li>
          <div><strong>Head back to the deck. It's on fire.</strong>
          <p>Leave through the bottom doorway. The boat is rebuilt from its map as always, but this time its start-up script sees the lantern tag. The sea is now lava and four fires are burning.</p>
          <p class="tag-line">no new tag: the boat reacts to the ones you already have</p></div>
          ${shot("play-06-lava.gif", "The boat deck surrounded by lava with fires burning.", "Same map file, different facts, different boat.")}
        </li>
        <li>
          <div><strong>Use the lantern at the bow.</strong>
          <p>Stand on the green patch at the front of the ship, open your inventory with <kbd>I</kbd> and use the Guiding Lantern. Fairies pour out, the water comes back and the lava fades away over two seconds. Try it anywhere else and you get "Nothing happened..."</p>
          <p class="tag-line">sets <code>storageLantern_fairies_released</code> and <code>weather=fairies</code></p></div>
          ${shot("play-07-fairies.gif", "Using the lantern at the bow: fairies appear and the lava fades back to water.", "The sea is back, but the deck is still burning.")}
        </li>
        <li>
          <div><strong>Ask the rain mage for help.</strong>
          <p>Talk to the crew member near the front of the deck. They walk all the way to the red chest at the stern, grab their weather charm, and call down the rain. The camera visits each fire as it goes out, the screen fades to black, and the slice ends on a "To Be Continued" card.</p>
          <p class="tag-line">sets <code>weather=rain</code> and <code>shipFinale_complete</code> · clears <code>storageLantern_flames_active</code></p></div>
          ${shot("play-08-rain-finale.gif", "The rain mage walks to the chest, summons rain, and the camera pans across the fires as each one goes out.", "The closing cutscene, start to finish.")}
        </li>
      </ol>
      <aside class="callout"><strong>Talk to everyone, often</strong><p>All three crew members have something new to say after each step. Each one has a rule table that checks your tags from the newest story beat down and picks the first line that fits. The C++ · Lua · YAML page shows how that works.</p></aside>
      <aside class="callout"><strong>Saving</strong><p>Press <kbd>Esc</kbd> → Save on the deck or in the storage room. The battle arena can't be saved, and the menu stays shut during cutscenes. With the default settings, the next launch drops you right back where you saved.</p></aside>
    `
  },

  topology: {
    title: "System topology",
    eyebrow: "Foundation · The four layers",
    lede: "The source tree is split into four folders, and the split is about one question: which code is allowed to know this is an RPG? Only Game is. Everything below it would be just as happy drawing a puzzle game.",
    body: `
      ${diagram("system-topology.svg", "The four layers", "Dependencies point down, never up.", "Application, Game, Framework and Platform stacked with arrows pointing down. YAML and Lua enter through Game. A dashed arrow shows Game calling Platform directly.", "Game talks to Platform directly for things like playing a sound or writing to the log. Nothing in Framework or Platform ever includes a Game header.")}
      <section class="prose">
        <h2>What each layer looks after</h2>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Layer</th><th>Looks after</th><th>Never knows about</th></tr></thead>
        <tbody>
          <tr><td>Application</td><td>Opening the window, reading <code>config.yaml</code>, the frame timer, turning key presses into named actions like <code>primaryAction</code>, and putting the finished picture on screen.</td><td>Quests, scenes or what any key "means" in the game.</td></tr>
          <tr><td>Game</td><td>The boat, the storage room, the battle, crew, doors, items, tags, Lua, menus, weather. In short, the RPG.</td><td>Raw Direct2D, XAudio2 or Win32 calls. It asks Platform instead.</td></tr>
          <tr><td>Framework</td><td>The small reusable skeleton: scenes made of layers, objects in those layers, a camera, a collision mask, and a resource lookup by ID.</td><td>Missions, inventory, battles, or what a red pixel in the mask means.</td></tr>
          <tr><td>Platform</td><td>Talking to Windows: drawing, text, audio, keyboard state, logging, and loading images, sprite sheets and sounds.</td><td>Anything about gameplay.</td></tr>
        </tbody>
      </table>
      <aside class="callout"><strong>Quick test for where code goes</strong><p>If a class mentions a mission, an item, a battle reward, a crew member or what some ground "means", it belongs in Game.</p></aside>
      <section class="prose">
        <h2>Why Framework is so small</h2>
        <p>It's tempting to build a general engine first and a game second. This project goes the other way: code only moves down into Framework after the ship slice has actually needed it in more than one place. That's why Framework has a camera and layers (every scene uses them) but no inventory or dialogue system (only the RPG needs those).</p>
      </section>
      ${sources([
        ["Application", "src/Application/Application.cpp · src/Application/AppConfig.cpp"],
        ["Game's hub", "src/Game/GameController.cpp"],
        ["Framework", "src/Framework/Scenes/BaseScene.cpp · src/Framework/Objects/BaseGameObject.cpp"],
        ["Platform", "src/Platform/Graphics.cpp · src/Platform/Audio/Audio.cpp · src/Platform/Input/DeviceInput.cpp"]
      ])}
    `
  },

  responsibilities: {
    title: "C++ · Lua · YAML",
    eyebrow: "Foundation · Three languages, three jobs",
    lede: "Three languages share the work, and each has one job. YAML says what exists. C++ runs the rules and owns the game state. Lua decides what things mean right now. The easiest way to see it is to press E at the storage door and follow what happens.",
    body: `
      ${diagram("language-boundaries.svg", "Press E at the storage door", "Five steps, three languages.", "Five cards: YAML places the door, C++ finds what's in reach and catches E, YAML names the script, Lua checks for the key and asks for changes, C++ applies them after the frame.", "Swap the door for a crew member and only step 3 changes: a rule table picks the script instead of naming it directly.")}
      <section class="prose">
        <h2>Why Lua only gets to ask</h2>
        <p>Lua scripts never touch a C++ object directly. They call a short list of functions such as <code>Say</code>, <code>SetTag</code>, <code>SpawnObject</code> and <code>SetGroundBehavior</code>. Tags and items change straight away, after a quick check. Anything that changes the world itself (spawning, removing, fading, re-labelling ground) just leaves a note in a queue. C++ works through that queue once per frame, after every object has finished updating, and checks each request first. A script can't delete the player or pull an object out from under a loop that's still running.</p>
        <h2>Rule tables: picking a script by situation</h2>
        <p>Crew members say different things as the quest moves along. Instead of one big Lua file full of <code>if</code> statements, each crew member has a YAML rule table. Rules are checked top to bottom, and the first one whose tags all match wins:</p>
      </section>
      ${code("assets/scenes/boatscene/characters/crew1/dialog.rules.yaml", "YAML", `rules:
  - when: { all: [mission_storageLantern, storageLantern_fairies_released] }
    script: …/crew1/dialog_fairies.lua
  - when: { all: [mission_storageLantern, storageLantern_has_lantern] }
    script: …/crew1/dialog_has_lantern.lua
  # … one rule per story beat, newest first …
  - when: { all: [mission_storageLantern] }
    script: …/crew1/dialog_find_key.lua
  - script: …/crew1/dialog_start_mission.lua   # no condition: the fallback`)}
      <p class="code-note">Shortened from the real file, which writes each rule out in long form. Rules can also use <code>any</code> and <code>none</code>.</p>
      <section class="prose">
        <h2>Cutscenes follow the same split</h2>
        <p>Lua writes the order of events: walk here, say this, pan over there. YAML stores the named points the crew walk between. C++ does the actual moving, panning, waiting and fading, and wakes the script up when each step is done. The <a href="#cutscenes">Cutscenes page</a> walks through one.</p>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Question</th><th>Who answers</th><th>Where</th></tr></thead>
        <tbody>
          <tr><td>Can the player walk here?</td><td>C++</td><td>The player's feet test the collision mask and the current region rules.</td></tr>
          <tr><td>Where does this door start, and what does it look like?</td><td>YAML</td><td>The door's <code>.obj.yaml</code> file and its spot in the map's layers.</td></tr>
          <tr><td>What happens when you use the locked door?</td><td>Lua</td><td><code>door/interact.lua</code> checks for the key and decides.</td></tr>
          <tr><td>Is that change allowed, and when does it happen?</td><td>C++</td><td>Queued, validated, and applied after the frame.</td></tr>
        </tbody>
      </table>
      ${sources([
        ["The Lua side of the fence", "src/Game/Scripting/LuaSystem.cpp"],
        ["Rule tables", "src/Game/Scripting/RuleTable.cpp"],
        ["Building objects from YAML", "src/Game/Scenes/SceneBuilder.cpp"],
        ["The door, start to finish", "assets/scenes/boatscene/objects/door/storage_door.obj.yaml · assets/scenes/boatscene/objects/door/interact.lua"]
      ])}
    `
  },

  runtime: {
    title: "Runtime lifecycle",
    eyebrow: "Flow · Startup and every frame",
    lede: "Startup happens once: read the settings, load the shared game data, then build the first scene. After that the same short loop runs every frame, and it's built so that scripts never change the world halfway through an update.",
    body: `
      ${diagram("runtime-flow.svg", "Startup and the frame loop", "Gold boxes are the decision points.", "Startup: config.yaml, game.yaml, check for a save, build the scene, initialize. Each frame: pause menu check, scene update, apply Lua requests, handle scene change or battle, render.", "When a warp or battle fires, the loop jumps back up to Build scene. The old scene is gone before the new one is built.")}
      <ol class="step-list">
        <li><strong>Read the settings.</strong><span><code>config.yaml</code> picks the window mode, the 1280 × 720 render size, the scaling filter, the dialogue typing speed, and how to start: always a given scene, or "resume my save if there is one".</span></li>
        <li><strong>Load what every scene shares.</strong><span><code>game.yaml</code> holds the hero's starting stats, the XP table, the item list, the menu sounds and both music tracks. These stay loaded for the whole run.</span></li>
        <li><strong>Build the first scene.</strong><span>Either the saved scene at your saved position, or the start scene from config. Building means <code>SceneBuilder</code> reading the map YAML in a fixed order; the Asset page lists that order.</span></li>
        <li><strong>Let the scene set itself up.</strong><span>Each object runs its <code>INIT</code> hook, then the scene's <code>init.lua</code> looks at your tags and adjusts the world: removing the door you already opened, swapping water for lava, lighting fires. On a brand-new game it also queues the opening cutscene.</span></li>
        <li><strong>Every frame: menu first.</strong><span>If the pause menu is open, only the menu updates and the world is frozen. If a dialogue box is open, the world keeps animating but ignores your movement keys. During a cutscene the menu and inventory stay shut too.</span></li>
        <li><strong>Update, then apply.</strong><span>The player moves, cutscene moves, pans, waits and fades tick forward, the camera follows, objects animate, and the E prompt goes to the nearest thing in reach. Anything Lua asked for during that time (spawns, removals, fades, region changes) is applied together afterwards.</span></li>
        <li><strong>Leave if asked.</strong><span>Walking onto a warp or starting a battle hands <code>GameController</code> a small request. It shuts the current scene down completely and builds the next one.</span></li>
        <li><strong>Draw.</strong><span>Map layers in order, then weather, then E prompts, then dialogue and menus, and finally the fade-to-black overlay a cutscene can raise. Shutdown is explicit too: objects run their <code>DESTROY</code> hooks while Lua is still alive.</span></li>
      </ol>
      ${shot("dialogue-typewriter.gif", "A dialogue box typing out a crew member's line.", "Dialogue types out at the speed set in config.yaml (84 characters per second). The world keeps moving behind it.")}
      ${diagram("runtime-vocabulary.svg", "Who owns what", "The long-lived owner, the one active scene, and the small requests that pass between them.", "GameController owns state, Lua, resources, UI and the active scene. BaseScene has two kinds: WorldScene (with BoatScene and StorageRoomScene) and BattleArenaScene. Scenes own layers; object layers own objects.", "BoatScene and StorageRoomScene are only a few lines each: they tell WorldScene which map and which collision mask to use. EndingScene just draws the closing card.")}
      ${sources([
        ["Startup and the main loop", "src/Application/main.cpp · src/Application/Application.cpp"],
        ["The per-frame order", "src/Game/GameController.cpp"],
        ["Building a scene", "src/Game/Scenes/SceneBuilder.cpp"],
        ["Scene update and the apply step", "src/Framework/Scenes/BaseScene.cpp · src/Game/Scenes/WorldScene.cpp"]
      ])}
    `
  },

  cutscenes: {
    title: "Cutscenes",
    eyebrow: "How it works · Scripted scenes",
    lede: "The slice opens and closes with a cutscene: the lookout strolling down to brief you, and the rain mage fetching their charm while the camera tours the fires. Each one is an ordinary Lua script that reads like stage directions. The trick is that 'walk over there' takes seconds, and the game can't stop for it.",
    body: `
      ${code("assets/scenes/boatscene/opening.lua", "LUA", `-- The whole opening cutscene.
playerInput = false
SetObjectPosition("crew1", "crew1StairsTop")
PlayAnimation("crew1", "walkDown")
MoveObjectTo("crew1", "crew1Watch", 150)    -- waits here until they arrive
PlayAnimation("crew1", "idleLeft")
dofile("assets/scenes/boatscene/characters/crew1/dialog_start_mission.lua")
playerInput = true`)}
      <section class="prose">
        <h2>A script that can wait without freezing anything</h2>
        <p>Every cutscene runs as a Lua <em>coroutine</em>: a script that can stop halfway and pick up again later. When it reaches <code>MoveObjectTo</code>, Lua leaves a request ("move crew1 to <code>crew1Watch</code> at 150 pixels a second") and goes to sleep. The game loop keeps running normally. Each frame, <code>WorldScene</code> slides the crew member a little closer, and when they arrive it wakes the script up on the very next line.</p>
        <p>Nothing in C++ knows what the opening or the finale is about. It only offers a handful of reusable actions: move, pan, wait, fade, change scene. The story, the order and the animations all live in Lua.</p>
      </section>
      ${diagram("cutscene-flow.svg", "The rain finale, one line at a time", "Lua on the left, what C++ does about it on the right.", "The rain finale script line by line. Non-blocking lines like PlayAnimation, SetTag and FollowObject take effect immediately. Blocking lines like MoveObjectTo, Say, Wait, MoveCameraTo, FadeOutObject and FadeOutScene put the script to sleep until WorldScene reports the action finished. ChangeScene ends on the To Be Continued card.", "The hourglass marks every line where the script sleeps. Between those lines, the game runs a normal frame loop.")}
      <table class="boundary-table">
        <thead><tr><th>Lua says</th><th>C++ does</th><th>Script continues when…</th></tr></thead>
        <tbody>
          <tr><td><code>MoveObjectTo(id, point, speed)</code></td><td>Slides the object in a straight line to a named waypoint.</td><td>it arrives</td></tr>
          <tr><td><code>MoveCameraTo(point, seconds)</code></td><td>Pans the camera to a named focus point.</td><td>the pan finishes</td></tr>
          <tr><td><code>Wait(seconds)</code></td><td>Counts down.</td><td>the time is up</td></tr>
          <tr><td><code>Say(text)</code></td><td>Shows the dialogue box and types the line.</td><td>you press E on the last page</td></tr>
          <tr><td><code>FadeOutObject(id, seconds)</code></td><td>Fades one object to transparent.</td><td>it's invisible</td></tr>
          <tr><td><code>FadeOutWeather(seconds)</code> · <code>FadeOutScene(seconds)</code></td><td>Fades the weather out, or the whole screen to black.</td><td>the fade finishes</td></tr>
          <tr><td><code>PlayAnimation</code> · <code>SetObjectPosition</code> · <code>FollowObject</code> · <code>SetTag</code> · <code>ChangeScene</code></td><td>Takes effect straight away.</td><td>immediately</td></tr>
        </tbody>
      </table>
      <section class="prose">
        <h2>Routes are drawn in the map, turns are chosen in Lua</h2>
        <p>Named points live in the scene's YAML, so you can nudge a route without touching any script. Movement is always a straight line between two points. There's no pathfinding, so a route that has to get around barrels is simply split into several legs:</p>
      </section>
      ${code("assets/scenes/boatscene/map.yaml (excerpt)", "YAML", `waypoints:
  mageAisle:      { x: 510, y: 405 }
  mageAcrossDeck: { x: 565, y: 345 }
  mageStairsBase: { x: 705, y: 345 }
  mageStairsTop:  { x: 705, y: 120 }
  mageChestAisle: { x: 470, y: 120 }
  mageChest:      { x: 470, y: 45 }
  fire1Focus:     { x: 591.72, y: 73.44 }   # camera targets are centers`)}
      <p class="code-note">Walking targets are sprite origins, the same x/y used to place objects in layers. Camera targets are the point to center on. The finale script loops over the six legs and picks <code>walkUp</code>, <code>walkLeft</code> and so on for each one, because only the script knows which way the mage should face.</p>
      ${shot("cutscene-route.gif", "The rain mage walking the six-leg route to the red chest at the stern, with the camera following.", "Six straight legs, each with its own walking animation.")}
      <section class="prose">
        <h2>Taking the controls away, and always giving them back</h2>
        <p>A script turns the player off with one line: <code>playerInput = false</code>. While it's false, your character ignores movement, E does nothing, and the menu and inventory won't open. The script sets it back to <code>true</code> when it's done.</p>
        <p>Two safety nets stop a broken cutscene from leaving you stuck. If an action fails (the waypoint doesn't exist, or the object was removed mid-walk), <code>WorldScene</code> cancels that cutscene, clears its camera and fade, and hands control back. And leaving a scene for any reason cancels everything still running in it.</p>
      </section>
      <section class="prose">
        <h2>Who starts a cutscene</h2>
        <p>The opening is started by the boat's <code>init.lua</code> with <code>StartCutscene</code>, but only on a brand-new game: if you already have the mission, the lantern, or a burning deck, it's skipped. That covers coming back from the storage room and loading a save. <code>WorldScene</code> owns the coroutine, and it starts before your first frame of control.</p>
        <p>The finale is different. It begins as an ordinary conversation, when you talk to the rain mage after the fairies are free, and that crew member's dialogue script simply carries on into the cutscene. Its last two lines fade the screen to black and call <code>ChangeScene("ending")</code>.</p>
      </section>
      ${shot("ending-card.png", "A black screen with the words To Be Continued.", "EndingScene: a black card that fades its text in and plays the victory jingle.")}
      <aside class="callout"><strong>Duties are not cutscenes</strong><p>Objects also have a <code>DUTIES</code> hook for work they do on their own, like a crew member pacing a patrol. Crew 1 has one, currently just a comment. Openings and finales belong to the scene, so a crew member's chores never decide when the story starts.</p></aside>
      ${sources([
        ["The two cutscenes", "assets/scenes/boatscene/opening.lua · assets/scenes/boatscene/characters/crew3/rain_finale.lua"],
        ["Where they start", "assets/scenes/boatscene/init.lua · assets/scenes/boatscene/characters/crew3/dialog_fairies.lua"],
        ["Waypoints", "assets/scenes/boatscene/map.yaml"],
        ["Running the actions", "src/Game/Scenes/WorldSceneCutscene.cpp · src/Game/Scripting/LuaSystem.cpp"],
        ["Camera pans and follow", "src/Framework/Rendering/Camera.cpp"],
        ["The ending card", "src/Game/Scenes/EndingScene.cpp · assets/scenes/ending/map.yaml"],
        ["Tests", "tests/WorldCutsceneTests.cpp"]
      ])}
    `
  },

  "world-rendering": {
    title: "World & rendering",
    eyebrow: "System · Walking around and drawing it",
    lede: "A walking scene is a stack of layers you can see, plus one image you can't: a collision mask that says where you can walk and which spots are special. The player has four different reference points, because 'where is the player?' has a different answer for the camera, the E key, the walls and draw order.",
    body: `
      ${diagram("render-stack.svg", "Draw order on the boat", "Bottom to top, exactly as listed in the boat's map YAML.", "Boat layers from water and lava up through floor, effects, doors, actors and foreground, then weather, interaction prompts, dialogue and menu, then the final 1280 by 720 picture scaled into the window.", "The lava layer sits above the water so Lua can fade lava out and reveal the water underneath.")}
      <section class="prose">
        <h2>Layers are drawn in the order you write them</h2>
        <p>Each scene's YAML lists its layers top to bottom, and that's the order they're drawn. On the boat, water comes first, then a spare layer for lava, the deck art, effects like fire, doors, then everyone walking around, and finally the foreground (railings and rigging that should cover your feet).</p>
        <p>The <code>actors</code> layer is the only one that re-sorts every frame: whoever's feet are lower on screen is drawn in front. After all the map layers, the scene adds weather and E prompts on top so nothing can hide the prompt. Then <code>GameController</code> draws dialogue and the pause menu, which never move with the camera.</p>
      </section>
      <section class="prose">
        <h2>Water that won't sit still</h2>
        <p>The sea is a <code>scrollingSurface</code> that drifts upward forever. A single scrolling image looks like a conveyor belt, so the water object also names an <code>alternateSprite</code>, a flipped copy of the same texture. The two blend back and forth every 1.5 seconds at 35% strength, and the waves seem to churn instead of slide.</p>
      </section>
      ${code("assets/scenes/boatscene/objects/water/water.obj.yaml", "YAML", `init:
  id: waterObj
  type: scrollingSurface
  sprite: water
  alternateSprite: waterFlipped   # the second frame
  crossfadeSeconds: 1.5
  crossfadeStrength: 0.35
  speedY: -20                     # pixels per second, upward`)}
      ${shot("world-water.gif", "The water around the ship scrolling and shimmering between two blended frames.", "Two images, one slow crossfade.")}
      ${shot("world-prompt.png", "Close-up of the E prompt floating above the storage door.", "The E prompt is drawn after every map layer, so a railing can never cover it.")}
      ${diagram("world-geometry.svg", "Four points on one sprite, and the mask underneath", "The real player frame and the real boat mask.", "The player sprite with the camera point at the chest, and the interaction point, two collision probes and the sort line at the feet. Next to it the boat art and its collision mask with a red door region and green bow region.", "All four points are set in player.obj.yaml in the sprite's own pixels; the game scales them along with the sprite.")}
      <table class="boundary-table">
        <thead><tr><th>Point</th><th>What it answers</th><th>Player value</th></tr></thead>
        <tbody>
          <tr><td><code>cameraPoint</code></td><td>What should the camera keep centered?</td><td><code>(119, 165)</code>: the chest</td></tr>
          <tr><td><code>interactionPoint</code></td><td>What can I reach with E?</td><td><code>(119, 300)</code>: between the feet</td></tr>
          <tr><td><code>collisionProbes</code></td><td>Can I stand here?</td><td><code>(90, 300)</code> and <code>(148, 300)</code>: each foot</td></tr>
          <tr><td><code>sortPointY</code></td><td>Am I in front of or behind that crew member?</td><td><code>300</code>: the soles</td></tr>
        </tbody>
      </table>
      <section class="prose">
        <h2>Who the camera watches</h2>
        <p>Normally the camera keeps the player's <code>cameraPoint</code> centered, clamped so it never shows past the edge of the map. A cutscene can take it over: <code>FollowObject</code> hands it to someone else (the rain mage, during the finale), and <code>MoveCameraTo</code> glides it to a named point over a set time.</p>
        <h2>The mask never changes, but its meaning can</h2>
        <p>Each walking scene has a <code>mask.png</code> the same size as the art. Black, or anywhere off the edge, always blocks you. Any other color is fine to walk on unless the scene has given that color a name. On the boat, red is the <code>storageDoor</code> region and green is <code>lanternRelease</code>.</p>
        <p>A named region has a behavior: <code>walkable</code>, <code>collision</code>, <code>warp</code> (go to another scene) or <code>trigger</code> (run a Lua file when you step on it). Lua can change that behavior at runtime. That's how the storage door works: its red pixels start as <code>collision</code>, and the door script flips them to <code>warp</code>. Nobody repaints the mask.</p>
      </section>
      <aside class="callout"><strong>How the picture reaches your screen</strong><p>Everything is drawn into a fixed 1280 × 720 image first. Application then scales that image into the window, either keeping its shape (<code>fit</code>) or filling it (<code>stretch</code>), with sharp <code>nearest</code> or smooth <code>linear</code> pixels. Game code never has to care how big your window is.</p></aside>
      ${sources([
        ["The boat's layers and regions", "assets/scenes/boatscene/map.yaml"],
        ["The player's four points", "assets/world/characters/player/player.obj.yaml · src/Game/Objects/Player.cpp"],
        ["Layers, sorting and prompts", "src/Framework/Scenes/BaseScene.cpp · src/Framework/Layer/ObjectLayer.cpp · src/Game/Scenes/WorldScene.cpp"],
        ["Camera, mask and regions", "src/Framework/Rendering/Camera.cpp · src/Framework/Collision/CollisionMask.cpp · src/Game/World/GroundRegionMap.cpp"],
        ["Scaling into the window", "config.yaml · src/Application/Application.cpp"]
      ])}
    `
  },

  "state-save-load": {
    title: "State & save/load",
    eyebrow: "System · Remembering what you did",
    lede: "A save file is tiny: where you're standing, your stats and items, and a list of true/false facts like storageLantern_door_open. It never stores the door itself. On load, the boat is built fresh from its map and init.lua reads those facts: door open? Then remove the door and turn its doorway into a warp. Same result, far less to go wrong.",
    body: `
      ${code("saves/save.yaml", "YAML", `version: 1
savedAt: 2026-10-18T14:32:05Z
scene:
  id: boatScene
  playerX: 612.5
  playerY: 948
player:
  name: Cecil
  level: 1
  experience: 27
  hp: 128
  maxHp: 142
  mp: 36
  maxMp: 36
  attack: 12
  defense: 5
  gold: 0
inventory:
  - crudeClub
  - guiding_lantern
  - rustyDagger
tags:
  mission_storageLantern: true
  storageBattle_triggered: true
  storageLantern_door_open: true
  storageLantern_flames_active: true
  storageLantern_has_doorKey: true
  storageLantern_has_lantern: true`)}
      <p class="code-note">An example of what the game writes after the fight, back on the burning deck. Items and tags are sorted so the file stays stable between saves.</p>
      <section class="prose">
        <h2>Three things live above every scene</h2>
        <p><code>GameController</code> owns <code>PlayerState</code> (stats), <code>Inventory</code> (a set of item IDs) and <code>GlobalState</code> (the tags). They survive every scene change, the battle arena uses the very same <code>PlayerState</code> (so damage sticks), and Lua reaches the tags and items through functions like <code>SetTag</code> and <code>AddInventoryItem</code>.</p>
        <p>Everything else is rebuilt on demand: doors, keys, fires, the lava, which region warps where. None of it is in the file, because the tags are enough to rebuild it. That's the whole trick, and it's the same trick the battle uses to send you back to the storage room.</p>
      </section>
      ${shot("menu-pause.gif", "The pause menu with Save, Load, Inventory, Status and Quit.", "Save is greyed out in the battle arena; Load is greyed out until a save exists. The menu can't open during cutscenes.")}
      ${diagram("save-load-flow.svg", "Saving, and loading carefully", "Loading checks everything before it touches the running game.", "Save: the menu asks, GameController collects, SaveGame validates and writes. Load: read into a temporary copy, check the file, check it against game data, then unload the old scene, swap the state in, and build the saved scene normally.", "If the file fails either check, you keep playing exactly where you were.")}
      <ol class="step-list">
        <li><strong>Read into a scratch copy.</strong><span>The file is parsed into temporary objects first. Wrong version, missing fields, impossible HP, duplicate items, or a position that isn't a real number: rejected.</span></li>
        <li><strong>Check it against the game.</strong><span>Your XP must fit the level table, every item ID must exist in <code>game.yaml</code>, and the scene must be <code>boatScene</code> or <code>storageRoom</code>.</span></li>
        <li><strong>Only then, swap.</strong><span>The current scene shuts down first, while it can still see the old state, so its cleanup can't scribble on the state you're loading. Then the new stats, items and tags are copied into the existing owners. Lua and the menus keep pointing at the same objects.</span></li>
        <li><strong>Build the scene the normal way.</strong><span>It's the same scene change a door uses, plus your saved X/Y. The scene's <code>init.lua</code> does the rest.</span></li>
      </ol>
      <aside class="callout"><strong>Starting the game</strong><p>With <code>startMode: saveOrScene</code> (the default), launching the game loads <code>saves/save.yaml</code> if it exists. A broken save stops startup with an error instead of quietly starting a new game, because a silently "lost" save is worse than an honest error.</p></aside>
      <aside class="callout"><strong>What's not here yet</strong><p>One save slot, format version 1. No multiple slots, no upgrading old save formats, and no saving mid-battle.</p></aside>
      ${sources([
        ["The three owners", "src/Game/GameController.h · src/Game/State/GlobalState.cpp · src/Game/State/Inventory.cpp · src/Game/State/PlayerState.cpp"],
        ["Reading and writing the file", "src/Game/State/SaveGame.cpp"],
        ["Save and load from the menu", "src/Game/GameController.cpp · src/Game/UI/MainMenu.cpp"],
        ["Rebuilding the boat from tags", "assets/scenes/boatscene/init.lua"],
        ["Tests", "tests/SaveGameTests.cpp"]
      ])}
    `
  },

  "battle-flow": {
    title: "Battle flow",
    eyebrow: "System · Goblins, turn by turn",
    lede: "Step onto the green tiles in the storage room and two goblins jump you. Behind the scenes, the storage room is thrown away completely and the battle arena is built in its place. When you win, the game rebuilds the storage room and drops you back on the exact pixel you left. Nothing is paused in the background, so this page is about how the game remembers enough to make that return feel seamless.",
    body: `
      ${diagram("battle-round-trip.svg", "Into battle and back again", "Six steps, and only the gold box survives all of them.", "Storage room trigger, BattleRequest, arena, victory rewards, SceneChangeRequest home, storage room rebuilt. Persistent player state, inventory and tags sit in the middle.", "The trigger script sets its tag before starting the fight, so the rebuilt room already knows the fight happened.")}
      <section class="prose">
        <h2>Leaving the world</h2>
        <p>The green floor in the storage room is a <code>trigger</code> region. Stepping on it runs <code>start_battle.lua</code>, which marks the fight as done, makes the tiles plain floor again, and calls <code>StartBattle</code> with two goblin IDs. On the way out, <code>WorldScene</code> adds where to come back to: the storage room, and your exact X and Y.</p>
        <p>That's all the arena gets: a small note with names and coordinates. It doesn't get a pointer to the old room, because the old room is about to be destroyed.</p>
      </section>
      ${shot("battle-strike.gif", "The hero swings at a goblin; the hit lands with a sound and the goblin flinches.", "Sound, flinch and damage all land on the same animation frame.")}
      ${diagram("battle-turn-flow.svg", "One round of combat", "You pick, C++ checks, the swing lands, and the goblin you hit answers.", "Your turn, C++ checks the move, the swing with impact on frame 3, damage formula, outcome check, victory or defeat, or the goblin you hit answers via its Lua script and the loop returns to your turn.", "The goblin's Lua script only ever sees copies of the stats. It returns a suggestion, and C++ runs the same checks on it as on yours.")}
      <section class="prose">
        <h2>The numbers right now</h2>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Who</th><th>HP</th><th>Attack / Defense</th><th>Hits for</th><th>Reward</th></tr></thead>
        <tbody>
          <tr><td>Cecil (you)</td><td>142</td><td>12 / 5</td><td>10 on Goblin A, 9 on Goblin B</td><td>—</td></tr>
          <tr><td>Goblin A (sword)</td><td>30</td><td>8 / 2</td><td>3</td><td>12 XP · Rusty Dagger</td></tr>
          <tr><td>Goblin B (club)</td><td>36</td><td>9 / 3</td><td>4</td><td>15 XP · Crude Club</td></tr>
        </tbody>
      </table>
      <p class="code-note">Damage is <code>max(1, round(attack × power) − defense)</code>, and every current move has power 1.0 and costs 0 MP. Enemy stats grow 10% per level above 1; both goblins are level 1.</p>
      <aside class="callout"><strong>Coming home</strong><p>Press Enter on the victory screen and the arena sends an ordinary scene-change request: storage room, at your saved X/Y. The room is built fresh, its <code>init.lua</code> sees <code>storageBattle_triggered</code>, and the green tiles stay plain floor. Your XP and the two items are already in your stats and inventory.</p></aside>
      <aside class="callout"><strong>Not in yet</strong><p>Only Attack is enabled; Magic, Item and Run are drawn but greyed out. Losing shows a defeat screen and stops there. XP is added, but levelling up isn't processed yet.</p></aside>
      ${sources([
        ["Starting a fight", "assets/scenes/storageroom/start_battle.lua · src/Game/Battle/BattleRequest.h"],
        ["Leaving and coming back", "src/Game/Scenes/WorldScene.cpp · src/Game/GameController.cpp"],
        ["The arena", "src/Game/Scenes/BattleArenaScene.cpp"],
        ["Moves and damage", "assets/battle/moves/ · src/Game/Battle/BattleMoveLoad.cpp · src/Game/Battle/BattleRules.cpp"],
        ["Goblins and their tactics", "assets/battle/characters/ · src/Game/Objects/BattleEnemy.cpp"],
        ["Tests", "tests/BattleRulesTests.cpp · tests/BattleMenuTests.cpp"]
      ])}
    `
  },

  "asset-dependencies": {
    title: "Asset dependency map",
    eyebrow: "System · How the files point at each other",
    lede: "There's no asset pipeline or packing step. The game reads the same YAML, PNG, WAV and Lua files you edit, copied next to the executable. Files point at each other in two ways: by path (open this file) or by ID (use the thing already loaded under this name).",
    body: `
      ${diagram("asset-reference-graph.svg", "Following crew member 1 through the files", "From the boat's map down to pixels and dialogue.", "boatscene/map.yaml registers crew.anim.yaml and crew1.obj.yaml under IDs and places crew1 in the actors layer. crew.anim.yaml points to crew.png. crew1.obj.yaml names sprite ID crew and points to two rule tables; the dialogue rule table picks a Lua file, which can spawn the storage key by path.", "Gold arrows open files. Cyan dashed arrows look up something that's already loaded.")}
      <section class="prose">
        <h2>The map is the scene's address book</h2>
        <p>A scene's <code>map.yaml</code> lists every image, sprite, sound and object file the scene needs and gives each one a short ID. Everything else in the scene refers to those IDs. That's why <code>crew1.obj.yaml</code>, <code>crew2.obj.yaml</code> and <code>crew3.obj.yaml</code> can all just say <code>sprite: crew</code>, and why the same player files are reused by both the boat and the storage room.</p>
        <p>An object's settings are layered. The <code>.obj.yaml</code> file sets defaults, the map's <code>resources</code> entry can override them, and its placement in a layer has the final say. A layer entry like <code>{ obj: waterObj, sprite: water, speedY: -20 }</code> wins over both.</p>
      </section>
      <table class="boundary-table">
        <thead><tr><th>File</th><th>What it describes</th><th>Read by</th></tr></thead>
        <tbody>
          <tr><td><code>config.yaml</code></td><td>Window, render size, start mode, dialogue speed.</td><td><code>AppConfig</code></td></tr>
          <tr><td><code>game.yaml</code></td><td>Hero stats, XP table, items, shared sounds and music.</td><td><code>GameController</code></td></tr>
          <tr><td><code>*/map.yaml</code></td><td>One scene: its files and IDs, regions, camera, layers and placement.</td><td><code>SceneBuilder</code></td></tr>
          <tr><td><code>*.anim.yaml</code></td><td>A sprite sheet: image, grid size, named animations and their frames.</td><td><code>SpriteAsset</code></td></tr>
          <tr><td><code>*.obj.yaml</code></td><td>One object: type, sprite ID, settings, triggers, hooks.</td><td>a <code>*Load.cpp</code> per type</td></tr>
          <tr><td><code>*.rules.yaml</code></td><td>Tag conditions → which Lua file to run.</td><td><code>RuleTable</code></td></tr>
          <tr><td><code>*.move.yaml</code></td><td>A battle move: power, MP cost, animations, impact frame, sound.</td><td><code>BattleMoveLoad</code></td></tr>
          <tr><td><code>*.item.yaml</code></td><td>Item name, description, value, optional use script.</td><td><code>ItemDefinitionLoad</code></td></tr>
        </tbody>
      </table>
      ${diagram("asset-lifetime.svg", "What stays loaded, and for how long", "Game-wide things load once; scene things are cleared on every scene change.", "Game scope loads once at startup. On scene change the old scene shuts down and scene resources are cleared, then SceneBuilder loads metadata, media, regions and moves, object templates, camera and layers in order.", "Looking up an ID checks the current scene first, then the game-wide set.")}
      <section class="prose">
        <h2>Spawning brings nothing new with it</h2>
        <p>When Lua calls <code>SpawnObject</code> with an <code>.obj.yaml</code> path, that file must name a layer that already exists and a sprite ID the scene already loaded. That's why the boat's map registers the key, lava and flame sprites up front, and has empty <code>lavaOverlay</code> and <code>effects</code> layers waiting, even though none of those things exist when the boat first loads.</p>
      </section>
      <aside class="callout"><strong>Building and shipping</strong><p>CMake copies <code>assets/</code>, <code>config.yaml</code> and <code>game.yaml</code> next to the executable as-is. If the graphics device is reset, images and sprite sheets reload from their original file names; masks and sounds don't need to.</p></aside>
      ${sources([
        ["Root files and copying", "config.yaml · game.yaml · CMakeLists.txt"],
        ["Building a scene from its map", "src/Game/Scenes/SceneBuilder.cpp"],
        ["IDs and lifetimes", "src/Framework/Resources/ResourceManager.cpp"],
        ["Loaders", "src/Platform/SpriteAsset.cpp · src/Game/Battle/BattleMoveLoad.cpp · src/Game/Items/ItemDefinitionLoad.cpp"],
        ["Spawning at runtime", "src/Game/Scenes/WorldScene.cpp"]
      ])}
    `
  },

  "legacy-correspondence": {
    title: "Legacy correspondence",
    eyebrow: "History · Then and now",
    lede: "The original 2002 project still lives in this repo, untouched, under legacy/. It had a ship, a hero walking around a scrolling deck, a collision mask, scripted events, a message window and weather. Nothing was ported. Every idea that survived was rebuilt from scratch, and a lot of what you can play now never existed in 2002 at all.",
    body: `
      ${shot("legacy-2002.png", "The original 2002 prototype running.", "The 2002 prototype, if it can still be coaxed into running.")}
      ${diagram("legacy-correspondence.svg", "Same ideas, new code", "Left: what the 2002 source did. Middle: the idea worth keeping. Right: what does that job today.", "Five rows mapping 2002 features to the idea behind them and the modern code that does that job now.", "Dotted arrows mean 'does the same job', never 'was copied from'.")}
      <section class="prose">
        <h2>Kept, rebuilt, new, and left out</h2>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Bucket</th><th>What's in it</th></tr></thead>
        <tbody>
          <tr><td>Kept the idea</td><td>Opening on a ship, a deck drawn in layers, an animated hero, a color mask for walls and events, scripted interactions, a message window, a following camera, weather.</td></tr>
          <tr><td>Rebuilt from scratch</td><td>Drawing (DirectDraw → Direct2D), input, asset loading, scene ownership, scripting (a homemade token format → YAML and Lua), UI, audio (DirectSound → XAudio2).</td></tr>
          <tr><td>Brand new</td><td>The quest, crew dialogue rules, inventory, items, battles, XP, save/load. The 2002 source had none of these working.</td></tr>
          <tr><td>Left out on purpose</td><td>Experimental UDP networking, the in-game debug console, old DirectX interfaces, a half-finished screen-transition effect.</td></tr>
          <tr><td>Left out until cleared</td><td>Old binaries and art or sound whose origin can't be confirmed, plus anything derived from tutorials or third-party code.</td></tr>
        </tbody>
      </table>
      <aside class="callout"><strong>The one rule</strong><p><code>legacy/</code> can answer "what did the prototype do?" It never decides how today's code is structured, and a system missing from it is never described as "restored".</p></aside>
      ${sources([
        ["The original, and notes about it", "legacy/README.md · docs/DRAGON_QUEST_AUDIT.md · docs/REMOVED_LEGACY_COMPONENTS.md"],
        ["2002 runtime and drawing", "legacy/src/hour7.cpp · legacy/src/render.h · legacy/src/statemanager.h"],
        ["2002 world and scripts", "legacy/src/scriptio.h · legacy/src/AnimationController.h · legacy/scripts/"],
        ["Today's equivalents", "src/Application/ · src/Game/GameController.cpp · src/Framework/ · src/Platform/"]
      ])}
    `
  },

  "source-index": {
    title: "Source index & glossary",
    eyebrow: "Reference · Where things live",
    lede: "A map of the repo, the names you'll keep running into, and the full list of things a Lua script is allowed to do. Your browser's Find (Ctrl+F) is the fastest way through this page.",
    body: `
      ${code("Repository layout", "TREE", `RPG2002-Ship/
├─ config.yaml          window, render size, start mode
├─ game.yaml            hero stats, XP table, items, shared audio
├─ src/
│  ├─ Application/      window, main loop, key bindings, presentation
│  ├─ Game/             everything RPG: scenes, objects, Lua, state, battle, UI
│  ├─ Framework/        scenes, layers, objects, camera, mask, resource IDs
│  └─ Platform/         Direct2D, XAudio2, input, image/sprite/sound loading
├─ assets/
│  ├─ scenes/           one folder per scene: map.yaml, init.lua, cutscenes, art, objects
│  ├─ world/            walking characters (player, crew) and weather art
│  ├─ battle/           battle characters, their tactics, and moves
│  ├─ items/  fx/  props/  sounds/  music/
├─ tests/               focused C++ tests plus small Lua fixtures
├─ legacy/              the 2002 original (read-only)
├─ docs/                provenance notes, and this site (docs/site-docs)
└─ scripts/build.ps1    the Windows build`)}
      <section class="prose">
        <h2>"I want to change…"</h2>
      </section>
      <table class="boundary-table">
        <thead><tr><th>I want to…</th><th>Start here</th><th>Also look at</th></tr></thead>
        <tbody>
          <tr><td>Change the window, resolution or start scene</td><td><code>config.yaml</code></td><td><code>src/Application/AppConfig.cpp</code></td></tr>
          <tr><td>Change a cutscene</td><td><code>boatscene/opening.lua</code> or <code>crew3/rain_finale.lua</code></td><td>the <code>waypoints:</code> list in <code>boatscene/map.yaml</code></td></tr>
          <tr><td>Change what a crew member says</td><td>their <code>dialog.rules.yaml</code> and <code>dialog_*.lua</code></td><td><code>src/Game/Scripting/RuleTable.cpp</code></td></tr>
          <tr><td>Move something on the deck</td><td><code>assets/scenes/boatscene/map.yaml</code> (layers)</td><td>its <code>.obj.yaml</code> file</td></tr>
          <tr><td>Add a new scene</td><td>a new <code>assets/scenes/…/map.yaml</code></td><td>a small <code>WorldScene</code> subclass (or a plain <code>BaseScene</code> one, like <code>EndingScene</code>), plus a branch in <code>GameController::ChangeScene</code> (and the allowed list in <code>LoadGame</code> if it should be saveable)</td></tr>
          <tr><td>Add a new object type</td><td>a <code>*Load.cpp</code> in <code>src/Game/Objects/</code></td><td>the type switch in <code>SceneBuilder::CreateGameObject</code></td></tr>
          <tr><td>Give Lua a new ability</td><td><code>src/Game/Scripting/LuaSystem.cpp</code></td><td>where the request is applied, usually <code>WorldScene.cpp</code></td></tr>
          <tr><td>Tune the battle</td><td><code>assets/battle/</code></td><td><code>src/Game/Battle/BattleRules.cpp</code></td></tr>
          <tr><td>Change menus or dialogue</td><td><code>src/Game/UI/</code></td><td>the matching <code>*Render.cpp</code></td></tr>
        </tbody>
      </table>
      <section class="prose">
        <h2>Object types</h2>
        <p>The <code>type:</code> line in an <code>.obj.yaml</code> picks one of these. Each has its own loader, so a typo is caught when the scene loads.</p>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Type</th><th>What it is</th><th>Example</th></tr></thead>
        <tbody>
          <tr><td><code>player</code></td><td>You, on foot. Exactly one per walking scene.</td><td><code>world/characters/player/player.obj.yaml</code></td></tr>
          <tr><td><code>interactable</code></td><td>Anything with an E prompt and hooks: crew, doors, the key, the lantern.</td><td><code>world/characters/crew/crew1.obj.yaml</code></td></tr>
          <tr><td><code>spriteObject</code></td><td>Just an animation playing somewhere. Give it a <code>sortPointY</code> and it sorts with the actors, like the ship's helm.</td><td><code>boatscene/objects/flames/flame.obj.yaml</code></td></tr>
          <tr><td><code>scrollingSurface</code></td><td>A tiled texture that scrolls forever, like the water and lava. It can crossfade with an <code>alternateSprite</code>.</td><td><code>boatscene/objects/water/water.obj.yaml</code></td></tr>
          <tr><td><code>playerBattle</code></td><td>You, in the arena.</td><td><code>battle/characters/player/player_battle.obj.yaml</code></td></tr>
          <tr><td><code>enemy</code></td><td>A battle enemy with stats, moves, rewards and a tactics script.</td><td><code>battle/characters/goblin_sword/goblin_sword.obj.yaml</code></td></tr>
        </tbody>
      </table>
      <section class="prose">
        <h2>Names you'll see everywhere</h2>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Name</th><th>In plain words</th><th>Lives in</th></tr></thead>
        <tbody>
          <tr><td><code>GameController</code></td><td>The hub. Owns your stats, items and tags, the Lua runtime, menus, music, and the one active scene.</td><td><code>src/Game/</code></td></tr>
          <tr><td><code>SceneBuilder</code></td><td>Reads a <code>map.yaml</code> and builds the scene it describes, in a fixed order.</td><td><code>src/Game/Scenes/</code></td></tr>
          <tr><td><code>BaseScene</code></td><td>A list of layers with a camera. Updates and draws them.</td><td><code>src/Framework/Scenes/</code></td></tr>
          <tr><td><code>WorldScene</code></td><td>A scene you walk around in: player, mask, regions, E prompts, and applying Lua's requests.</td><td><code>src/Game/Scenes/</code></td></tr>
          <tr><td><code>BattleArenaScene</code></td><td>The turn-based fight.</td><td><code>src/Game/Scenes/</code></td></tr>
          <tr><td><code>EndingScene</code></td><td>The black "To Be Continued" card the slice ends on.</td><td><code>src/Game/Scenes/</code></td></tr>
          <tr><td>Cutscene</td><td>A Lua coroutine started with <code>StartCutscene</code> and owned by the current <code>WorldScene</code>.</td><td><code>src/Game/Scenes/WorldSceneCutscene.cpp</code></td></tr>
          <tr><td>Waypoint</td><td>A named point in a map's <code>waypoints:</code> list, used as a walking target or a camera focus.</td><td><code>boatscene/map.yaml</code></td></tr>
          <tr><td><code>ResourceManager</code></td><td>Looks up images, sprites, masks and sounds by ID; scene ones first, then game-wide ones.</td><td><code>src/Framework/Resources/</code></td></tr>
          <tr><td><code>ObjectHook</code></td><td>One of <code>INIT</code>, <code>DUTIES</code>, <code>INTERACT</code> or <code>DESTROY</code>, pointing at a Lua file or a rule table.</td><td><code>src/Framework/Objects/</code></td></tr>
          <tr><td><code>RuleTable</code></td><td>"First rule whose tags match picks the script."</td><td><code>src/Game/Scripting/</code></td></tr>
          <tr><td><code>GroundRegionMap</code></td><td>Which mask colors have names, and what each one does right now.</td><td><code>src/Game/World/</code></td></tr>
          <tr><td>Tag</td><td>A named true/false fact, like <code>storageLantern_door_open</code>. Saved with your game.</td><td><code>src/Game/State/GlobalState.*</code></td></tr>
          <tr><td><code>SceneChangeRequest</code></td><td>"Go to this scene," plus a few values such as a spawn point or exact X/Y.</td><td><code>src/Framework/Scenes/</code></td></tr>
          <tr><td><code>BattleRequest</code></td><td>"Fight these enemies, then come back here."</td><td><code>src/Game/Battle/</code></td></tr>
        </tbody>
      </table>
      <section class="prose">
        <h2>Everything Lua can do</h2>
        <p>This is the complete list. If it's not here, a script can't do it.</p>
      </section>
      <table class="boundary-table">
        <thead><tr><th>Group</th><th>Functions</th><th>What for</th></tr></thead>
        <tbody>
          <tr><td>Facts and items</td><td><code>SetTag</code> · <code>GetTag</code><br><code>AddInventoryItem</code> · <code>RemoveInventoryItem</code> · <code>HasInventoryItem</code></td><td>Read and change your tags and items.</td></tr>
          <tr><td>Talking and sound</td><td><code>Say</code> · <code>DebugMessage</code><br><code>PlaySound</code> · <code>PlayMusic</code> · <code>StopMusic</code></td><td><code>Say(text, portrait, closeSound)</code> pauses the script until you've read the line.</td></tr>
          <tr><td>Changing the world</td><td><code>SpawnObject</code> · <code>DestroyObject</code> · <code>FadeOutObject</code><br><code>SetGroundBehavior</code> · <code>SetGroundParams</code></td><td>Queued and applied after the frame.</td></tr>
          <tr><td>Cutscenes</td><td><code>StartCutscene</code> · <code>Wait</code><br><code>PlayAnimation</code> · <code>SetObjectPosition</code> · <code>MoveObjectTo</code><br><code>FollowObject</code> · <code>MoveCameraTo</code><br><code>FadeOutWeather</code> · <code>FadeOutScene</code> · <code>ChangeScene</code></td><td>Moves, pans, waits and fades pause the script until they finish.</td></tr>
          <tr><td>Waiting and fighting</td><td><code>WaitForTrigger</code> · <code>StartBattle</code></td><td>Pause a <code>DUTIES</code> script until its object reports a named trigger, or start a fight.</td></tr>
          <tr><td>Where am I?</td><td><code>CurrentObjectId</code> · <code>CurrentItemId</code><br><code>CurrentSceneId</code> · <code>CurrentGroundRegionId</code></td><td>Read-only values describing who is running the script.</td></tr>
          <tr><td>Player control</td><td><code>playerInput</code></td><td>A global a script sets to <code>false</code> to take the controls away, and back to <code>true</code> to return them.</td></tr>
        </tbody>
      </table>
      <aside class="callout"><strong>Build and test</strong><p>On Windows, run <code>powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1</code>. Tests live in <code>tests/</code>, with small Lua fixtures beside them that exercise the scripting functions.</p></aside>
      ${sources([
        ["Project notes", "AGENTS.md · PROJECT_PLAN.md · CODEX_SCRATCHPAD.md"],
        ["The Lua function list", "src/Game/Scripting/LuaSystem.cpp"],
        ["Object types", "src/Game/Scenes/SceneBuilder.cpp"],
        ["Build", "scripts/build.ps1 · CMakeLists.txt"]
      ])}
    `
  }
};

const routeAliases = { "game-map": "play" };

function diagram(file, title, subtitle, alt, caption) {
  const src = `assets/diagrams/${file}`;
  return `
    <figure class="diagram-block">
      <div class="diagram-toolbar">
        <div><h2>${title}</h2><p>${subtitle}</p></div>
        <div class="diagram-actions">
          <button type="button" data-expand="${src}" data-alt="${escapeAttr(alt)}">Expand</button>
          <a href="${src}" download>SVG</a>
        </div>
      </div>
      <div class="diagram-frame"><img src="${src}" alt="${escapeAttr(alt)}"></div>
      ${caption ? `<figcaption class="diagram-caption">${caption}</figcaption>` : ""}
    </figure>`;
}

// Screenshots and clips live in assets/media/shots/. Until a file exists the
// figure shows a labelled placeholder so the page layout can be reviewed.
function shot(file, alt, caption) {
  const src = `assets/media/shots/${file}`;
  return `
    <figure class="shot">
      <img src="${src}" alt="${escapeAttr(alt)}" loading="lazy" onerror="this.closest('figure').classList.add('is-missing')">
      <div class="shot-missing" aria-hidden="true"><span>Screenshot coming</span><code>${file}</code><small>${alt}</small></div>
      ${caption ? `<figcaption>${caption}</figcaption>` : ""}
    </figure>`;
}

function code(label, language, text) {
  const escaped = text.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
  return `<figure class="code-block"><figcaption><span>${label}</span><span class="code-lang">${language}</span></figcaption><pre><code>${escaped}</code></pre></figure>`;
}

function pageCard(route, title, text, action) {
  return `<a class="page-card" href="#${route}"><strong>${title}</strong><p>${text}</p><small>${action} →</small></a>`;
}

function sourceLink(path) {
  const clean = path.trim();
  if (clean.includes("*") || clean.includes("…")) return `<code>${clean}</code>`;
  const kind = clean.endsWith("/") ? "tree" : "blob";
  return `<a class="source-link" href="${REPO}/${kind}/main/${clean}"><code>${clean}</code></a>`;
}

function sources(items) {
  return `<section><h2>Find it in the code</h2><ul class="source-list">${items.map(([title, paths]) => `<li><strong>${title}</strong><span class="source-paths">${paths.split(" · ").map(sourceLink).join("")}</span></li>`).join("")}</ul></section>`;
}

function escapeAttr(value) {
  return String(value).replaceAll("&", "&amp;").replaceAll('"', "&quot;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
}

function render() {
  const requested = location.hash.slice(1) || "overview";
  const route = routeAliases[requested] || requested;
  const page = pages[route];
  const content = document.getElementById("content");
  document.querySelectorAll("[data-route]").forEach(link => {
    if (link.dataset.route === route) link.setAttribute("aria-current", "page");
    else link.removeAttribute("aria-current");
  });

  if (!page) {
    content.innerHTML = `<article class="doc-page route-error"><p class="eyebrow">Off the map</p><h1>That page doesn't exist.</h1><p class="lede"><a href="#overview">Back to the overview</a></p></article>`;
    return;
  }

  document.title = `${page.title} · RPG2002-Ship Architecture Atlas`;
  const meta = page.meta ? `<div class="meta-row">${page.meta.map(item => `<span>${item}</span>`).join("")}</div>` : "";
  content.innerHTML = `<article class="doc-page"><header><p class="eyebrow">${page.eyebrow}</p><h1>${page.title}</h1><p class="lede">${page.lede}</p>${meta}</header>${page.body}</article>`;
  document.querySelector(".sidebar").classList.remove("is-open");
  document.querySelector(".menu-button").setAttribute("aria-expanded", "false");
  window.scrollTo(0, 0);
}

const menuButton = document.querySelector(".menu-button");
menuButton.addEventListener("click", () => {
  const sidebar = document.querySelector(".sidebar");
  const open = sidebar.classList.toggle("is-open");
  menuButton.setAttribute("aria-expanded", String(open));
});

const dialog = document.querySelector(".diagram-dialog");
document.addEventListener("click", event => {
  const button = event.target.closest("[data-expand]");
  if (!button) return;
  const image = dialog.querySelector("img");
  image.src = button.dataset.expand;
  image.alt = button.dataset.alt;
  dialog.showModal();
});

const themeColors = { sticker: "#ffc93c", voyage: "#070b14" };

function applyTheme(theme) {
  document.documentElement.dataset.theme = theme;
  document.querySelectorAll("[data-theme-choice]").forEach(button => {
    button.setAttribute("aria-pressed", String(button.dataset.themeChoice === theme));
  });
  document.querySelector('meta[name="theme-color"]').setAttribute("content", themeColors[theme]);
}

document.querySelectorAll("[data-theme-choice]").forEach(button => {
  button.addEventListener("click", () => {
    const theme = button.dataset.themeChoice;
    applyTheme(theme);
    try { localStorage.setItem("atlas-theme", theme); } catch (error) { /* storage unavailable; theme still applies */ }
  });
});

applyTheme(document.documentElement.dataset.theme === "voyage" ? "voyage" : "sticker");
window.addEventListener("hashchange", render);
render();
