# Reliability Audit — VideoReader (Simple Image Player)

Scope: find and fix low-risk, local reliability defects without changing intended
behavior or public interfaces. Every finding below was reproduced in this
environment; the evidence column says how.

Environment used: Ubuntu 24.04, GCC 13.3, CMake 3.28.3, Qt 6.4.2 and OpenCV 4.6.0
(installed from Ubuntu's `qt6-base-dev` and `libopencv-dev`; neither was present
initially). Line numbers in findings refer to the code **before** this change
(commit `5edd456`).

---

## 1. Repository overview

### Structure

| Path | Role |
|---|---|
| `src/core/main.cpp` | Entry point: creates `QApplication` and `MainWindow` |
| `headers/ui/mainwindow.h`, `src/ui/mainwindow.cpp` | Coordinator: owns the three components and wires their signals |
| `headers/ui/ui_controller.h`, `src/ui/ui_controller.cpp` | Builds widgets (image label, FPS spin box, Load/Play buttons), re-emits user actions as signals |
| `headers/core/processors/image_processor.h`, `src/.../image_processor.cpp` | Recursively collects `*.jpg/jpeg/png/bmp` paths from a folder; decodes one frame on demand with OpenCV and converts to `QImage` |
| `headers/core/controllers/playback_controller.h`, `src/.../playback_controller.cpp` | Playback state and timing; runs a `std::thread` that advances the frame index and emits `frameChanged` |
| `CMakeLists.txt`, `src/CMakeLists.txt` | CMake build: static libs `controllers`, `processors`, `ui`, executable `SimpleImagePlayer` |
| `build_scripts/build_simple_moc.sh` | Alternative build: runs `moc` by hand, then one `g++` invocation |
| `tests/` | **New.** Qt Test unit tests (see §4) |
| `docs/` | Architecture notes (partly stale, see §3) |

### Runtime flow

```
UIController (button / spin box) ──signal──▶ MainWindow ──▶ ImageProcessor::loadImagesFromFolder
                                                       └──▶ PlaybackController::start/pause/setFPS
PlaybackController thread ──frameChanged (queued)──▶ MainWindow::onFrameChanged
                                                       └──▶ ImageProcessor::getImage ──▶ UIController::updateImage
```

`frameChanged` is emitted from the playback thread; because `MainWindow` lives on
the GUI thread, Qt's `AutoConnection` delivers it as a queued event, so decoding
and painting happen on the GUI thread.

### Build and run

Requirements: C++17 compiler, CMake ≥ 3.16, Qt 6 (Core, Gui, Widgets; Test for
the tests), OpenCV 4. On Ubuntu 24.04:

```bash
sudo apt-get install qt6-base-dev libopencv-dev cmake g++
```

**CMake (recommended):**

```bash
cmake -S . -B build
cmake --build build -j
./build/src/SimpleImagePlayer          # note: binary is under build/src/
ctest --test-dir build --output-on-failure
```

Pass `-DVIDEOREADER_BUILD_TESTS=OFF` to skip the tests.

**Shell script** (assumes Debian/Ubuntu x86_64 Qt include paths):

```bash
./build_scripts/build_simple_moc.sh
./build_simple/SimpleImagePlayer
```

**What I could and could not run here:**

- Both builds: built successfully after the fixes. Before the fixes the CMake
  build **failed** (Finding 4); the shell script built.
- Unit tests: ran, output in §4.
- The GUI app: **not run interactively** — there is no display in this
  environment. I drove the real `MainWindow` headlessly with
  `QT_QPA_PLATFORM=offscreen` to reproduce Finding 1, but I did not click
  through the UI or check rendering by eye.
- Not tested: Windows, macOS, other Qt/OpenCV versions, non-x86_64.

---

## 2. Top 5 highest-risk issues

Ranking criterion: confirmed impact × how easily a normal user reaches it.

### Finding 1 — Use-after-free when closing the window after pressing Play — **FIXED**

- **Where:** `src/core/controllers/playback_controller.cpp:15-18` (destructor
  calls `stop()`), `:60-61` (`stop()` emits `playbackStopped` and
  `frameChanged(0)`); the freed object is used in `src/ui/mainwindow.cpp:82`.
