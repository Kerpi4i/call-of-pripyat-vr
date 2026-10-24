# Перевірка 2026-10-21

- `tools/diagnose.ps1`: успішно; локальна Steam CoP 1.6.02 x86,
  зареєстрований Meta OpenXR x64 runtime.
- MSVC 2019 встановлено, але збірка зупинилася через відсутній `crtdbg.h`
  (Windows SDK/UCRT). Повний OpenXRay не зібрано.
- Portable Zig 0.15.2: архів перевірено за SHA-256 із офіційного індексу.
- `tools/build-portable.ps1`: успішно, exit 0.
- `src/openxr_input.cpp` скомпільовано в `build/openxr_input.obj` і
  `build/libcopvr_input.a`. Це статична бібліотека для розробки, не DLL-мод гри.
- `build/interaction_tests.exe`: успішно. Перевірено захоплення, початок/зупинку
  стрільби, втрату фокуса, повторне озброєння тригера, просторові слоти,
  втрату tracking, швидкість кидання, перетворення осей, одноосібне володіння,
  гістерезис захоплення, зайнятий слот, натиснутий тригер при старті та невалідний aim.

## Наступний етап — реальний OpenXR-сеанс

- Додано та скомпільовано `build/vr_diagnostic.exe` із D3D11 та app-local
  Khronos OpenXR loader 1.1.65, офіційний архів перевірено за SHA-256.
- Probe: Runtime Oculus, System Oculus Quest2, position/orientation tracking = 1.
- Створено два swapchains 2080×2096, по три зображення.
- Виправлено помилку RTV для typeless texture: явно вказується формат swapchain.
- Реальний 60-секундний запуск: exit 0; 4252 стереокадри; head valid 4252,
  left/right grip valid по 2992. Ці числа описують кількість валідних семплів
  у цьому запуску, не оцінку точності tracking або гарантовану частоту кадрів.
- У `reports/vr-run.log` є захоплення та відпускання предмета обома руками.
- Користувач підтвердив: «Так, зображення й руки працюють».
- `tests/stereo_tests.cpp`: asymmetric FOV, D3D depth range, inverse eye
  translation/head rotation, stereo parallax — PASS.

Підтверджено окрему діагностичну сцену. Ігровий рендерер, реальні предмети,
баллістика, збереження та інвентар CoP не інтегровані. DLL у гру не встановлювалися.

## Підготовка інтеграції

- Відокремлено OpenXR/D3D11 host від діагностичної сцени.
- Додано валідацію та підтримку зовнішнього D3D11 device.
- `--probe-shared-device`: exit 0; реальний Quest 2 runtime, session та два
  swapchains на device зовнішнього власника. Перевірено збіг COM pointer.
- Відновлено всі підмодулі OpenXRay і NuGet-пакети. Microsoft Windows SDK
  10.0.19041.5 (headers/tools + x64 libraries) отримано локальними NuGet-пакетами.
- `xrCore.dll` x64 Release успішно зібрано MSVC 2019. Виправлено локальні SDK
  property sheets для раннього імпорту налаштувань у LuaJIT host tools.
- Підготовлено окрему конфігурацію ресурсів у `game-test/`; оригінальні Steam
  файли не перезаписувалися. Це ще не підтверджує запуск і проходження гри.
- Повна ціль `Game` у `tools/build-engine.ps1`: exit 0; MSVC 2019 x64 Release,
  `xrEngine.exe`, `xrGame.dll`, `xrRender_R4.dll` і залежності зібрані.
  У сторонніх статичних бібліотеках є linker warnings щодо PDB і defaultlib;
  успіх компіляції не підтверджує відсутність усіх runtime-проблем.
- Зібраний exe запущено з `game-test/`. Процес відповідає; створено вікно
  `S.T.A.L.K.E.R.: Call of Pripyat`. У логах ініціалізовано D3D11, текстури,
  скрипти головного меню. Перевірка ігрової сесії ще не виконана.
- PowerShell parser: усі `tools/*.ps1` без синтаксичних помилок.
- Користувач підтвердив звичайний запуск зібраного OpenXRay:
  «Так, головне меню працює». Нова гра, завантаження локації та збережень
  у цій збірці ще не перевірялися.

## Інтеграція стереорендерера — поточний етап

- Додано C ABI між MSVC engine та `copvr_bridge.dll` (Zig C++ x64).
- Перебудова зміненого рушія: exit 0. Підключено два рендер-проходи,
  eye camera/frustum, синхронізацію CPU camera readers, копіювання backbuffer,
  lifecycle/reset cleanup, OpenXR pacing.
- `tests/bridge_abi_tests.cpp`: PASS — поточний ABI v2, layout 52/120 bytes, exports,
  відхилення невалідного device/frame/copy, повідомлення помилок, повторний shutdown.
