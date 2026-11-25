import type { Frame } from '../engine/engine';

export interface Palette {
  readonly background: string;
  readonly lit: string;
}

/** draws a frame onto a canvas sized `frame.width * cellPx` by `frame.height * cellPx`. */
export function drawFrame(
  context: CanvasRenderingContext2D,
  frame: Frame,
  cellPx: number,
  palette: Palette,
): void {
  context.fillStyle = palette.background;
  context.fillRect(0, 0, frame.width * cellPx, frame.height * cellPx);
  context.fillStyle = palette.lit;
  const gap = cellPx >= 6 ? 1 : 0;
  for (let y = 0; y < frame.height; y += 1) {
    for (let x = 0; x < frame.width; x += 1) {
      if (frame.cells[y * frame.width + x] === 1) {
        context.fillRect(x * cellPx, y * cellPx, cellPx - gap, cellPx - gap);
      }
    }
  }
}
