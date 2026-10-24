# Attribution and distribution scope

Call of Pripyat VR is an unofficial free, noncommercial modification. Original game archives, levels, saves and music are not distributed. The original Steam game is required. Names and game assets belong to GSC Game World and their respective owners. Modified models, textures, UI and short reload-sound excerpts in `release/gamedata` are game-derived mod assets, not MIT-licensed original art.

- GSC: https://www.gsc-game.com/eula/ and https://www.gsc-game.com/guidelines/ (reviewed 2026-10-24). This package requires the original game and does not claim official status. Those terms remain applicable; this notice is not an independent grant of rights to game content.
- OpenXRay: https://github.com/OpenXRay/xray-16, base `c62d9d343981d50f6ed176412b85d94c61b578b5`. Changes are supplied as `patches/openxray-vr.patch`; retain both OpenXRay and original X-Ray notices in `release/licenses/`.
- Khronos OpenXR headers and loader: Apache-2.0, accompanying license texts; header revision is in `third_party/openxr/REVISION.txt`. Loader DLL version 1.1.65. System runtime is provided by the user, not bundled.
- LuaJIT, luabind, ImGui, LuaFileSystem, lua-pack, GameSpy and AMD AGS: upstream license texts are included. The pinned OpenXRay submodules supply their corresponding source and build configuration.
- SDL 2.32.4: zlib license, included. https://github.com/libsdl-org/SDL/tree/release-2.32.4
- OpenAL Soft 1.24.3: LGPL-2.0-or-later, dynamically linked; license and corresponding upstream source ZIP are included in `release/licenses/`. Users may replace the OpenAL DLLs with compatible rebuilt versions. Source: https://github.com/kcat/openal-soft/tree/1.24.3. The binary is from the OpenXRay SDK. No CoPVR modifications to this library.
- OpenSSL 3.3.2: Apache-2.0, license included; https://github.com/openssl/openssl/tree/openssl-3.3.2. `libcrypto-3-x64.dll` is from the OpenXRay SDK, unmodified by CoPVR.
- BugTrap: original IntelleSoft/BugTrap notices and upstream license included; https://github.com/bchavez/BugTrap. Binary supplied by the pinned OpenXRay SDK.
- ODE/OPCODE and statically linked X-Ray SDK components retain upstream notices. ODE/OPCODE source used by this build is included in the pinned OpenXRay tree. Additional notices are in `release/licenses/`.
- Microsoft Visual C++ runtime and Direct3D compiler binaries retain Microsoft's license. Bundled runtime files are the application-local redistributables used by the OpenXRay build; they are not covered by this repository's MIT license. Windows and .NET Framework are not bundled.

The release build disables Discord presence integration; the Discord SDK DLL is omitted. NVIDIA Ansel is an optional delay-loaded desktop feature and its SDK DLL is omitted from this VR package. No third-party source tree is assigned a new CoPVR license. GitHub automatic source archives alone do not contain the pinned engine; use the documented bootstrap command to retrieve it.
