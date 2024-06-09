<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import { GameStateType, LaunchPartyLightsPayload, LaunchPartyResultPayload, MiniGameIntroductionPayload, type MiniGamePayloadType } from '@/flatbuffers/messageClass';
import { type LauncPartyResultPair, type LaunchPartyLights, type LaunchPartyResults } from './LaunchPartyProcessor';
import LightsComponent from './LightsComponent.vue'
import { NScrollbar, NCard } from 'naive-ui'

const props = defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const lightsData = ref<LaunchPartyLights>({
    lights: -1,
})

const resultsData = ref<LaunchPartyResults>({
    results: []
})


const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.LaunchPartyLights: {
      viewState.value = ViewState.MiniGame
      const lights: LaunchPartyLightsPayload = data.gamestatepayload(
        new LaunchPartyLightsPayload()
      )

      lightsData.value = {
        lights: Number(lights.lights())
      }
      break
    }
    case GameStateType.LaunchPartyResult: {
      viewState.value = ViewState.Results
      const miniGameResultPayload: LaunchPartyResultPayload = data.gamestatepayload(new LaunchPartyResultPayload())

      const results: LauncPartyResultPair[] = []
      for(let i = 0; i < miniGameResultPayload.minigameResultsLength(); i++) {
        const result = miniGameResultPayload.minigameResults(i)
        results.push({
          player: result.name() || '',
          reaction_time: Number(result.reactionTime())
        })
      }

      resultsData.value = {
        results: results
      }
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
  }
  return []
}

defineExpose({
  update
})
</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/memoryMixer/memoryMixerLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      {{ lightsData.lights }}
      <div v-if="lightsData.lights != -1">
        <LightsComponent :value="lightsData.lights" />
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      <div class="flex flex-col gap-4 w-full h-full">
        <p class="text-4xl w-full text-center text-white mt-4">Final Ranking:</p>
        <n-scrollbar class="-mb-4">
          <div class="grid gap-4">
            <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in resultsData.results" :key="i">
              <n-card>
                <div class="w-full inline-flex justify-between text-2xl px-1">
                  <p class="inline-flex">
                    <span class="w-16">{{ i + 1 }}.</span><span class="font-bold col-span-5">{{ player.player
                      }}</span>
                  </p>
                  <p>
                    <span v-if="player.reaction_time >= 5000 && player.reaction_time < 10000">Too early</span>
                    <span v-else-if="player.reaction_time >= 10000">Too late</span>
                    <span v-else>reaction time: <span class="font-bold">{{ player.reaction_time }}</span>ms</span>
                  </p>
                </div>
              </n-card>
            </div>
          </div>
        </n-scrollbar>
      </div>
    </div>
  </div>
</template>
