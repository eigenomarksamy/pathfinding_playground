// A fixed-step clock, so the simulation advances at its own rate whatever the display's
// frame rate: each animation frame adds its elapsed time, and the clock answers how many
// whole steps are due.

export interface ClockState {
  /** time not yet spent on a step, in milliseconds */
  readonly carryMs: number;
}

export const initialClock: ClockState = { carryMs: 0 };

/** at most this many steps per frame, so a long pause, such as a hidden tab, can't flood the engine. */
export const maxStepsPerFrame = 8;

const toleranceMs = 1e-6;

export function advance(
  state: ClockState,
  elapsedMs: number,
  stepsPerSecond: number,
): { readonly steps: number; readonly state: ClockState } {
  if (!(stepsPerSecond > 0) || !(elapsedMs > 0)) {
    return { steps: 0, state };
  }
  const stepMs = 1000 / stepsPerSecond;
  const total = state.carryMs + elapsedMs;
  // the tolerance absorbs rounding, so sixty frames of 1000 / 60 ms make a whole second.
  const due = Math.floor((total + toleranceMs) / stepMs);
  if (due > maxStepsPerFrame) {
    // the time past the cap is dropped rather than caught up later.
    return { steps: maxStepsPerFrame, state: initialClock };
  }
  return { steps: due, state: { carryMs: Math.max(0, total - due * stepMs) } };
}