- **Why it's a problem:** `QWidget::~QWidget` deletes **all** children
  (including plain `QObject`s) *before* `QObject::~QObject` disconnects
  signals. `MainWindow`'s children are deleted in creation order:
  `ImageProcessor` first, then `PlaybackController`. The controller's destructor
  calls `stop()`, which emits `frameChanged(0)`. That reaches
  `MainWindow::onFrameChanged` on a half-destroyed window, which calls
  `imageProcessor->getImage(0)` on the `ImageProcessor` that was freed a moment
  earlier.
- **Evidence:** I drove the real `MainWindow` (load folder → start → `delete`).
  - In a debug Qt build, Qt aborts: *"ASSERT failure in MainWindow: Called
    object is not of the correct type (class destructor may have already run)"*.
  - In a release-style build under AddressSanitizer: `heap-use-after-free`
    in `ImageProcessor::isValidIndex` ← `getImage` ← `MainWindow::onFrameChanged`
    ← `PlaybackController::stop` ← `~PlaybackController` ← `QWidget::~QWidget`.
  - After the fix, the same probe exits cleanly with no ASan report.
- **Fix:** I added a private `joinPlaybackThread()` that stops and joins the
  thread **without emitting**. The destructor calls it; `stop()` also uses it and
  then emits as before. Public `stop()` behavior is unchanged.

### Finding 2 — Data race on shared playback state (undefined behavior) — **FIXED**

- **Where:** `headers/core/controllers/playback_controller.h:38-40` (`int fps,
  currentFrame, frameCount`). Written or read without synchronization by the GUI
  thread (`setFPS` `:23`, `getCurrentFrame` `:94`, `setFrameCount`,
  `setCurrentFrame`) and by the playback thread (`runLoop` `:106-114`).
- **Why it's a problem:** concurrent non-atomic access is undefined behavior
  in C++. In practice the compiler may cache or tear values. `fps` is used as a
  divisor (`1000 / fps`), so a bad read can mean a division by zero.
- **Evidence:** ThreadSanitizer reports races at
  `playback_controller.cpp:23` (`setFPS`) and `:94` (`getCurrentFrame`) against
  the write at `:107`. After the fix the full controller suite runs under TSan
  with **0 warnings** (§4).
- **Fix:** `fps`, `currentFrame` and `frameCount` are now `std::atomic<int>`.
  All accesses use `load`/`store`, and `runLoop` reads each value once per tick.
  These are private members, so the public interface is unchanged.
- **Limitation (not changed):** atomics remove the UB but not the logical
  interleaving. A `setCurrentFrame()` call made during playback can still be
  overwritten by the next tick, exactly as before.

### Finding 3 — Zero-millisecond sleep above 1000 fps: CPU busy-spin and event flood — **FIXED (partially)**

- **Where:** `src/core/controllers/playback_controller.cpp:114`
  (`milliseconds(1000 / fps)`).
- **Why it's a problem:** integer division gives `0` for any `fps > 1000`.
  The loop then never sleeps: it pegs one core and posts a queued
  `frameChanged` event to the GUI thread on every iteration, and the GUI cannot
  drain them.
- **Reachability:** the UI spin box is capped at 1000, so **this is reachable
  only through the public `setFPS()` API**, which accepts any positive value.
- **Evidence:** at `setFPS(5000)` the original code advanced
  **351,272 – 2,022,747 frames in 200 ms** across runs. The fixed code
  advanced 179, 179 and 181 frames in three runs. The test
  `highFpsDoesNotBusySpin` asserts a loose `< 2000` bound so it stays stable on
  slow CI machines.
- **Fix:** the per-frame interval is now floored at 1 ms. Behavior at
  1–1000 fps (everything the UI allows) is unchanged.
- **Not fixed:** see Additional finding C. The GUI thread can still fall
  behind at legal rates.

### Finding 4 — The documented CMake build does not produce a binary — **FIXED**

The README calls this the recommended build. It failed for three independent
reasons; each surfaced only after the previous one was fixed.