- Повторно виконано interaction та stereo tests: PASS.
- Збережений `patches/openxray-vr.patch`; reverse `git apply --check`: exit 0.
- Запущено `xrEngine.exe -fsltx fsgame.ltx -noprefetch -copvr` з `game-vr/`.
  Процес відповідає; завантажено змінений R4, `copvr_bridge.dll` та
  `openxr_loader.dll`. Це **ще не підтвердження показу ігрових кадрів у HMD**.
  Візуальне підтвердження меню/локації та коректності head tracking ще потрібне.

## Перевірка на реальній локації та виправлення UI

- Перший інтегрований запуск подав понад 4500 проєкційних кадрів без API-помилок;
  користувач виявив неправильний вигляд плоского меню і проблему керування.
- Додано menu flag до C ABI v2, quad layer на відстані 2 м та відновлення фокуса.
- Повторний запуск: користувач підтвердив, що меню нормальне, нова гра завантажується,
  «у грі зображення нормальне, гарнітура відстежується» та
  «Так, стерео та рух голови працюють».
- Журнал підтверджує перехід із `menu_quad=1` до `menu_quad=0` та щонайменше
  4800 поданих VR-кадрів у цілому запуску. Архів журналу:
  `reports/engine-vr-world-validated.log`.
- Користувач виявив двоїння штатного прицілу та старий формат loading/intro.
  Додано повне відключення `CHUDTarget::Render` лише для `-copvr`, інтеграцію
  loading draws у quad layer та export стану intro sequencer із xrGame.
  Ці останні виправлення потребують окремої візуальної перевірки.
- Остання збірка цих виправлень: exit 0; `copvr_is_intro` та R4 bridge exports
  підтверджені `dumpbin /exports`. Оновлений запуск відкрито; відповідь щодо
  loading/intro/reticle ще очікується. PID конкретного запуску — у
  `reports/game-vr-pid.txt`; не використовувати старі PID з історії розмови.
- Користувач підтвердив останню перевірку: «Усе це працює правильно» —
  завантаження/інтро на просторовій панелі та відсутність штатного прицілу
  на локації. Разом із попередніми перевірками підтверджено меню, ввід для
  запуску нової гри, стереозображення локації та відстеження гарнітури.

## Touch: перша інтеграція зброї

- Додано окремий controls ABI v1 (CopVrHand 60 байтів, CopVrControls 136 байтів),
  локальні пози aim/grip у каліброваній системі актора, validity, trigger/squeeze,
  timestamp та відхилення даних старіших за 100 мс.
- Додано input gate актора перед UpdateInventoryOwner, правий trigger → штатні
  fire start/stop; UI/смерть/tracking loss/weapon switch скидають gate.
- g_fireParams бере напрямок aim правого Touch і поточну HUD muzzle position.
- Наявний HUD rig переміщується як rigid assembly з правим контролером.
  Це не незалежна анімація двох рук та не IK.
- Збережено eye projection для HUD-зброї й дульних частинок. Камерний shot recoil
  замінено OpenXR haptic pulse правого Touch, що викликається зі shot callback.
- C ABI tests (зокрема controls/no-session/haptic exports): PASS.
- Trigger gate tests: PASS — гістерезис, startup, tracking/UI/focus inhibition,
  weapon switch, nonfinite inputs.
- Перебудова зміненого engine: exit 0. Виправлено const GetHUDmode та назву
  матричної функції відповідно до API цієї версії X-Ray.
- Тестовий запуск відкрито. Позиція моделі, напрямок пострілів і haptics
  **ще не підтверджені користувачем у гарнітурі**.
- Результат цього запуску підтверджено користувачем:
  «Так, рух зброї, постріли та вібрація працюють».
  У журналі є парні `CoPVR Touch: fire start` / `fire stop`, рендеринг
  локації продовжується. Знімок журналу: `reports/touch-weapon-validated.log`.
  Попередня позначка про непідтверджений запуск стосується стану до цієї відповіді.
  Підтверджено базовий сценарій; це не перевірка кожного типу зброї чи проходження гри.

## Навігація променем Touch — новий тест

- Реалізовано перетин променя aim з реальною позою та розмірами OpenXR quad.
- UI pointer ABI: 32 байти, тест експорту, неактивного стану без сесії,
  несумісного ABI та null pointer пройдено.
- Тести центра, координат UV, промаху, задньої сторони, паралельного
  променя й повернутої панелі пройдено.
- TriggerGate та попередні bridge ABI тести пройдено.
- MSVC збірка рушія і повторна збірка останніх змін xrGame: exit 0.
- git diff --check і reverse apply перевірка патча пройдені.
- Launcher додає -nointro. Візуальна перевірка пропуску логотипів,
  наведення курсора та вибору пунктів Touch у гарнітурі ще очікується.
- Просторова лінія променя та незалежні руки ще відсутні.

### Результат перевірки Touch UI користувачем

Користувач підтвердив у Quest 2: «Курсор і вибір працюють, логотипи пропущені».
Підтверджено наведення та вибір у меню й пропуск початкових логотипів.
Це підтвердження не охоплює всі внутрішньоігрові вікна, перетягування,
прокручування чи тривалу ігрову сесію. Просторова лінія променя та незалежні
руки залишаються нереалізованими.

