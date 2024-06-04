<script lang="ts" setup>
</script>
<template>
    <div class="w-full h-full grid grid-rows-4 grid-cols-1 justify-center">
        <div class=" m-auto row-start-1 row-span-3 h-1/3 mt-2">
            <img
                src="./../../public/assets/info.svg"
            />
        </div>
        <div class="max-w-md m-auto mb-4 row-start-4">
            <router-link to="/join" class="underline mr-4 text-2xl">Join a party!</router-link>
            <router-link to="/host" class="underline ml-4 text-2xl">Host a party!</router-link>
        </div>
    </div>
</template>