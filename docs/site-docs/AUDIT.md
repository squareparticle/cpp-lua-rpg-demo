# Architecture Atlas audit: code vs. docs

Audit date: 18 October 2026, against commit `2542aee`.

**Status:** acted on the same day. The voice, the new Play page, and the
diagram verdicts in §6 were approved and applied. The game issues in §3 were
addressed in the 19 October update (rain finale, fires put out, crew 1 duties
reduced to a placeholder); only the XP cap remains.

## The short version

- **The facts are mostly right.** The battle numbers, save fields, load gates,
  render order, player anchors, Lua API list, scene build order, and property
  override chain all match the code. I found a handful of real inaccuracies
  (listed below) and none of them is serious.
- **The writing is the main problem.** It is accurate but it is all abstraction
  ("authoritative", "boundary", "ownership", "contextual meaning"). A reader
  learns the rules before they ever see the game. The same disclaimers come up
  on almost every page, and almost no examples come from the actual ship.
- **The diagrams are generic.** All 16 are the same navy box-and-arrow style.
  About half are good. The others repeat a table that sits right next to them,
  or draw a concept that a real example from the game would show better.
- **What's missing matters more than what's wrong.** There is nothing on how to
  play the slice (controls and quest steps). There is nothing on how to add a
  scene or an object type, and no list of the object types that exist.

---

## 1. Inaccuracies (docs say X, code does Y)

| Where | Docs say | Code actually does | Fix |
|---|---|---|---|
| Topology diagram + overview text | Application → Game → Framework → Platform as a strict chain ("flows through Game and the deliberately small Framework into Platform") | Game includes Platform directly in 21 files (`Graphics`, `Audio`, `DebugLog`, assets). Application includes Platform too. The real rule is "dependencies only point down, and a layer may skip levels". Framework never includes Game, so that part is true. | Redraw with skip arrows, or say "points down, never up". |
| Runtime lifecycle diagram | Frame loop = Update → mutation point → Render, with a shutdown box | No scene change or battle hand-off in the loop. In `GameController::Update` the order is: audio → paused-menu branch (scene frozen) → scene update → scene-change request → dialogue advance → battle request. That is the most interesting part and it's absent. | Redraw (see §6). |
| Runtime vocabulary diagram | `BaseScene` → `WorldScene` only | `BattleArenaScene` is a sibling of `WorldScene`. `BoatScene` and `StorageRoomScene` are thin subclasses that only bind a map path and mask ID. | Add the three concrete scene classes. |
| World & rendering: mask text | "Black blocks, white is ordinarily walkable" | Black and out-of-bounds always block. Any other color is walkable unless it's a registered region whose current behavior is `collision` (`Player::CanOccupy`). | Reword; small. |
| Asset map: dynamic objects | "Its sprite… must already resolve" | True for `SpawnObject`. But objects placed in layers get a fallback: if `sprite:` isn't a registered ID, `ResolveObjectSprite` treats it as a path and loads it into scene scope. | Mention the fallback, or decide it's an implementation detail and leave it out. |
| Legacy page | "`DialogueWindow` renders text and portrait pages" | `Say` accepts an optional portrait ID, but no content uses it yet. | Say "supports a portrait (unused so far)". |
| Overview meta / everywhere | "NPC duties" presented as a working feature | Crew 1's `duties.lua` waits on triggers `mop_right` / `mop_left`, but `crew1.obj.yaml` only defines an `interaction` trigger. The coroutine starts, logs once, then appears to wait forever. | Describe duties as "wired up, no real duty yet", or fix the content first (see §3). |
| Battle page | "level-up processing … unimplemented" | Correct, but `AddExperience` also doesn't cap XP. If XP ever reaches the threshold, the next save will fail its own load check (`experience >= threshold` is rejected). That can't happen today (27 of 100 XP). | Docs are fine. This is a code note (§3). |

Everything else I spot-checked held up. That includes damage math
(12×1.0−2 = 10, 12−3 = 9, 8−5 = 3, 9−5 = 4), rewards (12 + 15 XP, Rusty Dagger and
Crude Club), the `StartBattle` → return-coordinates path, save fields, load gates,
layer order, anchors `(90,300)`, `(148,300)`, `(119,300)`, `(119,165)`, the Lua
function list, the `scene → game` resource lookup order, CMake asset copying,
and the 1280×720 `fit`/`nearest` presentation.

## 2. What the site is missing

1. **How to play.** Controls: WASD or arrows to move, **E** to interact, **I** for
   inventory, **Esc** for the menu, **Enter** to confirm. The quest in order:
   1. Talk to the crew member.
   2. The key spawns on deck. Pick it up.
   3. Unlock the storage door.
   4. Walk into the storage room. The goblin fight starts.
   5. Win and take the lantern.
   6. Back on deck, the sea has turned to lava and the deck is on fire.
   7. Use the lantern from the inventory at the bow (green region). Fairies
      come out and the water returns.

   This one page would ground every other page. Right now a reader has to put
   the story together from tag names.
