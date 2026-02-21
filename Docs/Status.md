# Surreal Engine - Current Status

The engine can load and render the maps. The Unrealscript VM is almost feature complete - only arrays and network conditional execution are not implemented yet.

It will attempt to load all level actors and initialize the map. However, while the menus and the HUD will appear, there are still many native functions not implemented yet. It is also quite possible some events aren't firing as they should. You will therefore see exceptions shown if you interact with them and that is where the project is at.

At the time of this writing, SurrealEngine can **detect** the following UE1 games:

* Unreal Tournament (v436, v451b, v469(a, b, c, d))
* Unreal (v200, v209, v220, v224v, v225f, v226f)
* Unreal Gold (v226b, v227(i, j, k_11))
* Deus Ex (v1002f, v1112fm)
* Klingon Honor Guard (219)
* NERF Arena Blast (v300)
* TNN Outdoors Pro Hunter (v200)
* Rune Classic (v1.10)
* Clive Barker's Undying (v420)
* Tactical-Ops: Assault on Terror (v3.4.0 and v3.5.0 - both running under UT436 and UT469 engines)
* Wheel of Time (v333)

> [!NOTE]
> From the list above, only Unreal Tournament v436 and Unreal Gold v226 is in a relatively playable state. Running any other game (and UT versions) can and will result in crashes.


## General Engine bugs (affects all games)
* Movers:
  - Some buttons are too easy to push, like the ceiling button in Kevlar Suit room in Vortex Rikers.
  You just need to pass under it while in the original you'd have to bump into it
  - Projectiles pass through some movers
  - It is possible to get stuck on some movers when approached from certain angles
* Mirrors/reflections rendering is a bit buggy.
* Portals:
  - They somewhat work, but might end up "pushing" players/projectiles in unexpected directions.
  - No portal rendering yet.
* Semisolid brushes are finicky. Usually you'll fall through them as if they weren't there.
* No dynamic lighting support (Dispersion Pistol projectiles/Flares don't illuminate their surroundings)
* Bot and Scripted Pawn AI isn't fully functional due to the related native functions not being implemented.
All they currently do is to sometimes retaliate if attacked, and pick up nearby items.
* Bots literally rotate their entire body (feet off the ground) to look up/down in ways they shouldn't be able to.
* Waving water textures at the end of waterfalls render broken. Easily observable in NyLeve's Falls (Unreal) or DM-ArcaneTemple (UT)
* Underwater collisions are somewhat buggy, especially on Klingon Honor Guard
* Player-to-decoration and player-to-other-pawns collisions aren't properly implemented. The player usually gets stuck and if they manage to get free
they end up dying because SE thinks that they've fallen from a great height.
* If the player currently has a power-up active that tints the screen (like Invisibility or Energy Amplifier), the tint remains
(and accumulates if a new power-up of the same type is picked up and activated) until the player switches to a different map.
* viewclass command crashes with null deref.
* Sometimes opening a map crashes the engine with a "Failed to spawn the player actor" error.
* Third person views don't work properly as the player character is not rendered.
* Inventory from loaded saves do NOT transfer to the next map.
* Saving packages (.u[xx] files, game saves, etc.) functionality is not fully implemented yet.
* There is no OpenGL renderer.
* There is no networking support at all.
* By design, native mods will never work with SE. Thankfully these kind of mods are extremely rare.
* **[Linux/ZWidget]** - Wayland backend is somewhat buggy due to Wayland APIs being unpleasant to work with, to name a few things:
  - Menus don't properly position themselves.
  - Sometimes menus remain persistent (due to rapidly switching?)
  - Running any ZWidget app (launcher/SurrealEditor) on GNOME will probably lead to not being able to move the window around, 
  because ZWidget has no custom window decorations (ZWidget uses server side decorations when they're available)

## General Engine bugs that are not confirmed yet

* There seem to be a memory leak due to Garbage Collection not being implemented. This is probably extremely hard to notice as UE1 games 
won't use much memory when run on modern computers. Might need to run SE for a looooong time before it is noticeable.
* Gibbing on certain maps in Debug mode causes out of bounds vertex access
* Lightmap rendering is kind of off

## Unreal (Gold)

226 version of the game launches. Menu options works most of the time. Single player maps can be played, as well as botmatches. The AI will behave more or less the same as how they behave in UT.

### Known Bugs:
* [227*] Many new native functions/features are not yet implemented.
* Some UPak native functions are not implemented yet.
* Firing sounds of Eightball Gun and RazorJack are immediately "drowned" by their projectiles' sounds.
* Nali Fruit Seeds and ASMDs in a map don't render, but are pickable.
* ASMD tertiary fire rings render wrong.

## Unreal Tournament

436 version of the game launches. Menu options will work and botmatches can be played, however the bots will barely have any AI (they move around sometimes and retaliate upon being attacked), and some maps might have some functionality missing.

### Known bugs:

* [469*] Many new native functions/features are not yet implemented.
* Shock Rifle beams render glitchy.
* Player can slide through on some of the catwalks in the room with lava in DM-Conveyor

## Deus Ex

1112fm version of the game boots to title screen but you'll get stuck because the menus don't work yet. You can load maps with `--url` command and play them, e.g. `SurrealEngine --url=00_Training.dx /path/to/deusex` but expect crashes.

### Known bugs:
* Keyboard and mouse inputs do not work on menus. If you accidentally opened one of those you'll get stuck.
* Attempting to initiate a dialog will crash the game.

## Tactical-Ops: Assault on Terror

### Known bugs:
* Currently, game crashes immediately due to SE being unable to find "TO_Replacer.ini".
* Attempting to bring up the menus crashes the engine.
* Opening up any map crashes the engine.

## Klingon Honor Guard

1.1 patch of the game launches. AI behaves more or less the same as the other games.

Known bugs list is quite small right now because this game isn't tested as much as Unreal and UT yet.

### Known bugs:
* Cannot set keybinds in-game (Modifying the ini file works though).
* Trying to start a botmatch crashes SE.

## Any other UE1 game not mentioned above

They crash the engine upon startup for various reasons. Will probably take quite a while before the necessary functionality gets implemented.
