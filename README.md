# cheat-division

![picture alt](https://raw.githubusercontent.com/nkga/cheat-division/master/doc/img/divsion.jpg "Cheat sample")

Cheat for Tom Clancy's The Division written in C (April 2016). Features included:
- Aimbot
- ESP
- No recoil
- No spread
- Infinite ammo
- Infinite magazine
- Speedhack
- Teleport to enemy
- Teleport to crosshair
- Teleport save/load position
- Player radar
- Crafting material ESP and teleport

### Background

Months after release The Division featured very little in the way of server 
side verification for client side actions, a game broken by design. 

### Usage

1. Rename mod_div.dll to mod.dll and place in the game's working directory.
2. Inject loader.dll into the game with your a DLL injector for Windows.
3. Refer to keybindings in the source code for different modules.

### Notes

- Features a fairly efficient sprite and text renderer (D3D11, HLSL)
- Fullscreen transitions may break the rendering overlay, but Ctrl+F9 can reload it.

### Building

1. Install [Visual Studio](https://www.visualstudio.com/).
2. Open `cheats.sln`.
3. Change solution configuration to `Release`.
4. Build solution.