## Видимі промені та екран готовності локації

- Знайдено окремий CUISequencer game_loaded, який не отримує кліки
  звичайних UI-панелей. Для -copvr готова однокористувацька локація
  переходить далі автоматично через update_game_loaded.
- Додано просторові блакитні промені обох Touch у режимі UI-панелі.
- Перевірено геометрію beamPose: напрямок сегмента, нормаль до голови,
  ортогональність, центр, вироджений сегмент і колінеарність.
- Увімкнено assert явно в ui_ray_tests і beam_math_tests для -O2;
  обидві перевірки повторно пройдені з активними assertions.
- Bridge ABI та TriggerGate тести пройдені; MSVC xrGame збірка exit 0;
  engine patch reverse-check та diff --check пройдені.
- Візуальна перевірка променів і автоматичного продовження після
  завантаження в Quest 2 ще очікується.

### Підтвердження користувача: промені та продовження після завантаження

У Quest 2 користувач підтвердив: «Промені видно, гра після завантаження починається».
Підтверджено видимість блакитних променів Touch у меню та автоматичний
перехід після завантаження локації. Решта внутрішньоігрових вікон і
незалежні руки не охоплені цією перевіркою.

## Перша реалізація незалежних скелетних рук

- Додано аналітичний двокістковий IK для обох рук з незалежними
  цілями кистей Touch. Знято переміщення всього ригу за зброєю.
- Arm IK tests з активними assertions пройдені: досяжні/недосяжні
  й близькі цілі, довжини плеча/передпліччя, точність кисті,
  звичайний і протилежний поворот сегмента, вироджені довжини.
- Bridge ABI, TriggerGate, UI ray та beam geometry tests також пройдені.
- MSVC xrGame build exit 0; patch reverse-check та diff --check пройдені.
- Збірку запущено для перевірки; візуально незалежність рук,
  калібрування кистей, згин ліктів та регресія стрільби ще не підтверджені.
- Модель рукавів/рукавичок і анімація пальців поки штатні.

### Виправлення вибору скелетних кісток

Користувач повідомив: руки залишилися у стандартній позі, хоча зброя рухалася.
Журнал показав left=65535 right=65535 (BI_NONE) для wpn_hand_05, 42 кістки:
попередні назви bip01_l_hand/bip01_r_hand відсутні у фактичній HUD-моделі.
IK тепер використовує конфігураційні ancor_0/ancor_1 цієї моделі й батьківські
зв'язки скелета. Додано журнал resolved wrist/elbow/shoulder та періодичну
похибку позиції кисті після перерахунку кісток. Новий візуальний результат
ще не підтверджений.

### Виправлення правої руки та standby-анімацій

Журнал попереднього тесту показав: ancor_0=lead_gun, його батько bip01,
без плеча; ancor_1=l_hand з коректним ланцюжком. Підтверджено незалежну
ліву руку з похибкою журналу ~0.00007 м. Праву руку попередня версія
не обробляла. Тепер справжні l_hand/r_hand мають пріоритет над точками
кріплення предметів; позиціонування зброї також враховує справжню праву кисть.

Додано наближене стискання пальців: змішування bind-пози та анімованого
хвату від squeeze, для finger1/index — від trigger. Модель і форми bind-пози
залишаються штатними. У VR idle HUD-анімації мають нульову швидкість,
інерція/додаткове похитування вимкнені, стандартні Weapon/Missile переходи
у eBore заблоковані. Постріл, зміна зброї та reload-анімації збережені.
Першу збірку player_hud пройдено; візуальна перевірка нових змін очікується.
- Остаточна збірка Weapon/Missile/player_hud: MSVC exit 0.

## Симетрія калібрування рук відносно гарнітури

Попередню версію користувач оцінив як таку, де обидві руки рухаються,
але відступи й осьові повороти асиметричні, пальці мають залишкові проблеми.
Журнал підтвердив обидва IK wrist errors близько 0..0.00006 м; отже помилка
була у калібруванні форми/осей, а не у прив'язці позиції wrist до контролера.

- Новий COPVR_HEAD_ABI 1 (56 bytes) передає свіжу tracked head pose,
  використовуючи ту саму origin/inverseYaw систему, що й контролери.
- Видалено wrist-to-animated-weapon калібрування. Анатомічний базис долоні
  отримується з bind-геометрії wrist/index/middle/little та враховує chirality.
- Нейтральний хват симетричний: долоні всередину, великі пальці вгору;
  протилежні roll-обертання +/-90 градусів компенсують анатомію, а не
  довільні локальні осі кісток. Кисть залишається точно у своїй grip position.
- Плечі мають дзеркальні відступи +/-0.18 м, -0.20 м вниз, -0.02 м назад
  у head-centered yaw frame. Довжини сегментів усереднені між bind-руками.
- Пальці тепер мають процедурні flexion-обертання у bind-анатомічних осях,
  з окремою віссю великого пальця. Залежність від animated foregrip прибрана.
