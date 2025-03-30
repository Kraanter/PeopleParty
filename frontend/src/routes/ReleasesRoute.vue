<script lang="ts" setup>
import { useReleasesStore } from '@/stores/releasesStore'
import { NScrollbar } from 'naive-ui'
import { format } from 'date-fns';
import { isMobile } from '@/util/detectmobilebrowser'

const releasesStore = useReleasesStore()

</script>

<template>
    <div>
        <div class="flex h-full w-full justify-center items-center">
            <div class="absolute top-10 left-28 z-10">
                <router-link to="/host" class="flex">
                    <span class="text text-blue-700 text-4xl underline">
                        ← Back
                    </span>
                </router-link>
            </div>
            <NScrollbar class="">
                <div class="flex justify-center items-center mb-4">
                    <div class="text text-6xl mt-6">Latest versions</div>
                </div>
                <div v-for="(release, i) in releasesStore.releases" :key="i" class="h-full w-full flex justify-center items-center flex-col">
                    <div v-if="release.body" :class="isMobile() ? 'w-full' : 'w-2/3 px-10'">
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
                            <div v-if="i == 0" class="flex m-2 mt-6 w-full flex-col text text-lg">
                                <span>Did you find a bug or do you want to suggest a feature or change?</span>
                                <span>
                                    Check out our
                                    <a href="https://github.com/Kraanter/PeopleParty/issues" target="_blank" class="text-primary underline">GitHub</a>
                                    and create an issue!
                                </span>
                            </div>
                        </div>
                    </div>
                    <div v-if="i == 0" class="flex justify-center items-center mb-4">
                        <div class="flex justify-center items-center mb-4">
                            <div class="text text-6xl mt-28">Older versions</div>
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