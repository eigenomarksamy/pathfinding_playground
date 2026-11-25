import { useEffect, useRef, useState } from 'react';

import { EngineError, type Engine } from '../engine/engine';
import { advance, initialClock } from '../simulation/clock';
import { drawFrame } from '../visualization/drawFrame';
import { Controls } from './Controls';

const columns = 48;
const rows = 32;
const cellPx = 12;
const defaultSpeed = 8;

function prefersReducedMotion(): boolean {
  return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
}

function readPalette(element: Element) {
  const style = getComputedStyle(element);
  return {
    background: style.getPropertyValue('--cell-background').trim() || '#000',
    lit: style.getPropertyValue('--cell-lit').trim() || '#fff',
  };
}

interface PlaygroundProps {
  engine: Engine;
}

export function Playground({ engine }: PlaygroundProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [step, setStep] = useState(0);
  // with reduced motion asked for, nothing moves until play is pressed.
  const [playing, setPlaying] = useState(() => !prefersReducedMotion());
  const [speed, setSpeed] = useState(defaultSpeed);
  const [error, setError] = useState<string | null>(null);

  // the simulation steps on a fixed clock, apart from the display's frame rate.
  useEffect(() => {
    if (!playing) {
      return undefined;
    }
    let clock = initialClock;
    let last: number | null = null;
    let frame = requestAnimationFrame(function tick(now) {
      if (last !== null) {
        const result = advance(clock, now - last, speed);
        clock = result.state;
        if (result.steps > 0) {
          setStep((current) => current + result.steps);
        }
      }
      last = now;
      frame = requestAnimationFrame(tick);
    });
    return () => cancelAnimationFrame(frame);
  }, [playing, speed]);

  useEffect(() => {
    const canvas = canvasRef.current;
    const context = canvas?.getContext('2d');
    if (!canvas || !context) {
      return;
    }
    try {
      drawFrame(context, engine.bands(columns, rows, step), cellPx, readPalette(canvas));
      setError(null);
    } catch (caught) {
      setPlaying(false);
      setError(caught instanceof EngineError ? caught.message : 'the frame could not be drawn');
    }
  }, [engine, step]);

  return (
    <section className="playground" aria-label="playground">
      <canvas
        ref={canvasRef}
        width={columns * cellPx}
        height={rows * cellPx}
        role="img"
        aria-label={`a ${columns} by ${rows} grid of moving diagonal bands, at step ${step}`}
      />
      {error && (
        <p className="error" role="alert">
          {error}
        </p>
      )}
      <Controls
        playing={playing}
        speed={speed}
        step={step}
        onPlayPause={() => setPlaying((current) => !current)}
        onStep={() => setStep((current) => current + 1)}
        onReset={() => {
          setPlaying(false);
          setStep(0);
        }}
        onSpeed={setSpeed}
      />
    </section>
  );
}