- Actor VR HUD update використовує звичайну camera_Matrix, як світовий
  VR рендер і напрямок стрільби, замість окремих desktop HUD camera effects.
- Head ABI, попередні bridge/gate/ray/beam/IK тести пройдені.
- Нові tests/hand_symmetry_tests.cpp з активними assertions пройдені:
  дзеркальні анатомічні базиси, відображення knuckles при довільних
  controller yaw/pitch/roll та mirror IK близьких, далеких і crossed targets.
- Візуальна перевірка симетрії та нових пальців у Quest 2 ще очікується.
- Остаточна MSVC збірка симетричного калібрування: exit 0; тестовий запуск відкрито.

### Підтвердження симетрії користувачем

Користувач перевірив Quest 2 і підтвердив: «Положення й обертання симетричні».
Підтверджено симетрію позиціонування й осьових обертань рук. Ця відповідь
не підтверджує окремо правильність усіх пальців, усіх моделей зброї чи
фізичної взаємодії. Фізичні перезаряджання, інвентар і колізії ще незавершені.

## Touch locomotion та кнопки UI

- Додано vector2 sticks, face/stick/menu buttons у Touch action set.
- Navigation ABI 40 bytes: export/inactive/null/wrong-ABI тести пройдені.
- NavigationGate tests з активними assertions пройдені: початковий held input,
  neutral rearm, одиничний snap, поворот в інший бік після центру,
  release при інгібіції, нечисловий ввід, button edge і focus reacquisition.
- Попередні bridge/trigger/ray/beam/arm/symmetry тести пройдені.
- MSVC xrGame build exit 0; diff --check і patch reverse-check пройдені.
- Нова збірка запущена. Рух/стрибок/спринт/присідання, UI X/B/Y,
  snap turn та scroll ще потребують hardware перевірки користувачем.

## Після підтвердження Touch locomotion

Користувач повідомив «все працює» щодо руху, поворотів та кнопок UI.
Додатково повідомив відсутність holster/use способів і прозоре двоїння
освітлення/об'єктів на Скадовську.

- Виявлено accumulator clear mark, спільний для двох очей одного dwFrame.
  Додано view-start invalidation, без скидання між окремими lights одного ока.
- Shared motion-blur reprojection вимкнено у VR, незалежно від ps_r2_mblur.
- HUDTarget picking переведено на реальну позу HMD для native use.
- Правий stick Y поза UI: holster/restore вниз, reload вгору.
- NavigationGate holster/reload edge/hold/release тести пройдені.
- Попередні bridge/trigger/ray/beam/IK/symmetry/navigation тести пройдені.
- R4 renderer вже зібрано успішно; hardware перевірка виправлень очікується.
- Остаточна повна збірка Game/R4: exit 0; тестовий запуск відкрито.

## Асиметрична проєкція в реконструкції G-buffer / SSAO

Користувач підтвердив holster/use керування, але прозоре двоїння залишилось,
сильніше зліва; при повороті HMD видно шум, що рухається проти повороту.
Попередній accumulator fix не усунув видимий симптом.

Виявлено r3_gbuffer_opt=on та r2_ssao=high. Binder pos_decompression_params
брав Device.fFOV/fASPECT, які не відповідають off-centre проєкціям OpenXR.
У VR binder тепер бере фактичні P11/P22/P31/P32 кожного ока:
xy=((1+P31)/P11,(P32-1)/P22), zw=(2/(W*P11),-2/(H*P22)).
SSAO uv_to_eye тепер використовує незалежні scale/offset параметри,
замість припущення, що offset.xy одночасно є симетричним масштабом.
Шейдер оновлено в engine/res та локальному game-vr overlay; Steam не змінено.

Projection decompression tests пройдені з активними assertions:
ліва/права/симетрична проєкції, вертикальні off-centre зсуви, різні глибини
й пікселі повертають вихідні view-space координати. Попередні тести також
пройдені. Візуальний результат на Скадовську ще очікується.

## Відновлення світла у правому оці

Користувач підтвердив: «двоїння та шуми пропали але освітлення є тільки у лівому оці».
Реконструкція G-buffer/SSAO усунула повідомлені артефакти; залишилась
асиметрія освітлення. Знімок журналу попереднього тесту збережено окремо.

Знайдено наступний per-frame cache: CLight_DB::add_light відкидав світло,
якщо L->frame_render==Device.dwFrame. Lights.Update очищає package для
кожного ока, але дві eye-calculations мають спільний dwFrame. Через це другий
package не отримував джерела, вже додані під час першого eye pass.

У VR дублікати тепер контролюються per-view set, який очищається разом із
package. Звичайний desktop frame marker збережено для решти режимів.
Спільні monocular light occlusion queries у VR не відсікають джерела;
frustum/portal selection залишається. Це може збільшити кількість намальованих
lights порівняно з occlusion-optimized desktop режимом. Додано per-view
point/spot/shadow counts у журнал для наступної перевірки.
Новий результат освітлення ще не підтверджений користувачем.
- MSVC R4 збірка per-view light selection: exit 0; diff/patch checks пройдені;
  виправлену збірку запущено для hardware перевірки.

