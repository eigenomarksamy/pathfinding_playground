import { describe, expect, it } from 'vitest';

import { advance, initialClock, maxStepsPerFrame } from '../src/simulation/clock';

describe('the fixed-step clock', () => {
  it('steps at its own rate whatever the frame rate', () => {
    // 10 steps per second for one second, at 60 frames per second and at 24
    for (const fps of [60, 24]) {
      let state = initialClock;
      let steps = 0;
      for (let frame = 0; frame < fps; frame += 1) {
        const result = advance(state, 1000 / fps, 10);
        state = result.state;
        steps += result.steps;
      }
      expect(steps).toBe(10);
    }
  });

  it('carries the time left over to the next frame', () => {
    const first = advance(initialClock, 150, 10);
    expect(first.steps).toBe(1);
    expect(first.state.carryMs).toBeCloseTo(50);
    expect(advance(first.state, 50, 10).steps).toBe(1);
  });

  it('caps the steps after a long pause and drops the rest', () => {
    const result = advance(initialClock, 60_000, 30);
    expect(result.steps).toBe(maxStepsPerFrame);
    expect(result.state).toEqual(initialClock);
  });

  it('does nothing for a zero, negative or invalid time or speed', () => {
    expect(advance(initialClock, 0, 10).steps).toBe(0);
    expect(advance(initialClock, -5, 10).steps).toBe(0);
    expect(advance(initialClock, 100, 0).steps).toBe(0);
    expect(advance(initialClock, Number.NaN, 10).steps).toBe(0);
  });
});
