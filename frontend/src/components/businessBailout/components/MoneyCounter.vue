<script setup lang="ts">
import { NNumberAnimation } from 'naive-ui'
import { ref, watch, defineProps, toRefs } from 'vue'

const props = defineProps<{
  value: number
}>()

const fromValue = ref(0)
const { value } = toRefs(props)

watch(value, (newValue, oldValue) => {
  fromValue.value = oldValue
})
</script>
<template>
  <div class="text-[10vw] z-20 text-green-600 font-extrabold font-mono">
    €
    <n-number-animation show-separator :active="true" :from="fromValue" :to="value" />
  </div>
</template>
