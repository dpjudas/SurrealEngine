# Bugtracker

- "controller models" (the circles with a line) are to large, they should be 30% the size
- when loading a game, the loaded game tends to crash after a few seconds
- when pulling the trigger with a "charging" weapong (e.g. dispersion pistol, impact hammer, rocket launcher) the hold action does not properly work. the rocket launcher for example fires a single rocket immediately, then starts charging the rockets. the same behaviour with the alt fire mode
- ~~in unreal tournament and in unreal gold the sprites of explosions, weapon impacts, etc. tend to face the wrong way - usually they should align with the players camera~~ FIXED (phase 8): sprites billboarded off `ViewRotation` alone, which in VR is only the camera's body-yaw rotation (phase 5's aim/view split keeps it pinned to the body anchor) - never the headset's actual pose. New `VisibleFrame::HeadLocalToWorld()` combines the two; applied to `VisibleSprite::Draw` and the wheel's icon fallback.
- on level transition (first to second level) the translater is lost - it is not in the item list anymore
- translater model in the weapon wheel (an in the hand when selected) is fully black for some reason. The model of the flare looks correct tho. 
- it is impossible to destroy glass or activate movers on a loaded level
