<script lang="ts" setup>
import { ref, computed } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import {
  parseShellShuffleHostPayload,
  parseShellShuffleRoundResult,
  parseShellShuffleResult
} from './ShellShuffleProcessor'
import type {
  ShellShuffleRoundResult,
  ShellShuffleResult,
  ShellShuffleResultPair
} from './ShellShuffleModels'
import { ShellShufflePhase } from './ShellShuffleModels'
import ShellShuffleGameView from './ShellShuffleGameView.vue'

const props = defineProps<{
  width: number
  height: number
}>()

// ─── State ───────────────────────────────────────────────────────────────────

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)
const intro = ref<IntroductionData>({ title: '', description: '', time_left: 0 })
const gameViewRef = ref<InstanceType<typeof ShellShuffleGameView>>()

const roundResult = ref<ShellShuffleRoundResult | null>(null)
const gameResult = ref<ShellShuffleResult | null>(null)

// ─── Podium ──────────────────────────────────────────────────────────────────

type PodiumGroup = { placement: number; players: ShellShuffleResultPair[] }

const podium = computed(() => {
  const sorted = gameResult.value?.results.slice().sort((a, b) => a.placement - b.placement) ?? []
  // Group consecutive same-placement entries into distinct groups
  const groups: PodiumGroup[] = []
  for (const r of sorted) {
    const last = groups[groups.length - 1]
    if (last && last.placement === r.placement) {
      last.players.push(r)
    } else {
      groups.push({ placement: r.placement, players: [r] })
    }
  }
  return {
    slots: groups.slice(0, 3), // top 3 distinct placement groups for the podium
    rest: groups.slice(3).flatMap((g) => g.players) // everyone beyond the 3rd distinct group
  }
})

// ─── Ordinal formatting ───────────────────────────────────────────────────────

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n: number) => `${n}${suffixes.get(pr.select(n))}`

// ─── Update handler ───────────────────────────────────────────────────────────

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.ShellShuffleHost: {
      viewState.value = ViewState.MiniGame
      const parsed = parseShellShuffleHostPayload(data)
      // Clear round result overlay when leaving ROUND_RESULT phase
      if (parsed.phase !== ShellShufflePhase.ROUND_RESULT) {
        roundResult.value = null
      }
      gameViewRef.value?.push(parsed)
      break
    }
    case GameStateType.ShellShuffleRoundResult: {
      roundResult.value = parseShellShuffleRoundResult(data)
      break
    }
    case GameStateType.ShellShuffleResult: {
      viewState.value = ViewState.Results
      gameResult.value = parseShellShuffleResult(data)
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const p: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: p.name() || '',
        description: p.instruction() || '',
        time_left: Number(p.timeLeft())
      }
      break
    }
  }
  return []
}

defineExpose({ update })
</script>