### Підтвердження стереоосвітлення на Скадовську

Користувач підтвердив у Quest 2: «Освітлення є в обох очах, двоїння й шуму немає».
Підтверджено виправлення повідомленого артефакту на перевіреній сцені:
проєкційна реконструкція G-buffer/SSAO разом із per-view light selection
дають освітлення в обох очах без попереднього прозорого двоїння й шуму.
Це не перевірка всіх локацій, погодних умов, джерел світла чи повного проходження.

## Відокремлення UI від стереокадру

Користувач повідомив, що діалоги/інвентар/PDA переводили весь VR кадр у
екранний режим. Реалізовано окремий UI overlay mode без втрати двох очей
у світі, перехоплення HUD/cursor на прозору текстуру та окрему quad композицію.
Додано ABI export та invalid-state тест для copvr_copy_ui; попередні
bridge/trigger/ray/beam/IK/symmetry/navigation/projection тести пройдені.
Повна MSVC збірка Game/R4/UICore: exit 0.

Користувач попросив наступний запуск у високій роздільній здатності й графіці.
Запущено HighQuality: High preset, requested 2560x1440 per-eye, texture_lod 0,
aniso16, high sun/SSAO, збережено VR DOF/mblur off.
Візуальна перевірка накладання UI та фактичних параметрів ще очікується.

### Підтвердження UI overlay у High 2560x1440

Користувач підтвердив у Quest 2: «Інтерфейс поверх стереосвіту працює правильно».
Підтверджено результат останнього тесту класичних вікон поверх VR-сцени.
Журнал запуску підтвердив фактичну роздільну здатність 2560x1440 per eye;
використано HighQuality launcher. Це не вимірювання FPS/затримки чи
перевірка всіх можливих вікон, станів інвентарю й діалогів.

## Підказки взаємодії та перший support grip

Користувач повідомив про залишкове двоїння world interaction/NPC підказок
та попросив вимкнути їх і продовжити роботу.
Знайдено й пригнічено quick-action text, pickup icon та PickupInfoDraw у VR.

Додано дворучний хват із спільним HUD/fire direction, hysteresis, rearm,
tracking/menu/weapon inhibition та лівою haptic-підказкою захоплення.
Two-hand tests з активними assertions пройдені: startup squeeze, acquire,
steering while held, release, miss behind/right-ray, release-required retry,
tracking loss/reacquisition та excessive separation. Попередні
bridge/trigger/ray/beam/arm/symmetry/navigation/projection тести пройдені.
Візуальна перевірка підказок і support grip ще очікується.
- MSVC xrGame збірка: exit 0; diff/patch checks пройдені.
  Новий HighQuality запуск відкрито для hardware перевірки.

### Корекція фіксації лівої кисті й осі дворучного хвату

Користувач підтвердив зникнення підказок і роботу support grip, але повідомив
про вільне обертання лівої кисті та зсунуту вісь зброї при дворучному хваті.

Замість прирівнювання barrel forward до wrist-to-wrist vector тепер
використовуються дві реальні wrist attachment точки в локальних координатах
поточної HUD-зброї. Shortest-arc correction повертає цей off-axis grip vector
до вектора контролерів, а translation зберігає праву основну точку хвату.
Ліва кисть отримує повну weapon-relative матрицю штатної цівки, включно з
обертанням, і більше не повторює незалежний controller roll, поки grip активний.
При відпусканні повертається незалежне симетричне tracked-hand калібрування.
Профіль фіксується на час хвату; при зміні зброї/меню/втраті tracking скидається.
Якщо зайнятий HUD slot 1 (наприклад, детектор), підтримка не активується.

Two-anchor tests з активними assertions пройдені: off-axis local grips,
повернута/перенесена зброя, точність обох точок при сумісній дистанції,
збереження primary grip і довжини при іншій дистанції, вироджена пара.
Попередні bridge/gate/ray/beam/IK/symmetry/navigation/projection/support тести
пройдені. Новий візуальний результат ще очікується.
- Остаточна MSVC xrGame збірка корекції pivot/locked wrist: exit 0;
  HighQuality тестовий запуск відкрито.

### Регресія обертання і уточнення вимоги

Користувач повідомив, що після останньої зміни зброя перевертається при хваті.
Після уточнення підтвердив: «Так, зберегти попередній напрямок і зафіксувати кисть».

Прибрано two-anchor correction із runtime та його невикористаний модуль/тест.
Відновлено раніше підтверджений напрямок зброї від правого до лівого Touch
із roll від правого Touch. Фіксація лівої кисті залишається окремою:
weapon-relative foregrip matrix обчислюється через остаточну позу зброї,
а не через додатковий поворот моделі. Відпускання повертає raw left tracking.
Опис two-anchor експерименту вище є історією невдалої перевірки, не поточною
реалізацією. Повторний візуальний результат ще очікується.
- MSVC збірка відновленого support aim + locked offhand: exit 0;
  HighQuality запуск відкрито для повторної перевірки.

