<script setup lang="ts">
import { Application } from 'vue3-pixi'
import { computed, defineProps, ref, toRefs } from 'vue'
import { Graphics } from 'pixi.js'
import { type BracketMatch, createBracket } from './bracket'
import { GameStateType } from '@/flatbuffers/game-state-type'
import { MiniGameIntroductionPayload } from '@/flatbuffers/mini-game-introduction-payload'
import { RPSBracketHostPayload } from '@/flatbuffers/rpsbracket-host-payload'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import type { IntroductionData } from '../introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'

const props = defineProps<{
  width: number
  height: number
}>()

const { width, height } = toRefs(props)

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const viewState = ref<ViewState>(ViewState.None)

// The single elimination bracket has all matches and the matches have the teams
// The last match is the final, the second to last is the semi-final, etc.
// The first match is the first round of the bracket
// The first match has the first two teams, the second match has the next two teams, etc.
// This means the first half of the matches are the first round, the first half of second half of the matches is the second round, etc.
const bracket = ref()

const xMargin = 0 / 2
const yMargin = 30 / 2
const bracketRows = computed(() =>
  bracket.value.length === 1 ? 1 : Math.ceil(bracket.value.length / 8) * 2
)
const bracketCols = computed(() =>
  bracket.value.length === 1 ? 1 : Math.ceil(Math.log2(bracket.value.length)) * 2 - 1
)

const calcBracketHeight = (rows: number): number => (height.value - yMargin * 2) / rows
const calcBracketWidth = (cols: number): number => (width.value - xMargin * 2) / cols

function getMatchIndex(round: number, row: number, rightSide: boolean = false) {
  const roundStart = Math.floor(bracket.value.length / Math.pow(2, round + 1))
  const roundEnd = Math.floor(bracket.value.length / Math.pow(2, round))
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

  if (i * 2 + 1 >= bracket.value.length) return false // out of bounds = no match

  if (bracket.value[i * 2 + 1]?.left?.name  === '' && bracket.value[i * 2 + 1]?.right?.name  === '' && bracket.value[i * 2 + 1]?.winner?.name !== ''
    && bracket.value[i * 2 + 2]?.left?.name  === '' && bracket.value[i * 2 + 2]?.right?.name  === '' && bracket.value[i * 2 + 2]?.winner?.name !== ''
  ) return false

  return true;
}

function render(graphics: Graphics) {
  graphics.clear()
  graphics.lineStyle(4, 0xffffff)
  const bracketWidth = calcBracketWidth(bracketCols.value)
  // Draw a grid of the cols and rows
  for (let col = 0; col < bracketCols.value; col++) {
    // If right of the middle col, flip
    const flip = isRightSide(col)
    const roundNr = getRoundNumber(col)
    const rowAmount = getRowsInRound(roundNr)
    const currentBracketHeight = calcBracketHeight(rowAmount)
    const BracketHeight = 2 * Math.min(calcBracketHeight(getRowsInRound(getRoundNumber(col))) / 8, height.value / 50)

    for (let row = 0; row < rowAmount; row++) {
      const x = xMargin + col * bracketWidth
      const y = yMargin + row * currentBracketHeight

      const curMatchIndex = getMatchIndex(roundNr, row, flip)
      const match = bracket.value[curMatchIndex];
      
      // draw match lines (last if is special edge case when there are 3 players)
      if (match.left.name !== '' && match.right.name !== '' || col != 0 && col != bracketCols.value - 1 || rowAmount == 1 && bracketCols.value == 3) {
        // draw horizontal lines
        if (col != 0 && (flip || hasPreviousMatch(roundNr, row, flip))) {
          //left side line of bracket (dont if most left col or left side no previous match)
          graphics.moveTo(xMargin + col * bracketWidth, y + currentBracketHeight / 2)
          graphics.lineTo(xMargin + col * bracketWidth + bracketWidth / 8, y + currentBracketHeight / 2)
        }
        if (col != bracketCols.value - 1 && (!flip || hasPreviousMatch(roundNr, row, flip))) {
          // right side line of bracket (dont if most right col or right side no previous match)
          graphics.moveTo(xMargin + (col + 1) * bracketWidth, y + currentBracketHeight / 2)
          graphics.lineTo(xMargin + (col + 1) * bracketWidth - bracketWidth / 8, y + currentBracketHeight / 2)
        }

        // draw vertical lines
        const localCol = flip ? col : col + 1
        if (rowAmount > 1) {
          if (row % 2 == 0) {
            // move line down
            graphics.moveTo(xMargin + localCol * bracketWidth, y + currentBracketHeight / 2)
            graphics.lineTo(xMargin + localCol * bracketWidth, y + currentBracketHeight)
          } else {
            // move line up
            graphics.moveTo(xMargin + localCol * bracketWidth, y + currentBracketHeight / 2)
            graphics.lineTo(xMargin + localCol * bracketWidth, y)
          }
        }

        // draw rectangle arround match
        graphics.drawRect(x + bracketWidth / 8, (y + currentBracketHeight / 2) - BracketHeight, bracketWidth - bracketWidth / 4, BracketHeight * 2)
      }
    }
  }
}

