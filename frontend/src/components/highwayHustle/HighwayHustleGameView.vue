<script lang="ts" setup>
import { ref, watch } from 'vue'
import { Container, Graphics, Sprite, Text, TextStyle, CanvasTextMetrics } from 'pixi.js'
import {
  useGameCanvas,
  createEntityLayer,
  useSnapshotBuffer,
  loadTexture
} from '@/composables/pixi'
import {
  getPlayerSprite,
  getPlayerSpriteDimensions,
  getObstacleSprite,
  getObstacleDimensions
} from './HighwayHustleSpriteMap'
import type {
  HighwayHustleEntity,
  HighwayHustleResultPair,
  HighwayHustleSnapshot
} from './HighwayHustleModels'

const props = defineProps<{ snapshot: HighwayHustleSnapshot }>()

// Real backend map size (HighwayHustle_Map(750, 480) — highway_hustle_mini_game.cpp:4), not the
// three mismatched guesses the pre-migration frontend used (800 Application width, 780 internal
// render-math width, 530 height). Car/obstacle sprites would stretch non-uniformly without a
// letterboxed world matching the real map.
const WORLD_WIDTH = 750
const WORLD_HEIGHT = 480
const ROAD_SPACING = 150
const VERTICAL_SPACING = 90

const nameStyle = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 18,
  fill: 'white',
  stroke: { color: 'black', width: 4 }
})
const whiteStyle = new TextStyle({ fill: 'white' })

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n: number) => `${n}${suffixes.get(pr.select(n))}`

// Fixed placement-bracket slot positions on the results screen. Purely a function of the world
// size/spacing constants above — a plain constant, not something the Graphics render needs to
// compute as a side effect (the pre-migration code populated this from inside the road Graphics'
// own render() callback, so the results sprites depended on that Graphics having already drawn
// once — moving it out here removes that hidden, invisible ordering dependency entirely).
const LEADERBOARD_LOCATIONS: { x: number; y: number }[] = Array.from({ length: 12 }, (_, i) => {
  let y = (i + 1) * VERTICAL_SPACING - VERTICAL_SPACING / 2 - 2 * i
  if (y >= WORLD_HEIGHT) y -= WORLD_HEIGHT
  const x = WORLD_WIDTH - (WORLD_WIDTH / 15) * i - 50
  return { x, y }
})

interface IndexedResult extends HighwayHustleResultPair {
  index: number
}

class PlayerDisplay extends Container {
  car = new Sprite()
  explosion = new Sprite()
  nameLabel = new Text({ text: '', style: nameStyle })

  constructor() {
    super()
    this.explosion.width = 50
    this.explosion.height = 50
    this.explosion.visible = false
    this.addChild(this.car, this.explosion, this.nameLabel)
    loadTexture(this.explosion, '/assets/games/highwayHustle/explosion.png', canvas.invalidate)
  }
}

class ResultDisplay extends Container {
  car = new Sprite()
  nameLabel = new Text({ text: '', style: nameStyle })
  ordinalLabel = new Text({ text: '', style: whiteStyle })

  constructor() {
    super()
    this.addChild(this.car, this.nameLabel, this.ordinalLabel)
  }
}

// ─── Canvas + entity layers ────────────────────────────────────────────────

const containerRef = ref<HTMLElement | null>(null)
const roadGfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: WORLD_WIDTH, height: WORLD_HEIGHT },
  backgroundColor: 0x000000,
  backgroundAlpha: 1,
  render: () => drawScene()
})
canvas.root.addChild(roadGfx)

const obstacleLayer = createEntityLayer<HighwayHustleEntity, Sprite>(canvas.root, {
  key: (entity) => entity.id,
  create: () => new Sprite(),
  update: (sprite, entity) => {
    const dims = getObstacleDimensions(entity.carType)
    loadTexture(sprite, getObstacleSprite(entity.carType), canvas.invalidate)
    sprite.width = dims.width * 1.5
    sprite.height = dims.height * 1.5
    sprite.position.set(entity.x || 0, entity.y || 0)
  }
})

