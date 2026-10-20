# Atlas shot list

Drop each file into `docs/site-docs/assets/media/shots/` using the exact name
below. Until a file exists, the page shows a striped "Screenshot coming"
placeholder in its place, so you can see where everything will go.

The game renders at 1280 × 720, so capture at that size. The site scales images
down, and pixel art stays crisp because the images use `image-rendering:
pixelated`. For GIFs, aim for 10–20 seconds and keep each one under about 5 MB.
A 640 × 360 GIF is plenty if a full-size one gets too large.

| File | Page | What to capture |
|---|---|---|
| `overview-deck.png` | Overview (top) | A good-looking still of the deck at the start: player in view, a crew member or two, no dialogue box. |
| `play-01-opening.gif` | Play, step 1 | The opening cutscene from a fresh start: the lookout walks down from the helm to the red carpet and delivers the mission lines. |
| `play-02-key.gif` | Play, step 2 | Walk to the key, let the E prompt appear, pick it up, show the pickup line. |
| `play-03-door.gif` | Play, step 3 | Try the door, unlock it (door disappears), walk through the doorway into the storage room. |
| `play-04-ambush.gif` | Play, step 4 | Walk up the storage room onto the trigger, the battle starts, a couple of attacks, and ideally the victory screen. |
| `play-05-lantern.gif` | Play, step 5 | Arrive back in the room after the fight, walk to the lantern, pick it up. |
| `play-06-lava.gif` | Play, step 6 | A still of the deck with lava around the ship and the four fires burning. |
| `play-07-fairies.gif` | Play, step 7 | Stand on the bow, press I, use the Guiding Lantern; fairies appear and the lava fades back to water. |
| `play-08-rain-finale.gif` | Play, step 8 | Talk to the rain mage after the fairies: the walk to the chest, "Here we go!", rain, the camera visiting each fire, and the fade to black. Speed it up if it runs long. |
| `cutscene-route.gif` | Cutscenes | Just the rain mage's six-leg walk to the red chest, with the camera following. |
| `ending-card.png` | Cutscenes | The black "To Be Continued" card, once the text has fully faded in. |
| `world-water.gif` | World & rendering | 4–6 seconds of open water beside the hull, so the scroll and the two-frame crossfade are visible. |
| `world-prompt.png` | World & rendering | A close-up crop around the storage door (or a crew member) with the E prompt showing. Cropping is fine. |
| `dialogue-typewriter.gif` | Runtime lifecycle | 3–5 seconds of a dialogue line typing out while the world keeps animating behind it. |
| `menu-pause.gif` | State & save/load | The Esc menu open on the deck: Save, Load, Inventory, Status, Quit. |
| `battle-strike.gif` | Battle flow | One full attack, ideally slowed down: wind-up, impact (sound plus goblin flinch), damage number or HP change. |
| `legacy-2002.png` | Legacy correspondence | Optional: the original 2002 prototype running, if it can still be launched. Delete this line from `app.js` if not. |

Nice-to-have, not wired into a page yet (tell Claude if you make them):

- `menu-inventory.png`: the inventory with the key or the lantern selected.
- `battle-victory.png`: the victory screen showing +27 XP and both items.
- `storage-room.png`: a still of the storage room with the lantern visible.

## Already copied from the game

These live in `assets/media/` and were generated from the game's own art:

- `boatscene-thumb.jpg`, `storageroom-thumb.jpg`: half-size scene art (background and foreground combined)
- `boatscene-mask.png`, `storageroom-mask.png`: the real collision masks
- `battlearena-thumb.jpg`: the arena backdrop, cropped the way the game shows it
- `player-idle-down.png`: one frame of the walking player sprite

Cropped versions of these are also embedded directly inside `game-map.svg` and
`world-geometry.svg`, because an SVG shown through an `<img>` tag can't load
outside image files.