function update(payload: MiniGamePayloadType) {
  switch (payload.gamestatetype()) {
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = payload.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.RPSBracketHost: {
      if (viewState.value === ViewState.Results) break
      viewState.value = ViewState.MiniGame
      const input: RPSBracketHostPayload = payload.gamestatepayload(new RPSBracketHostPayload())

      const matches: BracketMatch[] = []
      for (let i = 0; i < input.matchesLength(); i++) {
        const matchfb = input.matches(i)
        const match = {
          left: {
            name: decodeURI(matchfb?.player1() || '')
          },
          right: {
            name: decodeURI(matchfb?.player2() || '')
          },
          winner: {
            name: decodeURI(matchfb?.winner() || '')
          }
        }
        matches.push(match)
      }
      bracket.value = matches
      break
    }
    default:
      throw new Error(`Unknown gamestatetype: ${payload.gamestatetype()}`)
  }
}

defineExpose({ update })
</script>
<template>
  <template v-if="viewState == ViewState.Introduction">
    <div>
      <Introduction :data="intro" logoSVG="/assets/games/rpsBracket/rpsBracketLogo.svg" />
    </div>
  </template>
  <template v-else-if="viewState == ViewState.MiniGame">
    <div>
      <div
        v-if="bracket[0]?.winner?.name"
        class="absolute text-8xl bg-black/75 z-20 w-full h-full text-center text-secondary"
      >
        <span class="mt-auto">Winner: {{ bracket[0].winner?.name }}</span>
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
              :class="{ 'text-secondary' : 
                bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name 
                == bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name,
                'text-gray-500' : 
                bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name 
                != bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name
                && bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name !== '',
                'text-primary' : bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name === '',
              }"
              :style="{
                gridRowStart: isRightSide(col) ? 3 : 2,
                alignSelf: isRightSide(col) ? 'flex-start' : 'flex-end',
              }"
            >
              {{ bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.left?.name }}
            </p>
            <p
              class="font-bold"
              :class="{ 'text-secondary' : 
                bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right?.name 
                == bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name,
                'text-gray-500' : 
                bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right?.name 
                != bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name
                && bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name,
                'text-primary' : bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.winner?.name === '',
              }"
              :style="{ 
                gridRowStart: isRightSide(col) ? 2 : 3,
                alignSelf: isRightSide(col) ? 'flex-end' : 'flex-start'
              }"
            >
              {{ bracket[getMatchIndex(getRoundNumber(col), row, isRightSide(col))]?.right?.name }}
            </p>
          </span>
        </div>
      </div>
      <Application :width :height>
        <Graphics :x="0" :y="0" @render="render" />
      </Application>
    </div>
  </template>
</template>
