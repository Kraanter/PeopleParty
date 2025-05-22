<script setup lang="ts">
import { ref, reactive, computed, onBeforeUnmount, watch, onMounted } from 'vue'
import type { CSSProperties } from 'vue'

// TYPES: REMOVE ============

enum Shape {
    Circle = "circle",
    Square = "square"
}
enum InteractionEvents {
    PointerDown = "pointerdown",
    PointerMove = "pointermove",
    PointerUp = "pointerup",
    FollowStop = "FOLLOW_STOP"
}
type Direction = 'FORWARD' | 'RIGHT' | 'LEFT' | 'BACKWARD';
interface UpdateEvent {
    type: 'move' | 'stop' | 'start';
    x?: number;
    y?: number;
    direction?: Direction;
    distance?: number;
}
interface Coordinates {
    relativeX: number;
    relativeY: number;
    axisX: number;
    axisY: number;
    direction: Direction;
    distance: number;
}
interface State {
    dragging: boolean;
    coordinates?: Coordinates;
}
/**
 * Radians identifying the direction of the joystick
 */
enum RadianQuadrantBinding {
    TopRight = 2.35619449,
    TopLeft = -2.35619449,
    BottomRight = 0.785398163,
    BottomLeft = -0.785398163
}

// =========================

const _onPointerMove = (event: PointerEvent) => _pointerMove(event)
const _onPointerUp = (event: PointerEvent) => _pointerUp(event)

export interface JoystickComponentProps {
  /** The size of the joystick base (default: `100`) */
  size?: number
  /** The size of the joystick stick (defaults to ⅔ the size of the base) */
  stickSize?: number
  /** The color of the joystick base (default: `rgb(0 0 0 / 50%)`) */
  baseColor?: string
  /** The color of the joystick stick (default: `mediumpurple`) */
  stickColor?: string
  /** The duration to throttle the `move` event by in ms (default: `100`) */
  throttle?: number
  /** Disables the joystick from triggering actions (default: `false`) */
  disabled?: boolean
  /**
   * Determines whether the joystick sticks to the positions it is moved to.
   *
   * - `false` will bring the joystick back to its original position when released
   * - `true` will keep the joystick in its place when moved
   *
   * (default: `false`)
   **/
  sticky?: boolean
  /** The image to use for the joystick base background using CSS `background: url(...)` */
  baseImage?: string
  /** The image to use for the joystick stick background using CSS `background: url(...)` */
  stickImage?: string
  /** Determines whether the joystick follows your cursor (default: `false`) */
  followCursor?: boolean
  /** The shape of the joystick base (default: `circle`) */
  baseShape?: Shape
  /** The shape of the joystick stick (default: `circle`) */
  stickShape?: Shape
  /** The shape of the joystick control plane (default: `circle`) */
  controlPlaneShape?: Shape
  /** The minimum distance to travel before triggering the `move` event between 0 - 100 (default: `0`) */
  minDistance?: number
}

const props = withDefaults(defineProps<JoystickComponentProps>(), {
  size: 100,
  stickSize: undefined,
  baseColor: 'rgb(0 0 0 / 50%)',
  stickColor: 'mediumpurple',
  throttle: 100,
  disabled: false,
  sticky: false,
  baseImage: undefined,
  stickImage: undefined,
  followCursor: false,
  baseShape: Shape.Circle,
  stickShape: Shape.Circle,
  controlPlaneShape: Shape.Circle,
  minDistance: 0,
})

const emit = defineEmits<{
  (action: 'move', event: UpdateEvent): void
  (action: 'start', event: UpdateEvent): void
  (action: 'stop', event: UpdateEvent): void
}>()

const state = reactive<State>({ dragging: false })

const baseRef = ref<HTMLElement>()
const stickRef = ref<HTMLElement>()

const _parentRect = ref<DOMRect>()
const _pointerId = ref<number>()

const _stickSize = computed(() => props.stickSize || props.size / 1.5)
const _radius = computed(() => props.size / 2)

const _isClient = typeof window !== 'undefined'

const shapeFactory = (shape: Shape, size: number) => {
  return { borderRadius: `${shape === Shape.Square ? Math.sqrt(size) : size}px` }
}

const shapeBoundsFactory = (
  shape: Shape,
  absoluteX: number,
  absoluteY: number,
  relativeX: number,
  relativeY: number,
  dist: number,
  radius: number,
  baseSize: number,
  parentRect: DOMRect,
) => {
  if (shape === Shape.Square) {
    relativeX = getWithinBounds(absoluteX - parentRect.left - baseSize / 2, baseSize)
    relativeY = getWithinBounds(absoluteY - parentRect.top - baseSize / 2, baseSize)
    return { relativeX, relativeY }
  }
  if (dist > radius) {
    relativeX *= radius / dist
    relativeY *= radius / dist
  }
  return { relativeX, relativeY }
}

