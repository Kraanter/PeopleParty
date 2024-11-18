<script lang="ts" setup>
import { useReleasesStore } from '@/stores/releasesStore'
import { NScrollbar } from 'naive-ui'
import { format } from 'date-fns';
import { marked } from 'marked';

const releasesStore = useReleasesStore()

</script>

<template>
    <div>
        <div class="flex justify-center items-center">
            <div class="text text-6xl mt-6">PeopleParty versions</div>
        </div>
        <div class="flex h-5/6 w-full justify-center items-center mt-8">
            <NScrollbar class="">
                <div v-for="(release, i) in releasesStore.releases" :key="i" class="h-full w-full flex justify-center items-center">
                    <div v-if="release.body" class="px-10 w-2/3"> <!-- todo: on mobile remove w-2/3 -->
                        <div class="grid grid-rows-4 m-2 backdrop-blur-xl p-4 rounded-3xl">
                            <div class="grid grid-cols-2 ml-1">
                                <div class="text text-3xl">
                                    <span> Release {{ release.tag_name }}</span>
                                </div>
                                <div class="flex items-center justify-end mb-auto" style="">
                                    <span>{{ format(release.date, 'dd MMMM yyyy') }}</span>
                                </div>
                            </div>
                            <div class="row-span-3 mt-1 ml-4 mb-2">
                                <pre>{{ release.body }}</pre> <!-- todo: fix overflow -->
                            </div>
                        </div>
                    </div>
                </div>
            </NScrollbar>
        </div>
    </div>
</template>

