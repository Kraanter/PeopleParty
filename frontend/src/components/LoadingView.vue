<script setup lang="ts">
import { NSpin } from 'naive-ui'
</script>

<template>
    <div class="flex justify-center items-center">   
        <n-space class="lg:mx-44 md:max-w-2/3 2xl:mx-96 m-3">
            <n-spin size="large">
                <template #description>
                    Waiting on server...
                </template>
            </n-spin>
        </n-space>
    </div>
</template>