const getWithinBounds = (value: number, baseSize: number): number => {
  const halfBaseSize = baseSize / 2
  if (value > halfBaseSize) return halfBaseSize
  if (value < -halfBaseSize) return halfBaseSize * -1
  return value
}

const _throttleMoveCallback = (() => {
  let lastCall = 0
  return (event: UpdateEvent) => {
    const now = Date.now()
    const throttleAmount = props.throttle
    if (now - lastCall < throttleAmount) return
    lastCall = now
    return emit('move', event)
  }
})()

const getBaseShapeStyle = () => shapeFactory(props.baseShape, props.size)
const getStickShapeStyle = () => shapeFactory(props.stickShape, props.size)

const baseStyle = computed(() => {
  const baseColor = props.baseColor
  const baseSizeString = `${props.size}px`
  const padStyle = {
    ...getBaseShapeStyle(),
    height: baseSizeString,
    width: baseSizeString,
    background: baseColor,
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
  } as CSSProperties

  if (props.baseImage) {
    padStyle.background = `url(${props.baseImage})`
    padStyle.backgroundSize = '100%'
  }

  return padStyle
})

const stickStyle = computed(() => {
  const stickColor = props.stickColor
  const stickSize = `${_stickSize.value}px`

  let stickStyle = {
    ...getStickShapeStyle(),
    background: stickColor,
    cursor: 'move',
    height: stickSize,
    width: stickSize,
    border: 'none',
    flexShrink: 0,
    touchAction: 'none',
  } as CSSProperties

  if (props.stickImage) {
    stickStyle.background = `url(${props.stickImage})`
    stickStyle.backgroundSize = '100%'
  }

  if (state.coordinates !== undefined) {
    stickStyle = {
      ...stickStyle,
      position: 'absolute',
      transform: `translate3d(${state.coordinates.relativeX}px, ${state.coordinates.relativeY}px, 0)`,
    }
  }

  return stickStyle
})

/**
 * Update position of joystick - set state and trigger DOM manipulation
 * @param coordinates
 */
const _updatePos = (coordinates: Coordinates) => {
  if (!_isClient) return
  window.requestAnimationFrame(() => (state.coordinates = coordinates))
  if (typeof props.minDistance === 'number' && coordinates.distance < props.minDistance) return
  _throttleMoveCallback({
    type: 'move',
    x: (coordinates.relativeX * 2) / props.size,
    y: -((coordinates.relativeY * 2) / props.size),
    direction: coordinates.direction,
    distance: coordinates.distance,
  })
}

/**
 * Handle pointerdown event
 */
const _onStart = () => {
  if (!_isClient || props.disabled || props.followCursor || !baseRef.value || !stickRef.value) return

  _parentRect.value = baseRef.value.getBoundingClientRect()
  state.dragging = true

  document.addEventListener('mousemove', _onMove, { passive: false })
  document.addEventListener('mouseup', _onEnd, { passive: false })
  document.addEventListener('touchmove', _onMove, { passive: false })
  document.addEventListener('touchend', _onEnd, { passive: false })

  emit('start', { type: 'start' })
}

/**
 * Handle movement event
 * @param e MouseEvent | TouchEvent
 */
const _onMove = (e: MouseEvent | TouchEvent) => {
  if (!state.dragging || !_parentRect.value) return
  e.preventDefault()

  let clientX: number
  let clientY: number

  if (e instanceof TouchEvent) {
    if (e.touches.length === 0) return
    clientX = e.touches[0].clientX
    clientY = e.touches[0].clientY
  } else {
    clientX = e.clientX
    clientY = e.clientY
  }

  let relativeX = clientX - _parentRect.value.left - _radius.value
  let relativeY = clientY - _parentRect.value.top - _radius.value
  const dist = _distance(relativeX, relativeY)

  const bounded = shapeBoundsFactory(
    props.controlPlaneShape || props.baseShape,
    clientX,
    clientY,
    relativeX,
    relativeY,
    dist,
    _radius.value,
    props.size,
    _parentRect.value,
  )

  relativeX = bounded.relativeX
  relativeY = bounded.relativeY
  const atan2 = Math.atan2(relativeX, relativeY)

  _updatePos({
    relativeX,
    relativeY,
    distance: _distanceToPercentile(dist),
    direction: _getDirection(atan2),
    axisX: clientX - _parentRect.value.left,
    axisY: clientY - _parentRect.value.top,
  })
}

/**
 * Handle pointerup event
 * @param e MouseEvent | TouchEvent
 */
const _onEnd = () => {
  state.dragging = false
  if (!props.sticky) state.coordinates = undefined

  document.removeEventListener('mousemove', _onMove)
  document.removeEventListener('mouseup', _onEnd)
  document.removeEventListener('touchmove', _onMove)
  document.removeEventListener('touchend', _onEnd)

  let evt: UpdateEvent = { type: 'stop' }
  if (props.sticky && state.coordinates) {
    evt = {
      ...evt,
      x: (state.coordinates.relativeX * 2) / props.size,
      y: (state.coordinates.relativeY * 2) / props.size,
      direction: state.coordinates.direction,
      distance: state.coordinates.distance,
    }
  }

  emit('stop', evt)
}


