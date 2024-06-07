<script setup lang="ts">
import { Application } from 'vue3-pixi'
import { computed, defineProps, ref, toRefs } from 'vue'
import { Graphics } from 'pixi.js'
import { createBracket, type BracketMatch } from './bracket'

const props = defineProps<{
  width: number
  height: number
}>()

const { width, height } = toRefs(props)

// The single elimination bracket has all matches and the matches have the teams
// The last match is the final, the second to last is the semi-final, etc.
// The first match is the first round of the bracket
// The first match has the first two teams, the second match has the next two teams, etc.
// This means the first half of the matches are the first round, the first half of second half of the matches is the second round, etc.
const bracket = ref(createBracket(12))

const xMargin = 0 / 2
const yMargin = 30 / 2
const bracketRows = computed(() => Math.ceil(bracket.value.matches.length / 8) * 2)
const bracketCols = computed(() => Math.ceil(Math.log2(bracket.value.matches.length)) * 2 - 1)

const calcBracketHeight = (rows: number): number => (height.value - yMargin * 2) / rows
const calcBracketWidth = (cols: number): number => (width.value - xMargin * 2) / cols

function getCircleColor(match: BracketMatch, right: boolean) {
  const toCheckProp = right ? 'right' : 'left'
  if (!match) return 0x000000

  if (!match.winner) {
    return match[toCheckProp] ? 0xffffff : 0x000000
  }

  return match.winner === toCheckProp ? 0x00ff00 : 0xff0000
}

function drawMatch(
  g: Graphics,
  match: BracketMatch,
  x: number,
  y: number,
  width: number,
  height: number,
  flip: boolean,
  round: number
) {
  g.lineStyle(4, 0xffffff)
  if (round === 0 && !match.left && !match.right) {
    g.moveTo(x, y + height / 2)
    g.lineTo(x + width, y + height / 2)
    return
  }

  if (flip) {
    ;[x, y, width, height] = [x + width, y + height, -width, -height]
  }
  // top line
  if (round > 0 || match.left) {
    g.moveTo(x, y)
    g.lineTo(x + width, y)
  }

  // connecting line
  g.moveTo(x + width, y)
  g.lineTo(x + width, y + height)

  // bottom line
  if (round > 0 || match.right) {
    g.moveTo(x + width, y + height)
    g.lineTo(x, y + height)
  }

  g.lineStyle(2, 0xffffff)
  g.beginFill(getCircleColor(match, false))
  g.drawCircle(x + width, y, 10)
  g.endFill()
  g.beginFill(getCircleColor(match, true))
  g.drawCircle(x + width, y + height, 10)
  g.endFill()
}

function getMatchIndex(round: number, row: number, rightSide: boolean = false) {
  const roundStart = Math.floor(bracket.value.matches.length / Math.pow(2, round + 1))
  const roundEnd = Math.floor(bracket.value.matches.length / Math.pow(2, round))
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
  return bracketRows.value / Math.pow(2, round)
}

function render(graphics: Graphics) {
  graphics.clear()
  graphics.lineStyle(2, 0xffffff, 1)
  const bracketWidth = calcBracketWidth(bracketCols.value)
  // Draw a grid of the cols and rows
  for (let col = 0; col < bracketCols.value; col++) {
    // If the middle col skip it
    if (col === Math.floor(bracketCols.value / 2)) {
      continue
    }
    // If right of the middle col, flip
    const flip = isRightSide(col)
    const roundNr = getRoundNumber(col)
    const rowAmount = getRowsInRound(roundNr)
    const currentBracketHeight = calcBracketHeight(rowAmount)
    for (let row = 0; row < rowAmount; row++) {
      const curMatchIndex = getMatchIndex(roundNr, row, flip)
      const match = bracket.value.matches[curMatchIndex]
      const x = xMargin + col * bracketWidth
      const y = yMargin + row * currentBracketHeight

      drawMatch(
        graphics,
        match,
        x,
        y + currentBracketHeight / 4,
        bracketWidth,
        currentBracketHeight / 2,
        flip,
        roundNr
      )
    }
  }
}
</script>
<template>
  <div>
    <div
      v-if="bracket.matches[0]?.winner"
      class="absolute text-8xl bg-black/75 z-20 w-full h-full text-center text-secondary"
    >
      <span class="mt-auto">Winner: {{ bracket.matches[0][bracket.matches[0].winner]?.name }}</span>
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
        class="text-white text-center w-full h-full grid px-2"
        :style="{
          gridTemplateRows: `repeat(${getRowsInRound(getRoundNumber(col))}, 1fr)`
        }"
      >
        <span
          :key="row * col"
          v-for="(i, row) in bracketRows"
          class="text-white text-center text-nowrap overflow-hidden w-full h-full grid grid-rows-4"
          :class="{ hidden: row + 1 > getRowsInRound(getRoundNumber(col)) }"
          :style="{
            fontSize: `${Math.min(calcBracketHeight(getRowsInRound(getRoundNumber(col))) / 8, height / 30)}px`,
            rotate: getRoundNumber(col) > 3 ? '90deg' : '0deg'
          }"
        >
          <p
            class="m-auto text-primary font-bold"
            :style="{ gridRowStart: isRightSide(col) ? 3 : 2 }"
          >
            {{
              bracket.matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name
            }}
          </p>
          <p
            class="m-auto text-primary font-bold"
            :style="{ gridRowStart: isRightSide(col) ? 2 : 3 }"
          >
            {{
              bracket.matches[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right
                ?.name
            }}
          </p>
        </span>
      </div>
    </div>
    <Application :width :height>
      <Graphics :x="0" :y="0" @render="render" />
    </Application>
  </div>
</template>