| # | Where | Cause | Evidence |
|---|---|---|---|
| 4a | `src/CMakeLists.txt:17-24` | `processors` links only `Qt6::Core`, but `image_processor.h` includes `<QImage>` (Qt Gui). No target adds `${OpenCV_INCLUDE_DIRS}`, and Ubuntu installs OpenCV headers under `/usr/include/opencv4`. | `fatal error: QImage: No such file or directory` |
| 4b | `src/CMakeLists.txt:1-3, 13-15, 26-29` | Q_OBJECT headers live in `headers/`, not next to their `.cpp`, and are not listed as target sources, so AUTOMOC can't find them and generates no meta-object code. | `mocs_compilation.cpp`: *"No files found that require moc"*. The link would fail on missing `staticMetaObject` and vtables. |
| 4c | `CMakeLists.txt:13` | `CMAKE_AUTOUIC ON` treats any `#include "ui_*.h"` as a Designer-generated form. The hand-written `ui_controller.h` is taken as the form for a nonexistent `controller.ui`. The repo has **zero** `.ui` files. | `AutoUic error: … includes the uic file "ui/ui_controller.h", but the user interface file "controller.ui" could not be found` |

- **Fix:** I added `Qt6::Gui` and `${OpenCV_INCLUDE_DIRS}` to `processors`,
  listed the Q_OBJECT headers as target sources, and set `CMAKE_AUTOUIC OFF`.
  With all three changes, the clean CMake build succeeds for all targets.

### Finding 5 — Build script reports success after `moc` fails and links stale output — **FIXED**

- **Where:** `build_scripts/build_simple_moc.sh:16-24`.
- **Why it's a problem:** the script runs four `moc` commands, then checks
  `$?` once, which only covers the **last** one. If any earlier `moc` fails,
  the script prints "MOC successful" and compiles whatever stale
  `build_simple/moc_*.cpp` is left from a previous run. The result is a binary
  whose meta-object code doesn't match the headers.
- **Evidence:** I pointed the first `moc` input at a missing file:
  - original script: `moc: …: No such file` → `MOC successful, generated MOC
    files` → **exit 0** (the stale `moc_mainwindow.cpp` from the previous run
    was linked);
  - fixed script: `MOC failed on headers/ui/DOES_NOT_EXIST.h!` → **exit 1**.
- **Fix:** a small `run_moc` helper checks every invocation. I also quoted
  `"$MOC_PATH"`. The normal build output is unchanged.

---

## 3. Additional findings

