export const sourceUrl = 'https://github.com/eigenomarksamy/pathfinding_playground';

interface AboutPanelProps {
  engineVersion: string | null;
}

export function AboutPanel({ engineVersion }: AboutPanelProps) {
  return (
    <aside className="about" aria-labelledby="about-heading">
      <h2 id="about-heading">about</h2>
      <p>
        an interactive laboratory for graph search: build a map, pick a solver, watch its decisions
        step by step, and compare how algorithms find a route.
      </p>
      <p>
        this is the bootstrap. the bands on the canvas are computed by the C++20 engine, compiled to
        WebAssembly, one whole frame per step. the grid editor and the solvers come next.
      </p>
      <dl>
        <dt>engine</dt>
        <dd>{engineVersion ?? 'not loaded'}</dd>
        <dt>source</dt>
        <dd>
          <a href={sourceUrl} target="_blank" rel="noopener noreferrer">
            github.com/eigenomarksamy/pathfinding_playground
          </a>
        </dd>
      </dl>
    </aside>
  );
}
