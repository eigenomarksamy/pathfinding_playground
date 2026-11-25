// The typed adapter over the WebAssembly engine: it loads the module, checks and converts
// what the app passes in, and turns the engine's status codes into typed errors. Nothing
// else in the app imports the generated module.
import createPathfindingModule, { type MainModule } from './wasm/pathfinding.js';

export type EngineErrorCode =
  /** the module failed to download, compile or start */
  | 'load_failed'
  /** an argument isn't a whole number in range */
  | 'invalid_input'
  /** the engine rejected the width or height */
  | 'invalid_size'
  /** the engine answered in a way the adapter doesn't expect */
  | 'unexpected';

export class EngineError extends Error {
  readonly code: EngineErrorCode;

  constructor(code: EngineErrorCode, message: string, options?: { cause?: unknown }) {
    super(message, options);
    this.name = 'EngineError';
    this.code = code;
  }
}

/** a frame of cells, row by row: `cells[y * width + x]`, 1 where a band lies and 0 elsewhere. */
export interface Frame {
  readonly width: number;
  readonly height: number;
  readonly cells: Uint8Array;
}

export interface Engine {
  /** the engine's version, from CMakeLists.txt */
  readonly version: string;
  /** the largest width or height the engine accepts */
  readonly maxSide: number;
  /** the bootstrap's probe: diagonal bands, moved one cell per step of `phase`. */
  bands(width: number, height: number, phase: number): Frame;
}

export type ModuleFactory = () => Promise<MainModule>;

export async function loadEngine(
  factory: ModuleFactory = () => createPathfindingModule(),
): Promise<Engine> {
  let module: MainModule;
  try {
    module = await factory();
  } catch (cause) {
    throw new EngineError('load_failed', 'the WebAssembly engine failed to load', { cause });
  }
  return wrap(module);
}

function wrap(module: MainModule): Engine {
  const version = module.engineVersion();
  const maxSide = module.maxSide;

  function checkSide(name: string, value: number): void {
    if (!Number.isInteger(value)) {
      throw new EngineError('invalid_input', `${name} must be a whole number, not ${value}`);
    }
    if (value < 1 || value > maxSide) {
      throw new EngineError('invalid_size', `${name} must be from 1 to ${maxSide}, not ${value}`);
    }
  }

  return {
    version,
    maxSide,
    bands(width, height, phase) {
      checkSide('width', width);
      checkSide('height', height);
      if (!Number.isSafeInteger(phase) || phase < 0) {
        throw new EngineError('invalid_input', `phase must be a whole number from 0, not ${phase}`);
      }
      // the engine takes a 32-bit phase, and its pattern repeats long before that wraps.
      const phase32 = phase % 2 ** 32;

      let result: ReturnType<MainModule['bands']>;
      try {
        result = module.bands(width, height, phase32);
      } catch (cause) {
        throw new EngineError('unexpected', 'the engine failed while computing a frame', { cause });
      }

      switch (result.status) {
        case module.Status.ok:
          break;
        case module.Status.invalid_size:
          throw new EngineError('invalid_size', `the engine rejected a ${width} x ${height} frame`);
        default:
          throw new EngineError(
            'unexpected',
            `the engine answered with status ${result.status.value}`,
          );
      }
      const cells: unknown = result.cells;
      if (!(cells instanceof Uint8Array) || cells.length !== width * height) {
        throw new EngineError('unexpected', 'the engine returned a frame of the wrong size');
      }
      return { width, height, cells };
    },
  };
}
