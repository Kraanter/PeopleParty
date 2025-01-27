<script lang="ts" setup>
import { useReleasesStore } from '@/stores/releasesStore'
import { NScrollbar } from 'naive-ui'
import { format } from 'date-fns';
import { marked } from 'marked';
import { isMobile } from '@/util/detectmobilebrowser'

const releasesStore = useReleasesStore()

</script>

<template>
    <div>
        <div class="flex h-full w-full justify-center items-center">
            <NScrollbar class="">
                <div class="flex justify-center items-center mb-4">
                    <div class="text text-6xl mt-6">PeopleParty versions</div>
                </div>
                <div v-for="(release, i) in releasesStore.releases" :key="i" class="h-full w-full flex justify-center items-center">
                    <div v-if="release.body" class="" :class="isMobile() ? 'w-full' : 'w-2/3 px-10'">
                        <div class="grid grid-rows m-2 backdrop-blur-xl p-4 rounded-3xl">
                            <div class="grid grid-cols-2 m-2">
                                <div class="text text-3xl">
                                    <span> Release {{ release.tag_name }}</span>
                                </div>
                                <div class="flex items-center justify-end mb-auto" style="">
                                    <span>{{ format(release.date, 'dd MMMM yyyy') }}</span>
                                </div>
                            </div>
                            <div class="mt-2 mb-2" :class="isMobile() ? 'ml-2' : 'ml-8'">
                                <pre class="text text-black">{{ release.body }}</pre>
                            </div>
                        </div>
                    </div>
                </div>
            </NScrollbar>
        </div>
    </div>
</template>

<style scoped>
pre {
    white-space: pre-wrap;       /* Since CSS 2.1 */
    white-space: -moz-pre-wrap;  /* Mozilla, since 1999 */
    white-space: -pre-wrap;      /* Opera 4-6 */
    white-space: -o-pre-wrap;    /* Opera 7 */
    word-wrap: break-word;       /* Internet Explorer 5.5+ */
}
</style>