const playerLayer = createEntityLayer<HighwayHustleEntity, PlayerDisplay>(canvas.root, {
  key: (entity) => entity.id,
  create: () => new PlayerDisplay(),
  update: (display, entity) => {
    const dims = getPlayerSpriteDimensions(entity.carType)
    const w = dims.width * 1.5
    const h = dims.height * 1.5
    loadTexture(display.car, getPlayerSprite(entity.carType), canvas.invalidate)
    display.car.width = w
    display.car.height = h
    display.position.set(entity.x, entity.y)
    display.explosion.visible = entity.isDead
    display.nameLabel.text = `${entity.id}`
    const metrics = CanvasTextMetrics.measureText(`${entity.id}`, nameStyle)
    display.nameLabel.position.set(w / 2 - metrics.width / 2, -18 * 1.5)
  }
})

const resultLayer = createEntityLayer<IndexedResult, ResultDisplay>(canvas.root, {
  key: (entity) => entity.name,
  create: () => new ResultDisplay(),
  update: (display, entity) => {
    const player = props.snapshot.payload.players.find((a) => a.id === entity.name)
    const dims = getPlayerSpriteDimensions(player?.carType ?? 0)
    const w = dims.width * 1.5
    const h = dims.height * 1.5
    loadTexture(display.car, getPlayerSprite(player?.carType ?? 0), canvas.invalidate)
    display.car.width = w
    display.car.height = h

    const loc = LEADERBOARD_LOCATIONS[entity.index]
    if (!loc) {
      display.car.position.set(-100, 0)
      display.nameLabel.visible = false
      display.ordinalLabel.visible = false
      return
    }
    display.nameLabel.visible = true
    display.ordinalLabel.visible = true

    const carX = loc.x - w - 10
    const carY = loc.y - h / 2
    display.car.position.set(carX, carY)

    const nameMetrics = CanvasTextMetrics.measureText(entity.name, nameStyle)
    display.nameLabel.text = entity.name
    display.nameLabel.position.set(loc.x - nameMetrics.width / 2 - w / 2, carY - 27)

    display.ordinalLabel.text = `${formatOrdinals(entity.placement)}.`
    display.ordinalLabel.position.set(loc.x + 8, loc.y - 15)
  }
})

const buffer = useSnapshotBuffer<HighwayHustleSnapshot>(props.snapshot, {
  invalidate: canvas.invalidate
})

function drawRoad(distance: number, showBracket: boolean) {
  roadGfx.clear()
  const offset = distance < 0 ? 0 : distance % ROAD_SPACING
  for (let y = 0; y <= WORLD_HEIGHT; y += VERTICAL_SPACING) {
    for (let x = -offset; x <= WORLD_WIDTH; x += ROAD_SPACING) {
      roadGfx.moveTo(x, y - 10)
      roadGfx.lineTo(x + ROAD_SPACING / 3, y - 10) // draw dash of half-spacing width
    }
  }
  if (showBracket) {
    for (const loc of LEADERBOARD_LOCATIONS) {
      roadGfx.moveTo(loc.x, loc.y - 20)
      roadGfx.lineTo(loc.x, loc.y + 20)
      roadGfx.moveTo(loc.x, loc.y - 20)
      roadGfx.lineTo(loc.x - 20, loc.y - 20)
      roadGfx.moveTo(loc.x, loc.y + 20)
      roadGfx.lineTo(loc.x - 20, loc.y + 20)
    }
  }
  roadGfx.stroke({ width: 3, color: 0xffffff })
}

function drawScene() {
  const { phase, payload, results } = buffer.current

  drawRoad(payload.distance, phase === 'results')

  if (phase === 'race' && payload.players.length > 0) {
    obstacleLayer.sync(payload.obstacles)
    playerLayer.sync(payload.players)
    resultLayer.clear()
  } else if (phase === 'results' && results.results.length > 0) {
    obstacleLayer.clear()
    playerLayer.clear()
    resultLayer.sync(results.results.map((entity, index) => ({ ...entity, index })))
  } else {
    obstacleLayer.clear()
    playerLayer.clear()
    resultLayer.clear()
  }
}

watch(
  () => props.snapshot,
  (snapshot) => buffer.push(snapshot),
  { immediate: true }
)
</script>

<template>
  <div ref="containerRef" class="w-full h-full flex items-center justify-center"></div>
</template>
