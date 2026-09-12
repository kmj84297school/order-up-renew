# Bench view verification

Status: authored 2026-09-12; **not compiled or run in Unreal**.

Build the editor target and create the bootstrap level using NEXT_TASK.md first.
The bootstrap adds a PLACEHOLDER seat block at (350, 400, 22.5), in centimetres.
It is not a farm layout or a finished bench asset.

## Engine acceptance checks

1. Approach the seat and aim down at it within 250 cm. E should show "Sit on
   bench", then blend to the seated eye position (350, 400, 117.5) in 0.75 s.
2. Hold WASD before sitting: translation should stop. While seated, WASD and
   Shift must not move the pawn; mouse look should remain responsive.
3. Look away, including fully behind the bench. "Stand up" must remain visible.
   Press E once: return to Free Walk in 0.6 s and regain movement at the original
   approach position. No teleport or capsule resizing is performed.
4. Hold E: built-in input must toggle once per press, not every frame.
5. Repeat sitting/standing at least ten times, including presses during each
   blend. Check for camera jumps and accumulating movement locks.
6. While seated, destroy the bench in PIE. Within one focus timer interval
   (0.08 s), the camera should return and movement should be released.
7. Unpossess/repossess while seated; stop/restart PIE. The old controller must
   not retain this session's movement lock.
8. Add one independent SetIgnoreMoveInput(true) lock while seated. Standing
   must release only the bench lock. Release the independent lock separately.
9. Toggle the rotator while keeping focus on it. Its prompt must update
   immediately, and OnFocusChanged must report the new text. Looking away or
   destroying a focused actor must clear the prompt.
10. Run the original movement/interaction checklist in NEXT_TASK.md. Check
    output logs for errors and record failures in KNOWN_ISSUES.md.

## Deliberate limits

- View-only sitting: the capsule stays at its approach position. No seated body
  animation or physical seat relocation is implemented.
- No camera obstruction sweep yet. Check the transition in the open bootstrap
  area; authored seats near walls need obstruction handling before adoption.
- Entry is rejected while airborne. The seat is stationary; its eye position is
  sampled on entry. Moving seats and multiplayer occupancy are outside scope.
- These steps are a pending manual procedure, not test results.