2. **"How do I add…" recipes.** A new scene needs a map YAML, a thin `WorldScene`
   subclass, and a new branch in `GameController::ChangeScene` (the scene
   registry is hard-coded) plus `LoadGame`'s allowed list. A new object type
   needs a `*Load.cpp` and a branch in `SceneBuilder::CreateGameObject`. These
   are the questions a contributor will actually have.
3. **Object type list.** `spriteObject`, `scrollingSurface`, `player`,
   `interactable`, `enemy`, `playerBattle`, with one line each and the YAML
   that uses them.
4. **Pause vs. dialogue behavior.** The menu freezes the scene. Dialogue keeps
   the world animating but blocks input. Readers find this kind of detail
   satisfying, and it's easy to show.
5. **Enemy stat scaling.** `ScaleBattleStat` adds 10% per level above 1. The
   current goblins are level 1, so it's invisible, but it's a real rule.

## 3. Things I noticed in the game itself (not doc problems)

These are for you to decide. I haven't touched any code.

- **Rain never happens.** `WeatherSystem` supports `weather=rain`, but no
  script sets that tag. Crew 3 jokes about being a rain dancer, which sounds
  like a planned hook.
- **The flames never go out.** `storageLantern_flames_active` is set when you
  take the lantern and never cleared, so the four flames stay after the fairies
  restore the water. That might be intended.
- **Crew 1's duties stall** (see the table above).
- **The XP cap** (see the table above). It only matters once a second battle
  or bigger rewards exist.
- **Only the enemy you hit attacks back.** The other goblin never acts. The
  docs describe this accurately. It's just worth knowing that it's a design
  choice and not an accident.

## 4. Why it reads as AI-written

- **Abstract nouns doing the work.** Words like "authoritative", "contextual
  meaning", "narrow validated commands", "safe lifecycle points",
  "deterministic", and "durable causes" stand in for concrete things. Real
  examples (the door, the key, the goblins) appear late, if at all.
- **The same reassurance on every page.** "Not a general-purpose engine",
  "never disguise it as architecture", and "the code remains authoritative"
  appear in the overview cards, the sidebar note, the scope-guard callout,
  ledes, and tables. Saying it once on the overview is enough.
- **Meta-documentation.** The overview opens with three cards about how the
  docs are written ("The page carries meaning…"). Readers want the game first.
