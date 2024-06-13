<script setup lang="ts">
import { ref, watch, defineProps, toRefs } from 'vue'

const props = defineProps<{
  value: number
}>()

const fromValue = ref(0)
const { value } = toRefs(props)

const getLightClass = (index: number) => {
      if (value.value === 5) {
        return 'green-light';
      } else if (index <= value.value) {
        return 'red-light';
      } else {
        return '';
      }
    }

watch(value, (newValue, oldValue) => {
  fromValue.value = oldValue
})
</script>
<template>
  <div class="m-8">
    <div class="lights-container grid grid-cols-4 justify-center text-center h-full w-full bg-black">
      <div class="m-4 justify-center" v-for="index in 4" :key="index">
        <div class="m-auto" :class="['light', getLightClass(index)]"> </div>
      </div>
   </div>
  </div>
</template>


<style scoped>
.lights-container {
  border: 2px solid black;
  border-radius: 20px;
}

.light {
  border-radius: 50%;
  background-color: gray;
  width: 13vw;
  height: 13vw;
}

.red-light {
  background-color: red;
}

.green-light {
  background-color: green;
}
</style>