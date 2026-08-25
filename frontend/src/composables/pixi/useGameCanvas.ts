import { Application, Container, type ApplicationOptions } from 'pixi.js'
import {
  computed,
  onMounted,
  onScopeDispose,
  ref,
  watchEffect,
  type ComputedRef,
  type Ref
} from 'vue'

// One requestAnimationFrame loop shared by every useGameCanvas() instance on the page,
// instead of one per canvas (or Pixi's own Ticker, which redraws at full display refresh
// whether or not anything changed). A canvas only costs a frame when it's dirty or has
// opted into `animate`. Paused entirely while the tab is hidden, mirroring the
// visibilitychange handling BackgroundComponent.vue does for the CSS background.

interface LoopEntry {
  dirty: boolean
  animate: ((ctx: { dt: number }) => void) | null
  render: () => void
}

const loopEntries = new Set<LoopEntry>()
let rafHandle: number | null = null
let lastFrameTime = 0
// True for the whole duration of a runFrame() call. Distinguishes "rafHandle is null
// because we're mid-frame" (expected — cleared at the top of every frame) from "rafHandle
// is null because the loop is genuinely idle". Without this, an animate() callback that
// calls invalidate() (a completely normal thing to do — see useSnapshotBuffer.push())
// would see rafHandle===null mid-frame and schedule a second frame on top of the one the
// frame's own tail schedules, doubling every frame — an exponential runaway confirmed live
// (8191 renders in 500ms) while verifying this composable, not a hypothetical.
let frameInProgress = false

function scheduleFrameIfNeeded() {
  if (rafHandle === null && loopEntries.size > 0 && !document.hidden) {
    rafHandle = requestAnimationFrame(runFrame)
  }
}

function runFrame(time: number) {
  rafHandle = null
  frameInProgress = true
  // Resets to 0 whenever the loop (re)starts, so the first tick after being idle or
  // hidden reports dt=0 instead of a jump the size of however long the loop was stopped.
  const dt = lastFrameTime ? time - lastFrameTime : 0
  lastFrameTime = time

  try {
    for (const entry of loopEntries) {
      try {
        if (entry.animate) {
          entry.animate({ dt })
          entry.dirty = true // continuous mode: something changes by definition every tick
        }
        if (entry.dirty) {
          entry.dirty = false
          entry.render()
        }
      } catch (err) {
        // One canvas's render/animate throwing must not wedge the shared loop for
        // every other canvas registered against it.
        console.error('[useGameCanvas] render/animate callback threw', err)
      }
    }
  } finally {
    frameInProgress = false
    scheduleFrameIfNeeded()
  }
}

function ensureLoopRunning() {
  if (frameInProgress) return // the tail of the in-flight frame will (re)schedule once, after it finishes
  if (rafHandle === null && loopEntries.size > 0 && !document.hidden) {
    lastFrameTime = 0
    rafHandle = requestAnimationFrame(runFrame)
  }
}

document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    if (rafHandle !== null) {
      cancelAnimationFrame(rafHandle)
      rafHandle = null
    }
  } else {
    ensureLoopRunning()
  }
})

function registerLoopEntry(entry: LoopEntry) {
  loopEntries.add(entry)
  ensureLoopRunning()
  return () => {
    loopEntries.delete(entry)
    if (loopEntries.size === 0 && rafHandle !== null) {
      cancelAnimationFrame(rafHandle)
      rafHandle = null
    }
  }
}

export interface UseGameCanvasOptions {
  /** DOM mount point for app.canvas. Must be a real, non-v-if-gated element by onMounted. */
  container: Ref<HTMLElement | null>
  /** Draw into `root` — called once after init, then again on every invalidate() or animate() tick. */
  render: (ctx: { root: Container; app: Application }) => void
  /** "Trust the parent" mode, e.g. GameManager's own width/height props. Omit to size from the container itself. */
  size?: () => { width: number; height: number }
  /** Only consulted when `size` is omitted. Letterboxes to this aspect ratio, shellShuffle-style. */
  worldSize?: { width: number; height: number }
  backgroundColor?: number | string
  backgroundAlpha?: number
  antialias?: boolean
  resolution?: number
}

export interface GameCanvasHandle {
  /** Add your scene graph here, not to app.stage directly — already positioned/scaled for letterboxing. */
  readonly root: Container
  /** undefined until the first valid size triggers init(); render() callbacks never observe it undefined. */
  readonly app: Application | undefined
  readonly width: ComputedRef<number>
  readonly height: ComputedRef<number>
  readonly scale: ComputedRef<number>
  readonly offsetX: ComputedRef<number>
  readonly offsetY: ComputedRef<number>
  invalidate: () => void
  /** Continuous per-frame mode for dt-based interpolation. Never auto-stopped — call
   *  stopAnimating() once your interpolation has settled, or idle scenes never stop rendering. */
  startAnimating: (callback: (ctx: { dt: number }) => void) => void
  stopAnimating: () => void
}

