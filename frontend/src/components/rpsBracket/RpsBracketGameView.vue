<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { Graphics } from 'pixi.js'
import { useGameCanvas } from '@/composables/pixi'
import type { RpsBracketMatch } from './RpsBracketModels'

const props = defineProps<{ matches: RpsBracketMatch[] }>()

// The single elimination bracket has all matches and the matches have the teams
// The last match is the final, the second to last is the semi-final, etc.
// The first match is the first round of the bracket
// The first match has the first two teams, the second match has the next two teams, etc.
// This means the first half of the matches are the first round, the first half of second half of the matches is the second round, etc.

const xMargin = 0 / 2
const yMargin = 30 / 2

// ─── Canvas ────────────────────────────────────────────────────────────────
// No fixed-aspect "world" here — the bracket grid recomputes its own layout to fill whatever
// rectangle it's given (no circular/proportional shapes that could warp), so this deliberately
// omits `worldSize`/`size` and just trusts the container's own measured box, same as the raw
// width/height props the pre-migration code used directly.

const containerRef = ref<HTMLElement | null>(null)
const gfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  render: () => drawScene()
})
canvas.root.addChild(gfx)

const width = canvas.width
const height = canvas.height

const bracketRows = computed(() =>
  props.matches.length === 1 ? 1 : Math.ceil(props.matches.length / 8) * 2
)
const bracketCols = computed(() =>
  props.matches.length === 1 ? 1 : Math.ceil(Math.log2(props.matches.length)) * 2 - 1
)

const calcBracketHeight = (rows: number): number => (height.value - yMargin * 2) / rows
const calcBracketWidth = (cols: number): number => (width.value - xMargin * 2) / cols

function getMatchIndex(round: number, row: number, rightSide: boolean = false) {
  const roundStart = Math.floor(props.matches.length / Math.pow(2, round + 1))
  const roundEnd = Math.floor(props.matches.length / Math.pow(2, round))
  const roundLength = roundEnd - roundStart
  const roundIndex = roundStart + row
  return rightSide ? roundIndex + roundLength / 2 : roundIndex
}

function isRightSide(col: number) {
  return col > Math.floor(bracketCols.value / 2)
}

function getRoundNumber(col: number) {
  return isRightSide(col) ? bracketCols.value - col - 1 : col
}

function getRowsInRound(round: number) {
  if (bracketCols.value === 3) return 1
  return Math.ceil(bracketRows.value / Math.pow(2, round))
}

function hasPreviousMatch(round: number, row: number, rightSide: boolean = false): boolean {
  const i = getMatchIndex(round, row, rightSide)

  if (i * 2 + 1 >= props.matches.length) return false // out of bounds = no match

  if (
    props.matches[i * 2 + 1]?.left?.name === '' &&
    props.matches[i * 2 + 1]?.right?.name === '' &&
    props.matches[i * 2 + 1]?.winner?.name !== '' &&
    props.matches[i * 2 + 2]?.left?.name === '' &&
    props.matches[i * 2 + 2]?.right?.name === '' &&
    props.matches[i * 2 + 2]?.winner?.name !== ''
  )
    return false

  return true
}

