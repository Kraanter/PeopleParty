<script lang="ts" setup>
import { defineProps, toRefs } from 'vue'
import type { MemoryMixerGrid } from './GridProcessor'

const props = defineProps<{
    grid: MemoryMixerGrid
    isHost: boolean	
}>()

const { grid, isHost } = toRefs(props)



</script>
<template>
    <div>
        <div>
            <div v-for="(row, i) in grid.grid" :key="i">
                <div v-for="(cell, j) in row" :key="j">
                    <div v-if="isHost">
                        {{ cell.icon }}
                    </div>
                    <div v-else>
                        {{ cell.icon }} ({{ cell.players_on_card }})
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>
