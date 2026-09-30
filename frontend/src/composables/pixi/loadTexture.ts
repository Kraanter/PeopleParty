import { Assets, type Sprite, type Texture } from 'pixi.js'

// Pixi v8's Texture.from(url)/Sprite.from(url) is cache-only for a string id — confirmed by
// reading textureFrom() in the installed pixi.js source: `if (typeof id === 'string') return
// Cache.get(id)`, with no fallback to actually load it. Unlike v7, there is no implicit
// fetch-on-first-use for a bare URL string; a path never explicitly run through Assets.load()
// silently resolves to `undefined` (Sprite's texture setter then falls back to Texture.EMPTY —
// nothing renders, no error, no warning beyond a benign "not found in the Cache" console line).

const inFlight = new Map<string, Promise<Texture>>()

/**
 * Assigns `sprite.texture` from a URL, tolerant of the cache-only behavior above. Synchronous
 * (no visible flash) once a URL has been loaded once anywhere on the page, since Assets caches
 * by URL; the very first sprite ever created from a given URL renders blank for one or two
 * frames while the real fetch resolves, then `invalidate` triggers the repaint that shows it.
 */
export function loadTexture(sprite: Sprite, url: string, invalidate: () => void): void {
  if (Assets.cache.has(url)) {
    sprite.texture = Assets.cache.get(url) as Texture
    return
  }

  let promise = inFlight.get(url)
  if (!promise) {
    promise = Assets.load<Texture>(url)
    inFlight.set(url, promise)
  }

  promise.then((texture) => {
    sprite.texture = texture
    invalidate()
  })
}
