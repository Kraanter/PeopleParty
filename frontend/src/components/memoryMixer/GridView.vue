<script lang="ts" setup>
import { defineProps, toRefs } from 'vue'
import type { MemoryMixerGrid, PlayerSubmittedData } from './GridProcessor'
import { NCard } from 'naive-ui'
import PartyButton from '../PartyButton.vue'

const props = defineProps<{
    grid: MemoryMixerGrid
    isHost: boolean
    playerSubmitted: PlayerSubmittedData
}>()

const { grid, isHost, playerSubmitted } = toRefs(props)

const emit = defineEmits(['click'])
</script>
<template>
    <div>
        <div class="flex">
            <div v-for="(row, i) in grid.grid" :key="i">
                <div v-for="(cell, j) in row" :key="j">
                    <n-card
                        :style="{ width: '75px', height: '75px' }"
                        :bordered="false"
                        :body-style="{ padding: '' }"
                    >
                    <div v-if="isHost">
                        {{ cell.icon }} <!-- todo: properly display icons -->
                    <!--<p class="text-2xl">{{ cell.players_on_card }}</p>-->
                    </div>
                    <div v-else>
                        <PartyButton @click="emit('click', i, j)" 
                            :disabled="grid.maxOnCard <= cell.players_on_card || playerSubmitted.playerSubmitted"
                            :class="{ 'bg-green-500': playerSubmitted.x === i && playerSubmitted.y === j }"
                        >
                            {{ cell.icon }}
                        </PartyButton>
                    </div>
                    </n-card>
                </div>
            </div>
        </div>
    </div>
</template>