function drawScene() {
  gfx.clear()
  const bracketWidth = calcBracketWidth(bracketCols.value)
  // Draw a grid of the cols and rows
  for (let col = 0; col < bracketCols.value; col++) {
    // If right of the middle col, flip
    const flip = isRightSide(col)
    const roundNr = getRoundNumber(col)
    const rowAmount = getRowsInRound(roundNr)
    const currentBracketHeight = calcBracketHeight(rowAmount)
    const BracketHeight =
      2 * Math.min(calcBracketHeight(getRowsInRound(getRoundNumber(col))) / 8, height.value / 50)

    for (let row = 0; row < rowAmount; row++) {
      const x = xMargin + col * bracketWidth
      const y = yMargin + row * currentBracketHeight

      const curMatchIndex = getMatchIndex(roundNr, row, flip)
      const match = props.matches[curMatchIndex]

      // draw match lines (last if is special edge case when there are 3 players)
      if (
        (match.left.name !== '' && match.right.name !== '') ||
        (col != 0 && col != bracketCols.value - 1) ||
        (rowAmount == 1 && bracketCols.value == 3)
      ) {
        // draw horizontal lines
        if (col != 0 && (flip || hasPreviousMatch(roundNr, row, flip))) {
          //left side line of bracket (dont if most left col or left side no previous match)
          gfx.moveTo(xMargin + col * bracketWidth, y + currentBracketHeight / 2)
          gfx.lineTo(xMargin + col * bracketWidth + bracketWidth / 8, y + currentBracketHeight / 2)
        }
        if (col != bracketCols.value - 1 && (!flip || hasPreviousMatch(roundNr, row, flip))) {
          // right side line of bracket (dont if most right col or right side no previous match)
          gfx.moveTo(xMargin + (col + 1) * bracketWidth, y + currentBracketHeight / 2)
          gfx.lineTo(
            xMargin + (col + 1) * bracketWidth - bracketWidth / 8,
            y + currentBracketHeight / 2
          )
        }

        // draw vertical lines
        const localCol = flip ? col : col + 1
        if (rowAmount > 1) {
          if (row % 2 == 0) {
            // move line down
            gfx.moveTo(xMargin + localCol * bracketWidth, y + currentBracketHeight / 2)
            gfx.lineTo(xMargin + localCol * bracketWidth, y + currentBracketHeight)
          } else {
            // move line up
            gfx.moveTo(xMargin + localCol * bracketWidth, y + currentBracketHeight / 2)
            gfx.lineTo(xMargin + localCol * bracketWidth, y)
          }
        }

        // draw rectangle around match
        gfx.rect(
          x + bracketWidth / 8,
          y + currentBracketHeight / 2 - BracketHeight,
          bracketWidth - bracketWidth / 4,
          BracketHeight * 2
        )
      }
    }
  }
  // All the moveTo/lineTo/rect calls above accumulate as separate subpaths — one trailing
  // stroke() renders every one of them in the same style, equivalent to the original v7
  // lineStyle(4, 0xffffff) staying active for the whole render() call.
  gfx.stroke({ width: 4, color: 0xffffff })
}

watch(
  () => props.matches,
  () => canvas.invalidate(),
  { immediate: true }
)
</script>

<template>
  <div ref="containerRef" class="relative w-full h-full overflow-hidden">
    <div
      v-if="matches[0]?.winner?.name"
      class="absolute text-8xl bg-black/75 z-20 w-full h-full text-center text-secondary"
    >
      <span class="mt-auto">Winner: {{ matches[0].winner?.name }}</span>
    </div>
    <div
      class="absolute h-full w-full grid"
      :style="{
        gridTemplateColumns: `repeat(${bracketCols}, 1fr)`,
        padding: `${yMargin}px ${xMargin}px`
      }"
    >
      <div
        :key="col"
        v-for="(i, col) in bracketCols"
        class="text-center w-full h-full grid px-2"
        :style="{
          gridTemplateRows: `repeat(${getRowsInRound(getRoundNumber(col))}, 1fr)`
        }"
      >
        <span
          :key="row * col"
          v-for="(i, row) in bracketRows"
          class="text-center text-nowrap overflow-hidden w-full h-full grid grid-rows-4"
          :class="{
            hidden: row + 1 > getRowsInRound(getRoundNumber(col))
          }"
          :style="{
            fontSize: `${Math.min(calcBracketHeight(getRowsInRound(getRoundNumber(col))) / 8, height / 50)}px`
          }"
        >
          <p
            class="font-bold"
            :class="{
              'text-secondary':
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name ==
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name,
              'text-gray-500':
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name !=
                  matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner
                    ?.name &&
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name !==
                  '',
              'text-primary':
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name ===
                ''
            }"
            :style="{
              gridRowStart: isRightSide(col) ? 3 : 2,
              alignSelf: isRightSide(col) ? 'flex-start' : 'flex-end'
            }"
          >
            {{ matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name }}
          </p>
          <p
            class="font-bold"
            :class="{
              'text-secondary':
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right?.name ==
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name,
              'text-gray-500':
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right?.name !=
                  matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner
                    ?.name &&
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name,
              'text-primary':
                matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name ===
                ''
            }"
            :style="{
              gridRowStart: isRightSide(col) ? 2 : 3,
              alignSelf: isRightSide(col) ? 'flex-end' : 'flex-start'
            }"
          >
            {{ matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right?.name }}
          </p>
        </span>
      </div>
    </div>
  </div>
</template>