| ID | Where | Issue | Status |
|---|---|---|---|
| A | `.gitignore:36` (`ui_*.h`) | Silently ignores the real source header `headers/ui/ui_controller.h` (`git check-ignore -v --no-index` confirms). This is why commit `d6652d3` *"Add missing ui_controller.h header file"* was needed. Any new `ui_*.h` would be dropped the same way. | **Fixed** — removed the pattern (no `.ui` files exist, so it guarded nothing). |
| B | `src/ui/ui_controller.cpp:20-53` | `setupUI(nullptr)` leaves the layout unparented, so every widget and layout it creates leaks. A second call orphans the first widget set and triggers Qt's *"already has a layout"* warning. `MainWindow` calls it once with a valid widget, so the app itself doesn't hit this; only misuse of the public API does. | **Fixed** — early return on null or on repeat. LeakSanitizer: original leaks objects allocated at lines 22, 29, 38, 41, 47 and 50; fixed: none. |
| C | `playback_controller.cpp:111` + `mainwindow.cpp:80-84` | No backpressure. The playback thread posts `frameChanged` at the requested rate regardless of whether the GUI has finished decoding and painting the last frame. If disk read + decode + `SmoothTransformation` scaling takes longer than 1/fps, the event queue grows without bound (memory growth, UI lag after pausing). *Reasoned from the code, not measured.* | **Not fixed** — needs a design change (frame dropping or a "frame consumed" handshake), which changes playback behavior. |
| D | `image_processor.cpp:53-64`, `ui_controller.cpp:57` | `getImage` swallows every exception (including `std::bad_alloc`) with `catch (...)`, and a decode failure returns a null `QImage`. `updateImage` then silently keeps the previous frame, so a corrupt file shows a stale image with no error. Pinned by `unreadableFileYieldsNullImage` and `updateImageIgnoresNullImage`. | **Not fixed** — reporting it needs a signal from a `const` method or a changed return type, i.e. a public interface change. |
| E | `image_processor.cpp:41` | `imagePaths.sort()` is lexicographic on full paths, so `frame10.png` sorts before `frame9.png` and unpadded sequences play out of order. | **Not fixed** — changing the order changes behavior. A natural sort (`QCollator::setNumericMode(true)`) is the likely fix if wanted. |
| F | `docs/README.md`, `docs/LOW_COUPLING_ARCHITECTURE.md` | Stale. They say FPS is 1–60 (it's 1–1000), that `PlaybackController` depends on a Qt timer (it uses `std::thread`), and give `./build_simple_moc.sh` (actual path: `build_scripts/`). The top-level README gives the run path `./build_simple/...` even under its CMake heading. | **Not fixed** — docs only, out of scope. Correct commands are in §1. |

---

## 4. Tests

The new `tests/` directory uses **Qt Test**. It ships in the same
`qt6-base-dev` package the project already requires, so no new third-party
dependency was added. Tests use only the public API; the UI tests look up
child widgets with `findChildren`.

| Test binary | Cases | Covers |
|---|---|---|
| `tst_playback_controller` | 12 | FPS validation, frame-count and frame-index bounds, start/pause/stop transitions and signals; regressions for Findings 1, 2 and 3 |
| `tst_image_processor` | 10 | Missing or empty folder errors, extension filtering, recursion, index bounds, BGR→RGB correctness, corrupt file, `clear`, reload |
| `tst_ui_controller` | 9 | Widget tree, Finding B (null and repeat `setupUI`), button state, FPS signal, error label, null-image handling |

### Actual output (fixed code, `ctest` + per-binary)

```
Test project .../b1
    Start 1: tst_playback_controller
1/3 Test #1: tst_playback_controller ..........   Passed    0.65 sec
    Start 2: tst_image_processor
2/3 Test #2: tst_image_processor ..............   Passed    0.09 sec
    Start 3: tst_ui_controller
3/3 Test #3: tst_ui_controller ................   Passed    0.08 sec

100% tests passed, 0 tests failed out of 3
```

```
Totals: 14 passed, 0 failed, 0 skipped, 0 blacklisted   (TestPlaybackController, incl. init/cleanup)
Totals: 12 passed, 0 failed, 0 skipped, 0 blacklisted   (TestImageProcessor)
Totals: 11 passed, 0 failed, 0 skipped, 0 blacklisted   (TestUIController)
```

One expected stderr line, `libpng error: [20]rea: invalid chunk type`, comes
from OpenCV inside `unreadableFileYieldsNullImage`. That test feeds it a
corrupt PNG on purpose.

### The regression tests fail on the original code

I built the same tests against the pre-fix sources to confirm they detect the
bugs and don't just pass:

| Test | Original code | Fixed code |
|---|---|---|
| `destructorEmitsNoSignals` (F1) | **FAIL** — `owner.signalsAfterTeardown`: 2, expected 0 | PASS |
| `highFpsDoesNotBusySpin` (F3) | **FAIL** — advanced 613,457 frames in 200 ms | PASS |
| `concurrentSetFpsWhileRunningIsSafe` (F2), under `-fsanitize=thread` | 7 TSan warnings (incl. `setFPS:23`, `getCurrentFrame:94`) | **0 warnings**, 14/14 pass |
| `setupUiIsIdempotent` (B) | **FAIL** — Qt warns *"already has a layout"*; first button orphaned | PASS |
| `setupUiWithNullParentCreatesNothing` (B) | Passes without a sanitizer; the leak shows up under LSan | PASS, no leak |

Testing caveat: Qt's `QSignalSpy` connects with `DirectConnection`, so spying
on `frameChanged` while the playback thread runs is itself a data race. TSan
caught this in my first draft of the tests. The tests now record cross-thread
frames through a `Qt::QueuedConnection` receiver (`FrameRecorder`).

---

## 5. Behavior changes, stated explicitly

The goal was no behavior change. These are the observable differences, all
confined to defect paths:

1. `~PlaybackController` no longer emits `playbackStopped` / `frameChanged(0)`
   and no longer resets `currentFrame` before the object is destroyed. Those
   emissions were what caused Finding 1.
2. `setFPS(n)` with `n > 1000` now plays at about 1000 fps instead of
   busy-spinning. 1–1000 fps is unchanged.
3. `UIController::setupUI(nullptr)` and any second `setupUI` call are no-ops.
4. `ui_*.h` files are no longer git-ignored.
5. Build only: `CMAKE_AUTOUIC` is off (no `.ui` files exist), and tests build
   by default (`VIDEOREADER_BUILD_TESTS`).

No public method signatures changed. The only header edit is in
`PlaybackController`'s `private:` section (atomic member types plus one private
helper).
