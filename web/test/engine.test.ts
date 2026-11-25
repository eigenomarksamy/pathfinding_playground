// The adapter over the real WebAssembly module, which `npm run wasm` builds, and over fakes
// for the failures the real module can't be made to show.
import { beforeAll, describe, expect, it } from 'vitest';

import { EngineError, loadEngine, type Engine, type ModuleFactory } from '../src/engine/engine';

function expectEngineError(run: () => unknown, code: EngineError['code']): void {
  try {
    run();
  } catch (error) {
    expect(error).toBeInstanceOf(EngineError);
    expect((error as EngineError).code).toBe(code);
    return;
  }
  throw new Error(`expected an EngineError with code ${code}`);
}

describe('the WebAssembly engine', () => {
  let engine: Engine;

  beforeAll(async () => {
    engine = await loadEngine();
  });

  it('reports its version and limit', () => {
    expect(engine.version).toBe('0.1.0');
    expect(engine.maxSide).toBe(256);
  });

  it('returns a whole frame in one call', () => {
    const frame = engine.bands(4, 3, 0);
    expect(frame.width).toBe(4);
    expect(frame.height).toBe(3);
    // lit where (x + y) mod 8 < 3, as the native tests check
    expect(Array.from(frame.cells)).toEqual([1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0]);
  });

  it('moves the bands one diagonal per step', () => {
    const step0 = engine.bands(8, 1, 0).cells;
    const step1 = engine.bands(8, 1, 1).cells;
    expect(Array.from(step1.subarray(0, 7))).toEqual(Array.from(step0.subarray(1, 8)));
  });

  it('hands over frames the next call does not change', () => {
    const first = engine.bands(8, 8, 0);
    const copy = Array.from(first.cells);
    engine.bands(8, 8, 3);
    expect(Array.from(first.cells)).toEqual(copy);
  });

  it('accepts the largest frame and a phase past 32 bits', () => {
    expect(engine.bands(256, 256, 0).cells).toHaveLength(65536);
    expect(Array.from(engine.bands(8, 1, 2 ** 32 + 1).cells)).toEqual(
      Array.from(engine.bands(8, 1, 1).cells),
    );
  });

  it('rejects sizes out of range as invalid_size', () => {
    expectEngineError(() => engine.bands(0, 4, 0), 'invalid_size');
    expectEngineError(() => engine.bands(4, 257, 0), 'invalid_size');
    expectEngineError(() => engine.bands(-3, 4, 0), 'invalid_size');
  });

  it('rejects arguments that are not whole numbers as invalid_input', () => {
    expectEngineError(() => engine.bands(4.5, 4, 0), 'invalid_input');
    expectEngineError(() => engine.bands(Number.NaN, 4, 0), 'invalid_input');
    expectEngineError(() => engine.bands(4, 4, -1), 'invalid_input');
    expectEngineError(() => engine.bands(4, 4, 0.5), 'invalid_input');
  });
});

describe('the adapter', () => {
  it('reports a module that fails to start as load_failed', async () => {
    const failing: ModuleFactory = () => Promise.reject(new Error('no WebAssembly here'));
    await expect(loadEngine(failing)).rejects.toMatchObject({
      name: 'EngineError',
      code: 'load_failed',
    });
  });

  it('reports a module that throws or answers oddly as unexpected', async () => {
    const real = await loadEngine();
    const Status = { ok: { value: 0 }, invalid_size: { value: 1 }, buffer_too_small: { value: 2 } };
    const fake = (bands: () => unknown) =>
      (() =>
        Promise.resolve({
          Status,
          maxSide: real.maxSide,
          engineVersion: () => real.version,
          bands,
        })) as unknown as ModuleFactory;

    const throwing = await loadEngine(
      fake(() => {
        throw new Error('abort');
      }),
    );
    expectEngineError(() => throwing.bands(4, 4, 0), 'unexpected');

    const otherStatus = await loadEngine(
      fake(() => ({ status: Status.buffer_too_small, cells: null })),
    );
    expectEngineError(() => otherStatus.bands(4, 4, 0), 'unexpected');

    const wrongSize = await loadEngine(
      fake(() => ({ status: Status.ok, cells: new Uint8Array(3) })),
    );
    expectEngineError(() => wrongSize.bands(4, 4, 0), 'unexpected');
  });
});