/**
 * Use ArcTan2 (4 Quadrant inverse tangent) to identify the direction the joystick is pointing
 * https://docs.oracle.com/cd/B12037_01/olap.101/b10339/x_arcsin003.htm
 * @param atan2: number
 */
const _getDirection = (atan2: number): Direction => {
  if (
    atan2 > RadianQuadrantBinding.TopRight ||
    atan2 < RadianQuadrantBinding.TopLeft
  ) {
    return 'FORWARD'
  }
  if (
    atan2 < RadianQuadrantBinding.TopRight &&
    atan2 > RadianQuadrantBinding.BottomRight
  ) {
    return 'RIGHT'
  }
  if (atan2 < RadianQuadrantBinding.BottomLeft) {
    return 'LEFT'
  }
  return 'BACKWARD'
}

/**
 * Hypotenuse distance calculation
 * @param x: number
 * @param y: number
 */
const _distance = (x: number, y: number): number => {
  return Math.hypot(x, y)
}

const _distanceToPercentile = (distance: number): number => {
  const percentageBaseSize = (distance / (props.size / 2)) * 100
  if (percentageBaseSize > 100) return 100
  return percentageBaseSize
}

/**
 * Calculate X/Y and ArcTan within the bounds of the joystick
 * @param event
 */
const _pointerMove = (event: PointerEvent) => {
  event.preventDefault()
  if (state.dragging) {
    if (!_parentRect.value) return
    if (!props.followCursor && event.pointerId !== _pointerId.value) return
    const absoluteX = event.clientX
    const absoluteY = event.clientY
    let relativeX = absoluteX - _parentRect.value.left - _radius.value
    let relativeY = absoluteY - _parentRect.value.top - _radius.value
    const dist = _distance(relativeX, relativeY)
    const bounded = shapeBoundsFactory(
      props.controlPlaneShape || props.baseShape,
      absoluteX,
      absoluteY,
      relativeX,
      relativeY,
      dist,
      _radius.value,
      props.size,
      _parentRect.value,
    )
    relativeX = bounded.relativeX
    relativeY = bounded.relativeY
    const atan2 = Math.atan2(relativeX, relativeY)

    _updatePos({
      relativeX,
      relativeY,
      distance: _distanceToPercentile(dist),
      direction: _getDirection(atan2),
      axisX: absoluteX - _parentRect.value.left,
      axisY: absoluteY - _parentRect.value.top,
    })
  }
}

/**
 * Handle pointer up and de-register listen events
 */
const _pointerUp = (event: PointerEvent) => {
  if (!_isClient) return
  if (
    event.pointerId !== _pointerId.value &&
    event.type !== InteractionEvents.FollowStop
  ) {
    return
  }

  window.requestAnimationFrame(() => {
    state.dragging = false
    if (!props.sticky) state.coordinates = undefined
  })

  window.removeEventListener(InteractionEvents.PointerUp, _onPointerUp)
  window.removeEventListener(InteractionEvents.PointerMove, _onPointerMove)


  _pointerId.value = undefined

  let evt: UpdateEvent = { type: 'stop' }
  if (props.sticky && state.coordinates) {
    evt = {
      ...evt,
      x: (state.coordinates.relativeX * 2) / props.size,
      y: (state.coordinates.relativeY * 2) / props.size,
      direction: state.coordinates.direction,
      distance: state.coordinates.distance,
    }
  }
  emit('stop', evt)
}

/**
 * Start following
 */
const _followStart = () => {
  if (!_isClient) return
  if (baseRef.value) _parentRect.value = baseRef.value.getBoundingClientRect()
  state.dragging = true
  window.addEventListener(InteractionEvents.PointerMove, _onPointerMove, { passive: false })
  emit('start', { type: 'start' })
}

const _followStop = () => {
  if (!_isClient) return
  window.removeEventListener(InteractionEvents.PointerMove, _onPointerMove)
  _pointerUp(new PointerEvent(InteractionEvents.FollowStop))
}

watch(
  () => props.followCursor,
  () => (props.followCursor ? _followStart() : _followStop()),
  { immediate: true },
)

onMounted(() => {
  baseRef.value.addEventListener('contextmenu', (e) => e.preventDefault())
  stickRef.value.addEventListener('contextmenu', (e) => e.preventDefault())
}),

onBeforeUnmount(() => _followStop())
</script>

<template>
  <div
    ref="baseRef"
    class="joystick"
    :class="{ 'joystick--disabled': props.disabled }"
    :style="baseStyle"
    style="touch-action: none;"
  >
  <button
      ref="stickRef"
      :disabled="props.disabled"
      tabindex="0"
      class="joystick__stick"
      :class="{ 'joystick__stick--disabled': props.disabled }"
      :style="stickStyle"
      @mousedown="_onStart"
      @touchstart="_onStart"
    />
  </div>
</template>

