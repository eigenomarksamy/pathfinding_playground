import { useEffect, useState } from 'react';

import { AboutPanel } from './components/AboutPanel';
import { Playground } from './components/Playground';
import { loadEngine, type Engine } from './engine/engine';

type EngineState =
  | { readonly status: 'loading' }
  | { readonly status: 'ready'; readonly engine: Engine }
  | { readonly status: 'failed'; readonly message: string };

export function App() {
  const [state, setState] = useState<EngineState>({ status: 'loading' });

  useEffect(() => {
    let live = true;
    loadEngine().then(
      (engine) => live && setState({ status: 'ready', engine }),
      () =>
        live &&
        setState({
          status: 'failed',
          message: 'the engine could not start. this browser may not support WebAssembly.',
        }),
    );
    return () => {
      live = false;
    };
  }, []);

  return (
    <div className="app">
      <header>
        <h1>pathfinding playground</h1>
      </header>
      <main>
        {state.status === 'ready' && <Playground engine={state.engine} />}
        {state.status === 'loading' && <p className="status">loading the engine…</p>}
        {state.status === 'failed' && (
          <p className="status error" role="alert">
            {state.message}
          </p>
        )}
        <AboutPanel engineVersion={state.status === 'ready' ? state.engine.version : null} />
      </main>
      <footer>
        {/* a new tab, since leaving would lose an edited map */}
        <a className="button" href="https://eigenomar.com" target="_blank" rel="noopener">
          eigenomar.com
        </a>
      </footer>
    </div>
  );
}
