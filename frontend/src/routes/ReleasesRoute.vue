<script lang="ts" setup>
import { useReleasesStore } from '@/stores/releasesStore'
import { NDivider, NScrollbar } from 'naive-ui'
import { format } from 'date-fns';
import { marked } from 'marked';

const releasesStore = useReleasesStore()


</script>

<template>
 <div class="flex h-full w-full justify-center items-center mt-8">
    <NScrollbar>
        <div v-for="(release, i) in releasesStore.releases" :key="i" class="h-full w-full flex justify-center items-center">
            <div v-if="release.body" class="px-10 w-2/3">
                <n-divider v-if="i != 0"/>
                <div class="grid grid-rows-2 m-2">
                    <div class="grid grid-cols-2">
                        <div class="text text-3xl">
                            <span> Release {{ release.tag_name }}</span>
                        </div>
                        <div class="flex items-center justify-end">
                            <span>{{ format(release.date, 'dd MMMM yyyy') }}</span>
                        </div>
                    </div>
                    <div>
                        <div v-html="marked(release.body)"></div>
                    </div>
                </div>
            </div>
        </div>
    </NScrollbar>
 </div>
</template>

