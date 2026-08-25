import type { Container } from 'pixi.js'

export interface EntityLayerOptions<TEntity, TDisplay extends Container> {
  key: (entity: TEntity) => string | number
  create: () => TDisplay
  update: (display: TDisplay, entity: TEntity) => void
  release?: (display: TDisplay) => void
}

export interface EntityLayerHandle<TEntity> {
  /** Full-snapshot diff against the active set — present entities update, new ones are
   *  created-or-reused, missing ones return to the pool instead of being destroyed. */
  sync: (entities: readonly TEntity[]) => void
  /** Releases everything currently active (e.g. on a phase transition). Objects stay pool-available. */
  clear: () => void
  /** Final teardown — destroys every pooled and active display object. */
  destroy: () => void
}

/**
 * Keyed object pool diffed against a full snapshot every sync() call — the direct replacement
 * for marbleMania/highwayHustle's :key-driven Application recreation. `parent` is usually a
 * useGameCanvas handle's `root` (or a Container added under it).
 */
export function createEntityLayer<TEntity, TDisplay extends Container>(
  parent: Container,
  options: EntityLayerOptions<TEntity, TDisplay>
): EntityLayerHandle<TEntity> {
  const active = new Map<string | number, TDisplay>()
  const pool: TDisplay[] = []

  function acquire(): TDisplay {
    return pool.pop() ?? options.create()
  }

  function releaseToPool(key: string | number, display: TDisplay) {
    active.delete(key)
    parent.removeChild(display)
    options.release?.(display)
    pool.push(display)
  }

  function sync(entities: readonly TEntity[]) {
    const seen = new Set<string | number>()

    for (const entity of entities) {
      const key = options.key(entity)
      if (seen.has(key)) {
        console.warn(`[createEntityLayer] duplicate key "${key}" in one sync() call — skipping`)
        continue
      }
      seen.add(key)

      let display = active.get(key)
      if (!display) {
        display = acquire()
        active.set(key, display)
        // Reused/new objects always append to the end of parent.children (top of z-order) —
        // z-order isn't stable across recycling. Not a problem for any current minigame (flat
        // shapes, no interactivity, ≤16 entities); if a future one needs stable stacking, set
        // parent.sortableChildren = true and give each display object a stable .zIndex.
        parent.addChild(display)
      }
      options.update(display, entity)
    }

    for (const [key, display] of active) {
      if (!seen.has(key)) releaseToPool(key, display)
    }
  }

  function clear() {
    for (const [key, display] of active) releaseToPool(key, display)
  }

  function destroy() {
    // Container#destroy() is idempotent — safe even if useGameCanvas's own
    // app.destroy({children:true}) already tore down whatever's still parented.
    for (const display of active.values()) display.destroy()
    for (const display of pool) display.destroy()
    active.clear()
    pool.length = 0
  }

  return { sync, clear, destroy }
}
