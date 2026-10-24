# Temple flashlight / night vision gesture

Either existing temple activator, empty eligible hand, lower grip:
- release before 650 ms: toggle native torch;
- hold at least 650 ms: toggle native night vision once, stronger pulse;
- releasing after a long hold does not also toggle the torch;
- leaving the activator or losing tracking cancels the pending gesture;
- 250 ms release cooldown; fresh grip press required.

Native equipment and night-vision availability checks remain in CActor::SwitchNightVision. No game launch was performed.