### 2026-10-22 — продовження без тестів

Користувач підтвердив: попередня фіксація кисті й дворучне прицілювання
працюють; попросив звузити зону захоплення цівки та розвивати фізичні
взаємодії. Окрема вказівка: «продовжуй поки без тестів».

Додано вузьку зону цівки, передачу активної зброї між Touch, захоплення
наявних світових фізичних предметів, огляд/передачу/відпускання з лінійною
швидкістю, штатний підбір утримуваного предмета та умовне native Eat після
підтвердженого підбору витратного предмета біля рота. Звільнення фізичних
оболонок виконується також при net_Destroy актора. Перелік обмежень і жестів
записаний у physical-interaction.md.

Виконано лише компіляцію MSVC xrGame. Автоматичні тести, гра й VR-сцена
не запускалися. Цей запис не означає перевірку нових взаємодій у гарнітурі.

### 2026-10-22 — зони спорядження та вільна рука

Без запуску тестів додано зони прибирання/діставання зброї зі штатних
INV_SLOT_2/INV_SLOT_3, жест підбору утримуваного предмета біля кишені/плеча,
детектор біля лівої частини грудей і перемикання ліхтарика біля скроні.
Нативний USE переведено з навігаційного біта COPVR_USE на окремий TriggerGate
вільної руки; при зброї зліва взаємодія переходить на правий тригер.
Власник зброї й новий USE вимагають відпускання тригера після перемикання.
Зони є наближенням тулуба за HMD, не окремим відстеженням тіла.
Скидання стану жестів додано при net_Destroy. Жести й обмеження описані
в physical-interaction.md. Гра та тести не запускалися.

Фінальна компіляція цього етапу: MSVC xrGame exit 0. `git diff --check`
без зауважень; patches/openxray-vr.patch оновлено. Ігровий результат не
перевірений і не позначається як підтверджений користувачем.

### 2026-10-22 — робота до резерву 5% п'ятигодинного ліміту

Початковий залишок ліміту — 28%. Умова «без тестів» збережена.
У код додано:

- Активацію піднятої світової зброї лише після підтвердженого переходу у
  власність актора та наявності сумісного слота.
- Кутову швидкість кидка, відсікання tracking discontinuities й звільнення
  предметів до експорту сейву; окремий стан формату сейву не додається.
- Native bolt projectile із tracked release та зони діставання болта/бінокля.
- Витягування конкретного предмета з власного класичного інвентарю через
  промінь+grip, native drop event і очікування фізичної оболонки.
- Піднесення сумісної коробки набоїв до reload port, перевірку типу та native
  reload. Повне ручне перезаряджання з фізичними магазинами не реалізовано.
- Допоміжну кисть для обох основних рук через анатомічне перетворення осей,
  детектор у протилежній руці, наближений процедурний хват пальців.
- Жест руків'я затвора з haptics і native reload; фізичне зміщення його
  кістки та власний стан патронника поки відсутні.
- Навмисне відпускання активної зброї поза кобурою через native ownership
  reject, з передаванням пози та швидкостей світовій оболонці.

Bridge зібрано через новий -BuildOnly: ABI структур не змінено, додано лише
optional export copvr_get_ui_controls. Кілька проміжних MSVC xrGame збірок
пройшли. Нові механіки не запускалися в грі й не підтверджені в Quest 2.
Керування, обмеження та параметри калібрування — у physical-interaction.md.

Фінальний xrGame після фізичного відпускання зброї: MSVC exit 0;
copvr_bridge.dll: Zig -BuildOnly exit 0. git diff --check та reverse-apply
check збереженого engine patch — exit 0. Steam-каталог не змінювався.
Збережено локальний checkpoint джерел/patch/DLL у build/checkpoints.
Підтверджено лише компіляцію та цілісність патча, не ігрову поведінку.

### Зворотний зв'язок після запуску фізичної взаємодії

Користувач підтвердив передачу зброї, стрільбу та дворучний хват із лівою
основною рукою; піднімання/огляд/передачу/кидання світових предметів;
витягування з інвентарю; викидання зброї та уточнену форму пальців.
Зауваження: цівку взагалі не вдається захопити після звуження зони,
кидок слабкий, витягнуті предмети неприродно лежать у кисті, рукоятка
активної зброї не в долоні. Болт дістати не вдалося. Зони спорядження,
споживання та reload не перевірені; кнопкове прибирання/діставання працює.

У наступному виправленні прибрано додатковий generic cone цівки, який
відсікав коротку зброю; залишено область 7 см біля модельної точки долоні
цівки. Native HUD grip і tracked grip вирівнюються за центрами долонь,
а не зап'ястями. UI extraction отримує вкладення на стороні долоні та
окреме наближення рукоятки світової зброї (override copvr_world_grip).
Кидок використовує історію 80 мс, коефіцієнт 1,25 і межу 12 м/с;
tracking discontinuities очищають також історію. Болт/бінокль тепер
перемикаються зі зброї через звичайний Activate, з відкладеним вибором
руки після завершення native перемикання. Виправлення потребують
повторного підтвердження; автоматичні тести не запускалися.

