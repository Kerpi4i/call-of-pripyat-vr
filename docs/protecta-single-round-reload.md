# Protecta single-round reload

Protecta has internal kind 4 (Rotary), capacity remains the native 12 rounds. Rear waist reload chord supplies one existing full-size 12g shell. Release at the weapon port, initially the native HUD shell-point (0.020, 0.072, 0.228), consumes exactly one compatible cartridge. No standard reload animation or full-magazine fill is started.

Topping up a nonempty, ready gun preserves readiness. Loading the first round into an empty gun sets a persistent cocking requirement; further rounds cannot clear it. Operate the existing physical zatvor_jnt charging handle with a full rear stroke and release to prepare firing. Ordinary shots do not require another cocking cycle. A partial handle stroke does not clear the requirement. Existing save-extension state retains readiness.

Internal loader tests cover ready top-up, empty first load, continued loading while not cocked, partial stroke rejection, completed cocking, and subsequent shots. Asset/audio validators cover the sixth profile.

RG6 has not been converted: its existing rear-waist reload activator released over the generic reload port invokes native reload and fills available capacity, with native projectile ownership/spawning. It still uses an ammo-box proxy. A physical single-VOG drum mechanism would be a separate implementation.

Game launch and in-headset alignment are pending user authorization.
