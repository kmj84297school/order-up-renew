# DECISIONS

Technical decisions and why they were made. Do not redesign a system listed
here without a reason that contradicts the rationale recorded with it.

---

## D-01 — Target Unreal Engine 5.4

**Decision.** `EngineAssociation` is `"5.4"`.

**Rationale.** The project was created in an environment with no Unreal
installation, so the version could not be read off the machine. 5.4 is a
widely deployed, stable release with mature Lumen, Nanite and Enhanced Input.

**Limitation.** This is a guess about the user's machine, not a measurement.
Changing it costs three lines (the `.uproject` field and
`IncludeOrderVersion` in both Target.cs files). Confirm with the user.

---

## D-02 — Camera behaviour lives in camera modes, not on the Character

**Decision.** `UFarmCameraComponent` owns a `UFarmCameraModeStack` of
`UFarmCameraMode` objects. The component overrides `GetCameraView` and asks
the stack where the view is. `AFarmCharacter` holds the component and
nothing else camera-related.

**Rationale.** The brief requires four eventual camera modes (Free Walk,
Cinematic Tour, Bench View, Photo Mode) and says explicitly not to couple
camera behaviour to one Character. With a mode stack, each new mode is one
subclass and one `PushCameraMode` call. The alternative — branching inside
the Character — turns into a tangle by the third mode.

The shape follows Lyra's camera mode stack, which is a proven solution to
this exact problem, simplified to what this project needs.

**Consequences.** Modes are `UObject`s outered to the camera component; they
must not assume they are ticked by the actor. Blending is bottom-to-top by
`BlendWeight`, and modes below a fully-blended mode are dropped.

---

## D-03 — The project runs with zero content assets

**Decision.** Nothing in C++ hard-requires an authored `.uasset`:

- No input config assigned → `UFarmInputConfig::CreateRuntimeConfig()`
  builds `UInputAction`s and a `UInputMappingContext` in C++.
- Level has no ground or lights → `AFarmGameMode` spawns
  `AFarmWorldBootstrap`.
- Level has no `PlayerStart` → the GameMode spawns a transient one.

**Rationale.** Phases 1–2 were developed without an Unreal Editor, so no
binary asset could be authored. Rather than leave the project unrunnable
until someone opens the editor, the defaults live in code.

This also turns out to be good practice independently: the module is
reviewable and diffable in full, and a broken or missing asset degrades to a
working default instead of a black screen.

**Consequences.** Each fallback logs a warning and says it is a placeholder.
Each switches itself off when the real thing exists, so later phases do not
have to remove them — Phase 3's level simply contains its own ground and
`PlayerStart`, and the fallbacks stop firing.

---

## D-04 — Mouse look is not smoothed; the Y negation lives in the mapping

**Decision.** `AFarmCharacter::Look` applies sensitivity and an optional
invert, and nothing else. No interpolation, no acceleration. Engine mouse
smoothing and view acceleration are off in `DefaultEngine.ini`.

**Rationale.** The brief asks for Free Walk to be "comfortable and
responsive". Look smoothing buys visual smoothness by adding input latency,
which reads as sluggish and is the more common comfort complaint of the two.
Movement gets its smoothness from acceleration/braking on the movement
component, where it does not cost responsiveness.

**Separately:** the Y axis is negated by a `UInputModifierNegate` in the
mapping context, and `Look()` passes `Value.Y` straight through. This matches
the convention in the stock Unreal templates, so a hand-authored
`UInputMappingContext` asset can be dropped in later without the pitch
inverting. If look ends up inverted at first run, the fix belongs in the
mapping context, not in `Look()`.

---

## D-05 — An incomplete input config asset is ignored, not patched

**Decision.** `AFarmCharacter::ResolveInputConfig()` uses an assigned
`UFarmInputConfig` only if *every* field is set. Otherwise it logs which
fields are missing and uses the fully code-built config instead.

**Rationale.** Merging generated defaults into a half-authored asset produces
a binding set that matches neither, and the failure is silent. All-or-nothing
means what is live is always obvious from one log line.

The assigned asset is never mutated — writing into a `UDataAsset` at runtime
would dirty the asset in the editor during PIE.

---

## D-06 — Relaxed movement tuning, well below engine defaults

**Decision.** Walk 200 cm/s (engine default 600), max acceleration 600,
braking deceleration 700 (engine default 2048), ground friction 5.

**Rationale.** The stock character walks at a jog and stops dead. At that
pace an environment built for looking at goes past too quickly, and the
instant stop is the single most "video-gamey" thing about default Unreal
movement. 200 cm/s is a real walking pace; ~⅓ s to reach it and a soft stop
read as unhurried without feeling like ice.

Running exists (`bAllowRunning`, Left Shift) but is deliberately not a
priority, per the brief.

---

## D-07 — Eye height 162 cm is a fixed reference for environment scale

**Decision.** Capsule half-height 90 cm, `BaseEyeHeight` 72 cm → the camera
sits ~162 cm above the ground.

**Rationale.** Hay Day is isometric, so every proportion in the references
has to be re-judged for a person standing inside the scene. A fixed, stated
eye height is what makes that judgement repeatable: a fence is "above eye
line" or it is not.

