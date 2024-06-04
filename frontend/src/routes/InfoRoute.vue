<script lang="ts" setup>
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { NCard } from 'naive-ui'
</script>
<template>
    <div class="w-full h-full grid grid-rows-3 grid-cols-1 justify-center">
        <div class="row-start-1">
            <PeoplePartyLogo/>
            <!-- <p class="text-4xl"> A fun multiplayer party game</p> -->
        </div>
        <div class=" m-auto row-start-2 w-1/2">
            <n-card>
                <p class="text-2xl">People Party is a fun multiplayer party game where you can join or host a party and play with your friends!</p>
                <p class="text-2xl mt-4">The host screen will be the center screen where players can join on their phones!</p>
            </n-card>
        </div>
        <div class="max-w-md m-auto row-start-3">
            <router-link to="/join" class="underline mr-4 text-2xl">Join a party!</router-link>
            <router-link to="/host" class="underline ml-4 text-2xl">Host a party!</router-link>
        </div>
    </div>
</template>