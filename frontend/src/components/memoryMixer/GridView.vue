<script lang="ts" setup>
import { computed, defineProps, toRefs } from 'vue'
import type { MemoryMixerGrid, PlayerSubmittedData } from './GridProcessor'
import { NCard } from 'naive-ui'
import PartyButton from '../PartyButton.vue'
import { useWebSocketStore } from '@/stores/confettiStore';
import { VueFlip } from 'vue-flip';

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
    <div>
        <div class="flex">
            <div v-for="(row, i) in grid.grid" :key="i" class="ml-2 mr-2">
                <div v-for="(cell, j) in row" :key="j" class="mb-4">
                    <div v-if="isHost">
                        <n-card
                        :style="{ 
                            width: isGuessPhase? '440px' : '75px', 
                            height: isGuessPhase? '440px' : '75px',
                            backgroundImage: 'url(' + cell.icon + ')',
                            backgroundPosition: 'center',
                            backgroundSize: '70%',
                            backgroundRepeat: 'no-repeat',
                            borderRadius: isGuessPhase? '40px' : '20px'
                        }"
                        :class="[
                                cell.is_highlighted ? ['outline', '-outline-offset-8', 'outline-8', 'outline-green-500'] : '']"
                        >
                        </n-card>
                    </div>
                    <div v-else>
                        <vue-flip :active-hover="false" v-model="isGuessPhase" width="75px" height="75px" transition="1s">
                            <template v-slot:front>
                                <n-card
                                :style="{ 
                                    width: '75px', 
                                    height: '75px',
                                    backgroundImage: 'url(' + cell.icon + ')',
                                    backgroundPosition: 'center',
                                    backgroundSize: '70%',
                                    backgroundRepeat: 'no-repeat',
                                    borderRadius: '20px'
                                }"
                                :class="[
                                    cell.is_highlighted ? ['outline', '-outline-offset-8', 'outline-8', 'outline-green-500'] : playerSubmitted.x == i && playerSubmitted.y == j ? ['outline', '-outline-offset-8', 'outline-8', 'outline-secondary'] : '']"
                                >
                                </n-card>
                            </template>
                            <template v-slot:back>
                                <PartyButton @click="emit('click', i, j)" 
                                :style="{ width: '75px', height: '75px', borderRadius: '20px'}"
                                :disabled="(grid.maxOnCard <= cell.players_on_card 
                                    && (playerSubmitted.x != i || playerSubmitted.y != j)) 
                                    || (playerSubmitted.playerSubmitted 
                                    && (playerSubmitted.x != i || playerSubmitted.y != j)
                                    || eliminatedPlayers.includes(websocketStore.clientName))"
                                :class="[
                                    (playerSubmitted.x == i && playerSubmitted.y == j) ? 'bg-secondary' : 'bg-white', 'text-white']"
                                >
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
</template> 