**Consequence.** Changing these numbers invalidates scale decisions made in
Phase 3 onward. Treat them as fixed unless there is a strong reason.

---

## D-08 — No Tick on the Character

**Decision.** `PrimaryActorTick.bCanEverTick = false` on `AFarmCharacter`.

**Rationale.** Movement is integrated by the movement component and the
camera is evaluated from `GetCameraView`, which the camera manager already
calls once per frame. There was no per-frame work left for the actor to do.
Run speed changes the movement component's speed cap on key press rather than
interpolating a value every frame — the acceleration curve smooths it anyway.

---

## D-09 — Lumen with software tracing; no hardware ray tracing yet

**Decision.** `r.DynamicGlobalIlluminationMethod=1`,
`r.ReflectionMethod=1`, `r.Lumen.HardwareRayTracing=0`, plus
`r.GenerateMeshDistanceFields=True` and virtual shadow maps.

**Rationale.** Lumen is what allows good-looking lighting and the Phase 7
time-of-day presets without baking. Software tracing runs on far more GPUs
and the stylised target does not call for mirror-accurate reflections.
Distance field generation is not optional with software tracing — without it
Lumen falls back to screen traces and interiors go black.

Hardware ray tracing is a Phase 12 question, to be answered against a profile
of real content on the actual GPU, not guessed now.

**Also set:** motion blur and lens flare off at project level, per the brief's
comfort requirements, so every new camera and post-process volume inherits
the comfortable default.

---

## D-10 — Static validation stands in for a compiler, and is fault-tested

**Decision.** `Tools/validate_project.py` checks the module for the error
classes that Unreal C++ invites: generated.h placement, missing
`GENERATED_BODY()`, wrong include paths, brace balance, declared-but-never-
defined methods, undeclared `Farm*` types, and project wiring.

**Rationale.** The project is being written without an Unreal installation.
A checker that never fires would be worse than none — so it was tested by
injecting six faults, one per check, and confirming each was caught.

**Limitation.** It is not a compiler. It cannot see engine API mismatches,
which is exactly the class of error recorded as KI-01. Passing it means
"structurally sound", not "builds".

---

## D-11 — Interaction is an interface, and focus runs on a timer

**Decision.** `IFarmInteractable` (a `UINTERFACE`) is the contract.
`UFarmInteractionComponent` on the character sphere-sweeps from the pawn's
view point every 0.08 s and routes presses to whatever it finds.

**Why an interface, not a base actor class.** Farm interactables will be a
windmill, a bench, a chicken, a crop patch, a well. Forcing them to share an
inheritance chain would mean the chicken inheriting from something that knows
about static meshes. `AFarmInteractableActor` exists as an *optional*
convenience base for the common "prop with a mesh" case, not as the contract.

**Why a timer, not Tick.** Focus detection at 60 Hz is imperceptibly
different from 12 Hz — a player's head does not turn that fast — and a sphere
sweep every frame for a whole session is exactly the cost this project should
not pay. Consistent with D-08.

The press path re-tests rather than trusting the cached focus, since focus can
be up to one interval stale and the player may have looked away in between.

**Why a sphere sweep, not a line trace.** A line trace demands pixel-accurate
aiming, which is at odds with a relaxed experience. 14 cm of forgiveness makes
looking "near enough" at something work.

**Why the character broadcasts a delegate instead of calling the component.**
`AFarmCharacter::OnInteractPressed` keeps the character ignorant of what an
interactable is, and lets later systems (a tutorial, an achievement, audio)
listen to the same press without editing the character.

---

## D-12 — The Phase 4 test interactables are real prototypes, not throwaways

**Decision.** The two test interactables are
`AFarmInteractable_Rotator` (toggles continuous rotation) and
`AFarmInteractable_Nudge` (leans away and springs back).

**Rationale.** The brief names both of these as actual features:
"Windmill: start/stop rotation" and "Crop: gentle movement or reaction". Test
fixtures that happen to be the real mechanism cost the same to write and mean
Phase 6 inherits working behaviour instead of starting over — only the
placeholder cube mesh needs replacing.

Rotation uses `URotatingMovementComponent`, so the actor never ticks. The
nudge ticks *only* while the reaction is playing: tick starts disabled, is
enabled on interact, and is disabled again when the motion settles.

**Placeholder:** both use `/Engine/BasicShapes/Cube`. Engine content, so no
authored asset is required.

---

## D-13 — Prompt display is a declared placeholder, the framework is not

**Decision.** The interaction *framework* is complete — focus detection,
range, `CanInteract` gating, focus-enter/leave callbacks, a prompt string, and
a `OnFocusChanged` delegate for UI to subscribe to. The *presentation* is
`GEngine->AddOnScreenDebugMessage`, compiled out of Shipping.

**Rationale.** A real prompt needs a UMG widget, which is a binary asset that
cannot be authored without the editor. Rather than block Phase 4 on that, or
pretend the debug text is the finished feature, the seam is a delegate that a
widget subscribes to later, and the placeholder is recorded as KI-10.

The focus highlight is the same shape: `SetRenderCustomDepth(true)` is the
real hook and is already wired (`r.CustomDepth=3` is set in DefaultEngine.ini),
but it is inert until Phase 10 supplies an outline post-process material. A
4% scale-up stands in so focus is visible in the meantime.
