<script lang="ts" setup>
import { isMobile } from '@/util/detectmobilebrowser'
</script>
<template>
    <div v-if="isMobile()">
        <div class="w-full h-full grid grid-rows-4 grid-cols-1 justify-center">
            <div class="m-auto row-start-1 row-span-3 w-full mt-16">
                <img src="./../../public/assets/info-phone.svg" />
            </div>
            <div class="m-auto mb-8 row-start-4">
                <router-link to="/join" class="underline mr-4 text-2xl">Join a party!</router-link> 
            </div>
        </div>
    </div>
    <div v-else>
        <div class="w-full h-full grid grid-rows-4 grid-cols-1 justify-center">
            <div class="m-auto row-start-1 row-span-3 w-2/3 mt-16">
                <img src="./../../public/assets/info-desktop.svg" />
            </div>
            <div class="m-auto mb-8 row-start-4">
                <router-link to="/join" class="underline mr-16 text-4xl">Join a party!</router-link>
                <router-link to="/host" class="underline ml-16 text-4xl">Host a party!</router-link>
            </div>
        </div>
    </div>
</template>