### Смерть, завантаження та напрямок VR-камери

Користувач повідомив про подвоєний текст смерті, виліт при завантаженні
після смерті, зсув камери після діалогової анімації та рух відносно тіла,
а не погляду. Перед збіркою користувач підтвердив закриття гри.

Зміни: death UI включає single overlay; CUISequencer::OnRender (окремий
рендер tutorial game_over) теж проходить UI capture й пропускає друге око.
Додано guard порожнього sequencer. CActorCameraManager виконує ProcessCam
заради lifecycle/callbacks, але у -copvr відновлює camera info; scripted
first-eye look-at не змінює yaw/pitch. Прискорення актора поза драбинами
переводиться у світ через горизонтальний HMD forward і right, без повороту
фізичного тіла під гарнітуру.

У CLevel::net_Stop виправлено конкретне неправильне розіменування:
перевірка g_tutorial2 викликала Persistent() через g_tutorial, який попередній
Stop міг уже обнулити. Збережені snapshot dumps не мають exception stream,
а старий текстовий стек порожній; отже, причинний зв'язок саме цього дефекту
з повідомленим вильотом ще потребує повторної перевірки. Додано компактний
exception code/address та module-relative stack-top caller у xrDebug,
щоб наступний збій не залишився без даних при відмові SymInitialize.
tools/inspect-crash.py читає лише dump exception/modules/stack candidates,
без виведення секції environment з BugTrap-звіту. Автоматичні тести не
запускалися; перевірка поведінки залишається за повторним VR-запуском.


### Повторна перевірка 22 жовтня: діалоги, болт і меню

Користувач підтвердив текст смерті без подвоєння та рух за HMD. Камера
після допомоги пораненому й виліт при reload залишилися. Правий хват
підтверджено; лівий не збігався з рукояткою; болт діставався без кидка.

Exception 0xC0000005 за адресою 0x7ffd637438d7 зі snapshot 105151
зіставлено з базою xrGame 0x7ffd631e0000: offset 0x5638d7. Matching PDB
вказує MainMenu.cpp:307 — звернення до m_startDialog після factory cast.
DLL/PDB збережені в build/crash-105151 до перебудови. VR Activate тепер
повторно показує існуюче вікно; явний ReloadUI перевіряє тип нового вікна
до видалення попереднього й обробляє невдалий cast. Це виправлення
потребує підтвердження саме при завантаженні після смерті.

UITalkWnd::UpdateCameraDirection напряму змінював yaw/pitch базової
камери, обходячи effector manager. У VR цей автоповорот вимкнено.
Лівий основний хват отримав перетворення авторської правої кисті через
анатомічні базиси та суміщення центрів долонь. Болт створює native
projectile, який раніше створювався тільки через eThrowStart; момент
відпускання, поза й швидкість зберігаються до готовності (до 2 секунд).

Зони спорядження збільшені, мають haptic входу й draw після 150 мс
у зоні з уже затиснутим grip. Можна перемикати збройові слоти без
попереднього приховування кнопкою. Нові зміни ще не підтверджені у HMD;
автоматичні тести не запускалися.


### Пояс і явний хват: реалізація без запуску

Узгоджено з користувачем: утримання grip; новий натиск лише вільною
рукою; два однозначні збройові кріплення за слотом і типом, видимий пояс,
окремий proxy перезарядки, хотбар, chest-use і release-to-pickup.
Додані game_vr_belt_render.hpp, game_vr_collision.hpp, game_vr_ui_input.hpp.
Справжні коробки більше не запускають reload, тип вибирається UI drag/drop.
CWeaponMagazined/Shotgun не вибирають інший тип автоматично у VR.
UICellItem перевіряє virtual mouse-held, drag sprite проходить UI capture,
bridge утримує власника променя під час drag. Старий stick holster/reload
прибрано. Докладна поточна схема у physical-interaction.md.

Новий crash LuaJIT+0x15016 зі snapshot 111220 зіставлено з lj_api.c:763.
Збережені matching DLL/PDB у build/crash-111220. Стек-кандидати з memory
stream містять UIScriptWnd::NeedCursor -> DialogHolder -> MainMenu::OnFrame.
Reset callback меню раніше викликав ReloadUI перед знищенням Lua VM.
Тепер він знищує/очищає вікна до reset, а відбудова відкладена до OnFrame.
CleanInternals очищає також відкладений список render dialogs.
Додано copvr-exception.dmp з оригінальним exception context для наступного
збою. Причинний зв'язок і відсутність вильоту ще треба підтвердити.

Користувач прямо заборонив запуск тесту без підтвердження. Гра і тести
не запускалися; виконуються лише компіляція та статична перевірка патча.


### Зворотний зв'язок першого тесту пояса

