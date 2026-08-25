import { shallowRef, type ShallowRef } from 'vue'

export interface SnapshotBufferOptions<TSnapshot, THud> {
  invalidate: () => void
  toHud?: (snapshot: TSnapshot) => THud
}

export interface SnapshotBufferHandle<TSnapshot, THud> {
  readonly current: TSnapshot
  push: (next: TSnapshot) => void
  hud: ShallowRef<THud | undefined>
}

/**
 * Plain mutable holder for the game state a render() callback reads every frame — not a ref,
 * since render callbacks run off the shared rAF loop, not Vue's reactivity system. `initial` is
 * real first state, not a placeholder: GameManager's 500ms debounce before mounting the real
 * minigame component means several snapshots can already have arrived and been silently
 * dropped before this is ever constructed.
 */
export function useSnapshotBuffer<TSnapshot, THud = TSnapshot>(
  initial: TSnapshot,
  options: SnapshotBufferOptions<TSnapshot, THud>
): SnapshotBufferHandle<TSnapshot, THud> {
  let snapshot = initial
  const hud = shallowRef<THud | undefined>(options.toHud?.(initial))

  function push(next: TSnapshot) {
    snapshot = next
    if (options.toHud) hud.value = options.toHud(next)
    options.invalidate()
  }

  return {
    get current() {
      return snapshot
    },
    push,
    hud
  }
}
