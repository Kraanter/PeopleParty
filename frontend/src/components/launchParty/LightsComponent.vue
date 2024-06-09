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
   <div class="lights-container">
    <div
      v-for="index in 4"
      :key="index"
      :class="['light', getLightClass(index)]"
    ></div>
  </div>
</template>


<style scoped>
.lights-container {
  display: flex;
  gap: 10px;
}

.light {
  width: 30px;
  height: 30px;
  border-radius: 50%;
  background-color: gray;
}

.red-light {
  background-color: red;
}

.green-light {
  background-color: green;
}
</style>