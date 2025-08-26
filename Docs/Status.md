# Surreal Engine - Current Status

---

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
* Some native functions related to text rendering aren't implemented, resulting in glitchy texts.
* Mirrors/reflections rendering isn't implemented.
* Portals aren't implemented. They do not render or "teleport" the player, and crash the engine under certain conditions.
* No dynamic lighting support (Dispersion Pistol projectiles/Flares don't illuminate their surroundings)
* Bot and Scripted Pawn AI isn't fully functional due to the related native functions not being implemented.
All they currently do is to sometimes retaliate if attacked, and pick up nearby items.
* Bots literally rotate their entire body (feet off the ground) to look up/down in ways they shouldn't be able to.
* Waving water textures at the end of waterfalls render broken. Easily observable in NyLeve's Falls (Unreal) or DM-ArcaneTemple (UT)
* Inventory travelling is quite buggy: Either they travel, but get deselected upon the next level load, or cause a crash.
And not all travel types are implemented.
* Player-to-decoration and player-to-other-pawns collisions aren't properly implemented. The player usually gets stuck and if they manage to get free
they end up dying because SE thinks that they've fallen from a great height.
* If the player currently has a power-up active that tints the screen (like Invisibility or Energy Amplifier), the tint remains
(and accumulates if a new power-up of the same type is picked up and activated) until the player switches to a different map.
* viewclass command crashes with null deref.
* Sometimes opening a map crashes the engine with a "Failed to spawn the player actor" error.
* Third person views don't work properly as the player character is not rendered.
* Teleporters do not work properly sometimes.
* Zones don't apply velocity to the players when they should (Like the conveyor belt in DM-Conveyor)
* Zones don't hurt the player when they should (Drowning in water/getting burned in lava, etc.)
* There is no OpenGL renderer.
* There is no networking support at all.
* By design, native mods will never work with SE. Thankfully these kind of mods are extremely rare.
* **[Linux/ZWidget]** - Wayland backend is somewhat buggy due to Wayland APIs being unpleasant to work with, to name a few things:
  - Menus don't properly position themselves.
  - Sometimes menus remain persistent (due to rapidly switching?)
  - Running any ZWidget app (launcher/SurrealEditor) on GNOME will probably lead to not being able to move the window around, 
  because ZWidget has no custom window decorations (ZWidget uses server side decorations when they're available)
  - No clipboard support yet

## General Engine bugs that might need further testing
* There seem to be a memory leak due to Garbage Collection not being implemented. This is probably extremely hard to notice as UE1 games 
won't use much memory when run on modern computers. Might need to run SE for a looooong time before it is noticeable.
* Gibbing on certain maps in Debug mode causes out of bounds vertex access
* Lightmap rendering is kind of off

## Unreal (Gold)

226 version of the game launches. Menu options works most of the time. Single player maps can be played, as well as botmatches. The AI will behave more or less the same as how they behave in UT.

### Known Bugs:
* [227*] Many new native functions/features are not yet implemented.
* Fighting Skaarj can sometimes lead to crashes.
* Firing sounds don't seem to play with Eightball Gun and RazorJack.
* Zones are extremely broken, especially water zones. Easily observable with DmAriza.
* Nali Fruit Seeds and ASMDs in a map don't render, but are pickable.
* SE crashes when trying to finish Vortex Rikers.
* Extreme lag occurs while DuskFalls is the current map.
* Trying to open up Chizra crashes the engine.
* ASMD tertiary fire rings render wrong.

## Unreal Tournament

436 version of the game launches. Menu options will work and botmatches can be played, however the bots will barely have any AI (they move around sometimes and retaliate upon being attacked), and some maps might have some functionality missing.

### Known bugs:

* [469*] Many new native functions/features are not yet implemented.
* When compiled with GCC or Clang, the engine can crash after running for a second. This doesn't happen with MSVC.
* Shock Rifle beams render glitchy.
* Player can slide through on some of the catwalks in the room with lava in DM-Conveyor

## Deus Ex

1112fm version of the game launches. Sadly, only the intro flyby works.

### Known bugs:
* Keyboard (and mouse?) input doesn't work. No keys are recognized, preventing us from testing literally everything else.
  - Might be because input stuff is moved to native Extension package, which isn't implemented?

## Tactical-Ops: Assault on Terror

### Known bugs:
* Currently crashes immediately due SE being unable to find "TO_Replacer.ini".
* Attempting to bring up the menus crashes the engine.
* Opening up any map crashes the engine.

## Any other UE1 game not mentioned above

They crash the engine upon startup for various reasons. Will probably take quite a while before the necessary functionality gets implemented.
