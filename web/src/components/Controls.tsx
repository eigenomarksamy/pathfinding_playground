export const minSpeed = 1;
export const maxSpeed = 30;

interface ControlsProps {
  playing: boolean;
  speed: number;
  step: number;
  onPlayPause(): void;
  onStep(): void;
  onReset(): void;
  onSpeed(speed: number): void;
}

export function Controls({
  playing,
  speed,
  step,
  onPlayPause,
  onStep,
  onReset,
  onSpeed,
}: ControlsProps) {
  return (
    <div className="controls" role="group" aria-label="simulation controls">
      <button type="button" onClick={onPlayPause} aria-pressed={playing}>
        {playing ? 'pause' : 'play'}
      </button>
      <button type="button" onClick={onStep}>
        step
      </button>
      <button type="button" onClick={onReset}>
        reset
      </button>
      <label className="speed">
        <span>speed</span>
        <input
          type="range"
          min={minSpeed}
          max={maxSpeed}
          value={speed}
          onChange={(event) => onSpeed(Number(event.currentTarget.value))}
        />
        <output>{speed} steps/s</output>
      </label>
      <output className="step-count" aria-live="off">
        step {step}
      </output>
    </div>
  );
}