- **Every diagram has the same caption** ("Use the visual for orientation; the
  article below defines…"). That's boilerplate from `diagram()` in `app.js`.
- **Rhythm.** There are lots of "X, not Y" sentence pairs and triplets, and
  every page follows the same template (lede → diagram → prose → table →
  callout → sources).
- **Internal process leaking out.** "Decision owner: Jamie", "Documentation
  rule", and "Status: implemented" chips are notes for contributors, not for
  visitors.

### Voice sample (for you to approve before I rewrite everything)

**Now (Battle flow lede):**
> A battle is a temporary scene replacement, not a stacked mode. Lua requests who
> participates and proposes enemy intent; YAML supplies definitions; C++ validates,
> sequences, resolves, rewards, and returns to the exact world position.

**Proposed:**
> Step onto the green tiles in the storage room and two goblins jump you. Behind
> the scenes, the storage room is thrown away completely and the battle arena is
> built in its place. When you win, the game rebuilds the storage room and drops
> you back on the exact pixel you left. Nothing is "paused" in the background, so
> this page explains how the game remembers enough to make that return feel
> seamless.

**Now (State & save/load, "Facts, not a world dump"):**
> The active `WorldScene` supplies only its scene ID and the player's exact world
> position when saving. Mutable scene objects, current ground-region behavior,
> menus, dialogue, effects, and Lua coroutines are deliberately absent from the file.

**Proposed:**
> A save file is tiny: where you're standing, your stats and items, and a list of
> true/false facts like `storageLantern_door_open`. It never stores the door object
> itself. On load, the boat is built fresh from its map and `init.lua` reads those
> facts: door open? Then remove the door and turn its doorway into a warp. Same
> result, far less to go wrong.

The approach: lead with what the player sees, then show the mechanism, and
name real files and tags as the examples.

## 5. Page by page

| Page | Verdict | Notes |
|---|---|---|
| Overview | **Rewrite** | Drop the three meta "principles" cards and the scope-guard callout. Open with a screenshot or the game map, one paragraph on what the slice is, then the page grid. |
| *(new)* Play the slice | **Add** | Controls, quest walkthrough, which tags each step sets. Ideally with real screenshots. |
| System topology | **Keep, fix** | Fix the dependency claim. The "Owns / Does not own" table is good; keep it. The boundary-test callout is good; keep it. |
| C++ · Lua · YAML | **Keep, rewrite** | The best content is the "A concrete interaction" paragraph and the question/owner table. Lead with those (the door example) and shrink the abstract lane cards. |
| Runtime lifecycle | **Keep, rewrite** | The step list is accurate. Add scene change, battle request, pause, and dialogue to the frame loop. |
| World & rendering | **Keep** | Strong, concrete page (real anchors, real layer names). Fix the mask sentence. Could split anchors and ground regions into their own short sections with an image. |
| State & save/load | **Keep, tighten** | Accurate and well-structured. The step list and the load-flow diagram overlap heavily; trim the steps to the three surprising ones (commit point, owners keep identity, rebuild via init Lua). |
| Battle flow | **Keep, rewrite voice** | Content is accurate and has good numbers. Two callouts about "current scope" can merge into one. |
| Asset dependency map | **Keep, simplify** | Two big tables plus two diagrams is heavy. Replace the abstract graph with one concrete chain (boat map → crew1 → sprite → PNG → rule table → Lua). Keep the lifetime table. |
| Legacy correspondence | **Trim** | The diagram and first table say the same thing, and so do the second diagram and second table. Keep one of each pair. This page is a nice story and could be friendlier ("what we kept from 2002"). |
| Source index & glossary | **Keep, reorganise** | The glossary, suffix table, and Lua table are genuinely useful reference. Drop the repository-map diagram (a folder tree in a code block is clearer). Add the object-type list. |

Site-wide:
- Replace the boilerplate diagram caption with a real caption per diagram.
- Remove the sidebar "Documentation rule" note (move it to `README.md`).
- Turn the "Trace it in the repository" path lists into GitHub links.
- Drop the meta chips (Decision owner, Status) from page headers, or keep one
  short, useful line instead.

## 6. Diagram by diagram

Legend: **Keep** = accurate and earns its place. **Fix** = small corrections.
**Redraw** = same idea, clearer picture. **Replace** = different, more concrete
picture. **Drop** = a table or text already covers it.

| Diagram | Verdict | Why |
|---|---|---|
| `system-topology.svg` | **Fix** | Clear, but implies a strict chain. Add "Game → Platform" and "Application → Platform" skip arrows (or a note). |
| `language-boundaries.svg` | **Replace** | Three columns of abstract bullets. Replace with the door interaction as a five-step strip: YAML places door → C++ picks nearest + E → rule table picks script → Lua `Say`/`SetTag` → C++ applies after the frame. This shows the same idea through something the reader has played. |
| `runtime-flow.svg` | **Redraw** | Accurate but incomplete. Add the scene-change/battle branch and the pause and dialogue gates. The current dashed shutdown loop is confusing. |
| `render-stack.svg` | **Keep** | Good. Uses real boat layer names. Maybe add a tiny thumbnail per layer later. |
| `world-geometry.svg` | **Redraw** | The idea is great, but the "Player sprite" is a grey box and the four anchors bunch together at the bottom. Draw it on the real player frame (enlarged) and show a crop of the real boat `mask.png` with its red door and green bow regions. That would be the best "aha" picture on the site. |
| `state-ownership.svg` | **Drop or merge** | Overlaps the save/load diagram and the "Saved field" table. If kept, show a real `save.yaml` next to it instead. |
| `save-load-flow.svg` | **Keep** | Clear, accurate gates and commit point. Minor arrow tidy. |
| `battle-round-trip.svg` | **Redraw** | Accurate but tangled: crossing arrows, a floating "bind references" label, overlapping text. Redraw as one clean loop: Storage room → `BattleRequest` → Arena → Victory → `SceneChangeRequest` → Storage room, with persistent state in the middle. |
| `battle-turn-flow.svg` | **Fix** | Good content. Simplify the return arrows. Explicitly say "only the enemy you hit responds". |
| `game-map.svg` | **Replace** | The most important picture, and the vaguest. Make it a numbered quest map (steps 1–7 from §2), ideally over thumbnails of the boat and storage backgrounds, with the tag set at each step. |
| `asset-reference-graph.svg` | **Replace** | Arrows cross and one edge appears to go from `*.rules.yaml` to Layers. Replace with one concrete chain from `boatscene/map.yaml` down to `crew.png` and `dialog_find_key.lua`, colored path vs. ID edges. |
| `asset-lifetime.svg` | **Keep** | Accurate and readable. |
| `legacy-correspondence.svg` | **Keep** | Clean. Shorten the matching table instead. |
| `modernization-disposition.svg` | **Drop** | A hub of five lists that repeats the disposition table word for word. |
| `runtime-vocabulary.svg` | **Fix** | Add `BattleArenaScene`, `BoatScene`, `StorageRoomScene`. Consider moving it to the Runtime page. |
| `source-index.svg` | **Drop** | A folder listing drawn as boxes. A plain annotated tree is clearer and easier to keep current. |

**Totals:** keep 4, fix 3, redraw 3, replace 3, drop 3. Two drops are pure
duplicates, and dropping or merging `state-ownership` depends on what you want.

## 7. Decisions for you

1. **Voice.** Do the samples in §4 feel right? A little more playful, or more
   neutral?
2. **Screenshots.** Can the site use real in-game screenshots and art (player
   sprite, mask, backgrounds)? They would make the redrawn diagrams much better.
   Check provenance first, since the project is careful about media.
3. **New "Play the slice" page.** Add it?
4. **Diagram changes.** Approve the keep, fix, redraw, replace, and drop list,
   or tell me which to argue about.
5. **Game issues in §3.** Document them as they are, or fix some first (duties
   and rain especially)? If you fix them, I'll document the fixed behavior.
