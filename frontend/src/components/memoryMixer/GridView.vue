<script lang="ts" setup>
import { computed, defineProps, toRefs } from 'vue'
import type { MemoryMixerGrid, PlayerSubmittedData } from './GridProcessor'
import { NCard } from 'naive-ui'
import PartyButton from '../PartyButton.vue'
import { useWebSocketStore } from '@/stores/confettiStore'
import { VueFlip } from 'vue-flip'

const websocketStore = useWebSocketStore()

const props = defineProps<{
  grid: MemoryMixerGrid
  isHost: boolean
  playerSubmitted: PlayerSubmittedData
  eliminatedPlayers: string[]
}>()

const { grid, isHost, playerSubmitted, eliminatedPlayers } = toRefs(props)

const isGuessPhase = computed(() => {
  return grid.value.phase == 1 ? true : false
})

const emit = defineEmits(['click'])
</script>

<template>
  <div class="m-auto w-full h-full mt-8">
    <div class="grid grid-cols-5 w-full">
      <div v-for="(row, i) in grid.grid" :key="i" class="mr-3 grid grid-rows-5 h-full">
        <div v-for="(cell, j) in row" :key="j" class="mb-3 h-full w-full">
          <div v-if="isHost">
            <n-card :style="{
              width: isGuessPhase ? '440px' : '90px',
              height: isGuessPhase ? '440px' : '90px',
              backgroundImage: 'url(' + cell.icon + ')',
              backgroundPosition: 'center',
              backgroundSize: '70%',
              backgroundRepeat: 'no-repeat',
              borderRadius: isGuessPhase ? '40px' : '20px'
            }"
              :class="[
                cell.is_highlighted ? ['outline', '-outline-offset-8', 'outline-8', 'outline-green-500'] : '']">
            </n-card>
          </div>
          <div v-else>
            <vue-flip :active-hover="false" v-model="isGuessPhase" width="60px" height="60px" transition="1s">
              <template v-slot:front>
                <n-card :style="{
                  width: '60px',
                  height: '60px',
                  backgroundImage: 'url(' + cell.icon + ')',
                  backgroundPosition: 'center',
                  backgroundSize: '70%',
                  backgroundRepeat: 'no-repeat',
                  borderRadius: '20px'
                }"
                  :class="[
                    cell.is_highlighted ? ['outline', '-outline-offset-8', 'outline-8', 'outline-green-500'] : playerSubmitted.x == i && playerSubmitted.y == j ? ['outline', '-outline-offset-8', 'outline-8', 'outline-secondary'] : '']">
                </n-card>
              </template>
              <template v-slot:back>
                <PartyButton @click="emit('click', i, j)"
                  :style="{ width: '60px', height: '60px', borderRadius: '20px' }" :disabled="(grid.maxOnCard <= cell.players_on_card
                    && (playerSubmitted.x != i || playerSubmitted.y != j))
                    || (playerSubmitted.playerSubmitted
                      && (playerSubmitted.x != i || playerSubmitted.y != j)
                      || eliminatedPlayers.includes(websocketStore.clientName))"
                  :class="[
                    (playerSubmitted.x == i && playerSubmitted.y == j) ? 'bg-secondary' : 'bg-white', 'text-white']">
                  <div v-if="playerSubmitted.x == i && playerSubmitted.y == j">
                    {{ "X" }}
                  </div>
                  <div v-else>
                    {{ "?" }}
                  </div>
                </PartyButton>
              </template>
            </vue-flip>
          </div>
        </div>
      </div>
    </div>
  </div>
  </div>
  </div>
</template>
