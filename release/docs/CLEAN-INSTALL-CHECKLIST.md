# Release gate / Перевірка перед публікацією

Candidate: **0.1.0-alpha.2**. A checked item must have a date, tester and result. Leave untested items unchecked. The automated extraction check does not replace interactive/headset checks. No public upload until the owner approves the accepted candidate.

## Automated / Автоматично

- [ ] Native engine + bridge final build succeeds / Фінальна збірка рушія й bridge.
- [ ] All 24 pure/ABI tests pass / Усі 24 тести.
- [ ] Resource/geometry/audio validators and hashes pass / Ресурси, звуки й хеші.
- [ ] Installer verification extraction into a new empty folder passes; original archives/config remain byte-identical / Розпакування без зміни оригіналів.
- [ ] Launcher `--self-check` passes in extracted folder / Перевірка комплектності лаунчера.
- [ ] Normal and delay-loaded dependencies classified; no required bundled DLL missing / DLL-перевірка.
- [ ] No private saves, logs, local user paths, test kits or unlimited weight / Немає персональних даних та тестових читів.

## Interactive clean install / Чиста установка вручну

- [ ] Close the development game. Keep the development build separate / Закрити гру, не змішувати з розробкою.
- [ ] Install with the actual Setup EXE into a new folder, using original unmodified Steam game / Встановити справжнім інсталятором у нову папку.
- [ ] Verify Steam detection and manual folder selection, error for wrong folder / Автопошук і помилки вибору.
- [ ] Verify no administrator prompt is required for default location / Без адміністратора.
- [ ] Check desktop shortcut, Installed Apps entry, EN/UK launcher controls and offline HTML guide / Ярлик, запис видалення, обидві мови й інструкція.
- [ ] Launch with USB Link and confirm stereo menu, both Touch rays, new game/loading / USB Link, меню, промені, нова гра.
- [ ] Verify current-config graphics defaults and that in-game changes survive relaunch / Графіка та її збереження.
- [ ] Toggle zone outlines, Apply, return to game, relaunch and confirm persistence / Видимість зон і збереження.
- [ ] Calibrate upright, lean, physically crouch; verify movement speed/sprint/collision / Калібрування, нахил, присяд.
- [ ] Verify left/right weapon grips, support aim, physical reload, jam clearance, bolt/knife/grenade/detector/binocular / Основні взаємодії.
- [ ] Verify chest/quick/rear/head/temple zones without overlap replacing held items / Зони без випадкової заміни предмета.
- [ ] Verify world grabs/throws/quest pickup and held-object camera stability / Лут, кидки, квестові предмети, камера.
- [ ] Inventory, trade, loot, PDA, sleep, dialogue and wrist UI / Усі інтерфейси.
- [ ] Confirm the depth-flicker fix at wall/ground intersections; binocular/scope near-eye behavior / Мерехтіння й оптика.
- [ ] Save → reload; die → load at least twice; verify no crash / Збереження і двічі завантаження після смерті.
- [ ] At least one sustained session: record duration, frame timing and crashes / Тривала сесія з результатами.
- [ ] Uninstall: managed files removed, data preserved, original game launches normally / Видалення зі збереженням data та оригінальної гри.
- [ ] Reinstall and portable ZIP flow checked / Повторна установка й portable.

## Publish / Публікація

- [ ] Update feature-status tables with these results; do not erase known unverified items / Оновити таблиці фактичними результатами.
- [ ] Review attribution, notices, assets, source patch and exact package hashes / Перевірити атрибуцію й пакет.
- [ ] Record GitHub owner/repo, tag, actual release URL; replace Steam draft marker / Вказати справжні реквізити.
- [ ] Owner approves publication of this exact checksum / Власник дозволяє публікацію саме цього пакета.

Owner reported installation and launch working on **2026-10-25** and authorized publication. This does not individually certify every unchecked regression item. / Власник підтвердив роботу установки й запуску **2026-10-25** та дозволив публікацію; окремі неперевірені регресійні пункти лишаються відкритими.