Користувач підтвердив роботу proxy набоїв, але повідомив про обертання
пояса/плечей за головою, захоплення зап'ястям, малі зони, слабкі haptics,
перевернуту ліву кисть і відсутність видимих use/stow зон. Гру закрито
за підтвердженням користувача. Внесені зміни описані в останньому
розділі physical-interaction.md. Перекомпіляція без нового VR-запуску;
результат поведінки ще не підтверджений. Два незалежні активні стволи
залишаються обмеженням native системи однієї активної зброї.


### Експериментальна модель тіла

Після відгуку «вже краще» користувач попросив спробувати показ моделі
персонажа й відклав контури, яких не було видно. Реалізовано visual-only
clone з прихованими head/neck/upperarm та копіюванням native поз видимих
кісток. Зони влучання й оригінальна видимість кісток не змінюються.
Native weapon attachments вимкнені лише для локального VR body render.
Очищення при Die, OnChangeVisual, net_Destroy та destructor.
Контури вимкнено. Компіляція без запуску; VR-перевірки ще не було.


### Відмова від тіла та сітка слотів

Користувач попросив прибрати тіло й зосередитися на зручності слотів;
закриття гри підтверджене. Експериментальний actor clone повністю
видалено, native first-person visibility відновлена. Слоти розведені
у два рівні ряди, моделі збільшено, кріплення зброї наближені.
Вибір — normalized reach box від долоні, один найближчий кандидат,
12% hysteresis та окремі per-hand cooldown. Подробиці в кінці
physical-interaction.md. Гру й автоматичні тести не запускали.

### Equipment alpha 1 — 2026-10-22

Added a backward-compatible grip orientation export and same-frame validation;
tracked palms and native weapon wrist attachments now share a contact point.
Removed the universal left-hand PI correction. Right primary fingers preserve
weapon-authored animation; left fingers remain procedural pending authored poses.
Natural belt item scale, no hover growth; configurable anchors and per-item grip
corrections in copvr_equipment.ltx. Equipment-fit script verified for standing,
seated offset, width and restoring defaults. Defaults staged in game-vr only.

Bridge regression suite passed: ABI/export rejection (including new extension),
trigger gates, UI rays, beams, arm IK, symmetry, navigation gates, projection
reconstruction and two-hand grip. xrGame Release x64 compiled successfully.
Engine patch diff check and reverse-apply check passed. No game or headset
session launched. In-headset grip quality and layout remain unverified.
See equipment-alpha-1.md for exact scope and known limitations.

### Alpha 1a barrel orientation regression
User screenshots disproved alpha 1 weapon orientation. Restored validated aim barrel orientation while retaining grip palm contact and attached wrist. weapon_contact_tests, existing hand symmetry and two-hand tests passed without runtime/game launch. Visual fix remains unconfirmed in Quest.


### Alpha 1b hand poses
User confirmed automatic rifle direction, reported free-hand orientation, pistol contact and support-hand appearance defects. Restored free-hand aim basis, added proper mirrored wrist frames and authored finger retargeting. Hand symmetry (extended reflection tests), weapon contact and two-hand tests passed. No headset/game launch; visual result unconfirmed.


### Alpha 1c hands only
Direct wrist tracking replaces arm IK. Sleeve skinning palette collapses to wrist while attachment bones remain intact. Per-frame current-item wrist poses replace stale firearm cache for bolts; support state clears on non-weapons. Knife/binocular support grip disabled, secondary wrist source corrected. Contact/symmetry/two-hand regressions passed. No game launch. Left grip and dedicated item poses remain visually unconfirmed after earlier failures.


### Alpha 1d mirrored contact translation
User confirmed free hands, isolated left primary/opposite support dorsal contact and tool wrist orientation issues. Full mirrored contact position replaces copied source position and forced shared palm centre. Support acquisition uses the actual offhand contact. Knife/binocular/bolt now solve object pose around the confirmed free wrist frame. Extended hand contact reflection, weapon contact and support tests passed. No game launch; visual result pending.


### Alpha 1e world handle equip / detector
User confirmed all alpha 1d grips correct. Added authored world-handle grip detection, ownership-confirmed native equip in grabbing hand, long-object regrip, inactive-slot storage fallback and pending-request guards. Detector now uses authored source-hand fingers and mirrored secondary attachment where required. Existing contact/symmetry/support regressions passed. No game launch; feature integration needs VR confirmation.


### Alpha 1f grenades and manual reload
User confirmed alpha 1e interactions. Added grenade belt slot, native tracked throw/consumption/fuse path, fresh-trigger and offhand pin-pull priming, unprimed return, inert world/inventory grenade pickup. Disabled actor VR automatic empty-mag reload and underbarrel automatic reload paths. Grenade primer tests and existing contact/symmetry/support regressions pass. Native projectile/count/fuse and all weapon classes still require authorized gameplay verification. No game launched. See grenades-and-manual-reload.md.


### Test graphics preference
Future VR tests default to High at 2560x1440 per eye, with maximum supported draw distances: rs_vis_distance=1.5, geometry_lod=2, detail_radius=300, dtex_range=175, sun_far=180, wet_surfaces_far=100. Explicit MaxQuality remains available. Preference saved in launcher; running game not restarted.