export function useGameCanvas(options: UseGameCanvasOptions): GameCanvasHandle {
  const root = new Container()

  const widthPx = ref(0)
  const heightPx = ref(0)
  const scaleRef = ref(1)
  const offsetXRef = ref(0)
  const offsetYRef = ref(0)

  let app: Application | undefined
  let initializing = false
  let disposed = false
  let unregister: (() => void) | null = null

  const loopEntry: LoopEntry = {
    dirty: false,
    animate: null,
    render: () => {
      if (!app) return
      options.render({ root, app })
      app.render()
    }
  }

  function invalidate() {
    loopEntry.dirty = true
    ensureLoopRunning()
  }

  function applySize(rawWidth: number, rawHeight: number) {
    let width = rawWidth
    let height = rawHeight
    let scale = 1
    let offsetX = 0
    let offsetY = 0

    if (!options.size && options.worldSize) {
      const aspect = options.worldSize.width / options.worldSize.height
      width = Math.min(rawWidth, Math.round(rawHeight * aspect))
      height = Math.round(width / aspect)
      scale = width / options.worldSize.width
      offsetX = (rawWidth - width) / 2
      offsetY = (rawHeight - height) / 2
    }

    widthPx.value = width
    heightPx.value = height
    scaleRef.value = scale
    offsetXRef.value = offsetX
    offsetYRef.value = offsetY
    root.position.set(offsetX, offsetY)
    root.scale.set(scale)

    if (width <= 0 || height <= 0) return // not laid out yet — wait for a real size

    if (!app) {
      if (!initializing) {
        initializing = true
        void startInit(width, height)
      }
      return
    }

    app.renderer.resize(width, height)
    invalidate()
  }

  async function startInit(width: number, height: number) {
    const instance = new Application()
    const initOptions: Partial<ApplicationOptions> = {
      width,
      height,
      antialias: options.antialias ?? true,
      resolution: options.resolution ?? Math.min(window.devicePixelRatio || 1, 2),
      autoDensity: true,
      // Pixi's own Ticker would re-issue draw calls at full display refresh whether or not
      // anything changed. The shared rAF loop above takes that job over instead, on an
      // opt-in, dirty-flag basis.
      autoStart: false
    }
    // Pixi merges init options as {...defaults, ...yours} — an explicit undefined here
    // would override Pixi's real default instead of falling back to it.
    if (options.backgroundColor !== undefined) initOptions.backgroundColor = options.backgroundColor
    if (options.backgroundAlpha !== undefined) initOptions.backgroundAlpha = options.backgroundAlpha

    try {
      await instance.init(initOptions)
    } catch (err) {
      console.error('[useGameCanvas] Application.init() failed, canvas will stay blank', err)
      return
    }

    if (disposed || !options.container.value) {
      // Torn down (or the container ref went away) while init() was in flight. Application.init()
      // unconditionally replaces `this.renderer` with no teardown of whatever was there before, so
      // an un-mounted instance left alive here would leak a live WebGL context forever.
      instance.destroy(true, { children: true })
      return
    }

    instance.stage.addChild(root)
    options.container.value.appendChild(instance.canvas)
    // The size that triggered this init() may already be stale if another resize landed
    // while awaiting — resize unconditionally rather than trust init()'s own options.
    instance.renderer.resize(widthPx.value, heightPx.value)

    app = instance
    unregister = registerLoopEntry(loopEntry)
    loopEntry.render() // paint immediately, don't wait for the next rAF tick
  }

  onMounted(() => {
    const el = options.container.value
    if (!el) {
      console.error('[useGameCanvas] container ref is empty on mount — canvas will never appear')
      return
    }

    let stopSizeWatch: (() => void) | null = null
    let resizeObserver: ResizeObserver | null = null

    if (options.size) {
      const getSize = options.size
      stopSizeWatch = watchEffect(() => {
        const { width, height } = getSize()
        applySize(width, height)
      })
    } else {
      applySize(el.clientWidth, el.clientHeight)
      resizeObserver = new ResizeObserver((entries) => {
        const { width, height } = entries[0].contentRect
        applySize(width, height)
      })
      resizeObserver.observe(el)
    }

    onScopeDispose(() => {
      disposed = true
      stopSizeWatch?.()
      resizeObserver?.disconnect()
      unregister?.()
      app?.destroy(true, { children: true })
    })
  })

  return {
    root,
    get app() {
      return app
    },
    width: computed(() => widthPx.value),
    height: computed(() => heightPx.value),
    scale: computed(() => scaleRef.value),
    offsetX: computed(() => offsetXRef.value),
    offsetY: computed(() => offsetYRef.value),
    invalidate,
    startAnimating(callback) {
      loopEntry.animate = callback
      ensureLoopRunning()
    },
    stopAnimating() {
      loopEntry.animate = null
    }
  }
}
