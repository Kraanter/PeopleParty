<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'

// --color-secondary-dark is a live CSS custom property maintained by colorStore.ts
// regardless of who reads it, so tinting works here with zero store import.
const isHidden = ref(document.hidden)
const handleVisibilityChange = () => {
  isHidden.value = document.hidden
}

onMounted(() => {
  document.addEventListener('visibilitychange', handleVisibilityChange)
})

onUnmounted(() => {
  document.removeEventListener('visibilitychange', handleVisibilityChange)
})
</script>

<template>
  <div
    class="scrolling-background"
    :class="{ 'scrolling-background--paused': isHidden }"
    aria-hidden="true"
  ></div>
</template>

<style scoped>
.scrolling-background {
  --bg-tile-size: 153.6px;
  background-color: var(--color-secondary-dark);
  pointer-events: none;
  -webkit-mask-image: url('/drawing.png');
  mask-image: url('/drawing.png');
  -webkit-mask-repeat: repeat;
  mask-repeat: repeat;
  -webkit-mask-size: var(--bg-tile-size) var(--bg-tile-size);
  mask-size: var(--bg-tile-size) var(--bg-tile-size);
  animation: bg-scroll 5.12s linear infinite;
}

.scrolling-background--paused {
  animation-play-state: paused;
}

@keyframes bg-scroll {
  from {
    -webkit-mask-position: 0 0;
    mask-position: 0 0;
  }
  to {
    -webkit-mask-position: calc(var(--bg-tile-size) * -1) 0;
    mask-position: calc(var(--bg-tile-size) * -1) 0;
  }
}

@media (prefers-reduced-motion: reduce) {
  .scrolling-background {
    animation: none;
  }
}
</style>