<template>
  <template v-if="viewState === ViewState.Introduction">
    <Introduction logoSVG="/assets/games/shellShuffle/shellShuffleLogo.svg" :data="intro" />
  </template>

  <template v-else-if="viewState === ViewState.MiniGame">
    <ShellShuffleGameView ref="gameViewRef" :round-result="roundResult" />
  </template>

  <template v-else-if="viewState === ViewState.Results">
    <div class="flex flex-col w-full h-full bg-black text-white overflow-hidden">
      <!-- Title -->
      <div class="text-center text-4xl font-bold text-yellow-300 pt-5 pb-2 shrink-0">
        Shell Shuffle Results
      </div>

      <!-- Podium area: 2nd (left) — 1st (center) — 3rd (right) -->
      <div class="flex items-end justify-center gap-6 px-12 flex-1 min-h-0">
        <!-- 2nd distinct placement — silver, left -->
        <div v-if="podium.slots[1]" class="flex flex-col items-center">
          <div class="flex flex-col items-center gap-0.5 mb-2 max-w-36">
            <div
              v-for="p in podium.slots[1]?.players ?? []"
              :key="p.name"
              class="text-2xl font-bold text-gray-200 text-center truncate max-w-full"
            >
              {{ p.name }}
            </div>
            <div v-if="!podium.slots[1]" class="text-base text-gray-600">—</div>
          </div>
          <div
            class="w-20 h-24 rounded-b-lg"
            style="background: #9ca3af; clip-path: polygon(15% 0%, 85% 0%, 100% 100%, 0% 100%)"
          ></div>
          <div class="w-24 h-4 rounded" style="background: #6b7280"></div>
          <div
            class="w-32 rounded-t-md flex flex-col items-center justify-end pb-3 gap-1"
            style="height: 80px; background: #374151"
          >
            <div class="text-2xl font-bold text-gray-300">
              {{ podium.slots[1] ? formatOrdinals(podium.slots[1].placement) : '' }}
            </div>
          </div>
        </div>

        <!-- 1st distinct placement — gold, center, tallest -->
        <div v-if="podium.slots[0]" class="flex flex-col items-center">
          <div class="flex flex-col items-center gap-0.5 mb-2 max-w-40">
            <div
              v-for="p in podium.slots[0]?.players ?? []"
              :key="p.name"
              class="text-2xl font-bold text-yellow-300 text-center truncate max-w-full"
            >
              {{ p.name }}
            </div>
            <div v-if="!podium.slots[0]" class="text-lg text-gray-600">—</div>
          </div>
          <div
            class="w-24 h-28 rounded-b-lg"
            style="background: #d97706; clip-path: polygon(15% 0%, 85% 0%, 100% 100%, 0% 100%)"
          ></div>
          <div class="w-28 h-5 rounded" style="background: #92400e"></div>
          <div
            class="w-36 rounded-t-md flex flex-col items-center justify-end pb-3 gap-1"
            style="height: 120px; background: #4b3a14"
          >
            <div class="text-3xl font-bold text-yellow-300">
              {{ podium.slots[0] ? formatOrdinals(podium.slots[0].placement) : '' }}
            </div>
          </div>
        </div>

        <!-- 3rd distinct placement — bronze, right -->
        <div v-if="podium.slots[2]" class="flex flex-col items-center">
          <div class="flex flex-col items-center gap-0.5 mb-2 max-w-36">
            <div
              v-for="p in podium.slots[2]?.players ?? []"
              :key="p.name"
              class="text-2xl font-bold text-amber-600 text-center truncate max-w-full"
            >
              {{ p.name }}
            </div>
            <div v-if="!podium.slots[2]" class="text-base text-gray-600">—</div>
          </div>
          <div
            class="w-20 h-20 rounded-b-lg"
            style="background: #b45309; clip-path: polygon(15% 0%, 85% 0%, 100% 100%, 0% 100%)"
          ></div>
          <div class="w-24 h-4 rounded" style="background: #78350f"></div>
          <div
            class="w-32 rounded-t-md flex flex-col items-center justify-end pb-3 gap-1"
            style="height: 56px; background: #374151"
          >
            <div class="text-2xl font-bold text-amber-700">
              {{ podium.slots[2] ? formatOrdinals(podium.slots[2].placement) : '' }}
            </div>
          </div>
        </div>
      </div>

      <!-- Remaining players (beyond 3rd distinct placement) -->
      <div v-if="podium.rest.length > 0" class="px-8 pb-4 shrink-0 h-max overflow-y-auto">
        <div
          v-for="r in podium.rest"
          :key="r.name"
          class="flex items-center gap-3 border-b border-white border-opacity-10 py-2"
        >
          <span class="text-gray-400 font-bold w-10 text-right">{{
            formatOrdinals(r.placement)
          }}</span>
          <span class="flex-1 text-lg text-gray-200">{{ r.name }}</span>
          <span class="text-sm text-gray-500"
            >{{ r.rounds_survived }} round{{ r.rounds_survived !== 1 ? 's' : '' }}</span
          >
        </div>
      </div>
    </div>
  </template>
</template>
