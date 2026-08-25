<script setup lang="ts">
const props = defineProps<{
  disabled?: boolean
  btnClass?: string
  cndClass?: any // conditional class like normal :class behavior
}>()

const emit = defineEmits<{
  click: []
}>()

const handleClick = () => {
  if (props.disabled) return
  emit('click')
}
</script>
<template>
  <div @click="handleClick" class="button-wrapper cursor-pointer h-full w-full">
    <!-- style="margin-top: -0.5rem; padding-top: 0.5rem;" -->

    <button
      :disabled="disabled"
      :class="`${btnClass} ${cndClass}`"
      class="bg-primary text-4xl text-white font-bold rounded-2xl w-full h-full disabled:bg-slate-400"
      style="pointer-events: auto"
    >
      <slot></slot>
    </button>
  </div>
</template>
