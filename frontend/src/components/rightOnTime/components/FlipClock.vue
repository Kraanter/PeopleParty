<template>
  <div class="clock">
    <div class="cards-container">
      <div class="container">
        <div class="card bg-up">
          <div class="inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[0].nextFormat.slice(-1) }}
            </div>
          </div>
        </div>
        <div class="card bg-down">
          <div class="inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[0].currentFormat.slice(-1) }}
            </div>
          </div>
        </div>
        <div
          class="flip card"
          :style="
            computedRealClock[0].ifTens
              ? `transform: rotateX(-${computedRealClock[0].degree}deg);`
              : ''
          "
        >
          <div class="front inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[0].currentFormat.slice(-1) }}
            </div>
          </div>
          <div class="back inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[0].nextFormat.slice(-1) }}
            </div>
          </div>
        </div>
      </div>
      <div class="container">
        <div class="card bg-up">
          <div class="inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[1].nextFormat.slice(-1) }}
            </div>
          </div>
        </div>
        <div class="card bg-down">
          <div class="inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[1].currentFormat.slice(-1) }}
            </div>
          </div>
        </div>
        <div
          class="flip card"
          :style="`transform: rotateX(-${computedRealClock[1].degree}deg);`"
        >
          <div class="front inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[1].currentFormat.slice(-1) }}
            </div>
          </div>
          <div class="back inner" :style="coreStyle">
            <div class="content">
              {{ computedRealClock[1].nextFormat.slice(-1) }}
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: "flip-clock",
  props: {
    innerColor: {
      type: String,
      default: "rgb(41 128 185)",
    },
    number: {
      type: Number,
      required: true,
    },
  },
  data() {
    return {
      realClock: [
        { max: 9, current: 0, degree: 0 }, // Represents the tens place
        { max: 9, current: 0, degree: 0 }, // Represents the units place
      ],
    };
  },
  computed: {
    isFlipping() {
      return this.realClock.some((clock) => clock.degree > 0);
    },
    computedRealClock() {
      const { realClock } = this;
      const numberString = (this.isFlipping ? this.number - 1 : this.number).toString().padStart(2, "0"); // Ensure the number is 2 digits

      return realClock.map((clock, index) => {
        const currentDigit = parseInt(numberString[index]);
        const nextDigit = currentDigit + 1 > clock.max ? 0 : currentDigit + 1;

        return {
          ...clock,
          current: currentDigit,
          nextFormat: `0${nextDigit}`,
          currentFormat: `0${currentDigit}`,
          ifTens: index === 1 ? parseInt(currentDigit / 10) !== parseInt(nextDigit / 10) : this.number % 10 === 0,
        };
      });
    },
    coreStyle() {
      return "background: " + this.innerColor;
    },
  },
  methods: {
    flip(newVal, index = 1) {
      const clock = this.realClock[index];

      if (clock.degree < 180) {
        clock.degree += 4;
        requestAnimationFrame(() => {
          this.flip(newVal, index);
        });
      } else {
        clock.degree = 0;
        clock.current = newVal;
      }
    },
    updateClock() {
      const numberString = this.number.toString().padStart(2, "0"); // Ensure the number is 2 digits

      this.realClock.forEach((clock, index) => {
        const currentDigit = parseInt(numberString[index]);
        if (clock.current !== currentDigit) {
          requestAnimationFrame(() => {
            this.flip(currentDigit, index);
          });
        }
      });
    },
  },
  watch: {
    number() {
      this.updateClock();
    },
  },
  created() {
    this.updateClock();
  },
};
</script>

<style>
.clock {
  display: flex;
  align-items: center;
  justify-content: center;
}

.cards-container {
  display: flex;
  justify-content: center;
  position: relative;
}

.container {
  position: relative;
  width: 30px;
  height: 45px;
  perspective: 200px;
  position: relative;
  border-radius: 4px;
  box-shadow: 0 2px 5px rgb(0 0 0 / 40%);
}

.container::after {
  content: "";
  position: absolute;
  left: 0;
  right: 0;
  height: 1px;
  background: rgb(0 0 0 / 70%);
  box-shadow: 0 1px 2px 0 rgb(0 0 0 / 40%);
  top: calc(50% - 1px);
}

.container + .container {
  margin-left: 5px;
}

.card {
  border-radius: 4px;
  position: absolute;
  font-size: 40px;
  text-align: center;
  line-height: 50px;
  width: 100%;
  height: 50%;
  transform-style: preserve-3d;
  transform-origin: 0% 100%;
}

.inner {
  border-radius: 4px 4px 0 0;
  position: absolute;
  width: 100%;
  height: 100%;
  box-sizing: border-box;
  backface-visibility: hidden;
  overflow: hidden;
}

.inner .content {
  color: white;
  font-weight: bold;
  text-shadow: 1px 2px 2px rgb(34 40 49 / 30%);
  cursor: context-menu;
}

.bg-down {
  top: 50%;
}

.bg-down .inner {
  border-radius: 0 0 4px 4px;
}

.bg-down .inner .content {
  transform: translateY(-48%);
  cursor: context-menu;
}

.back {
  transform: rotateY(180deg);
}

.back .content {
  transform-origin: 50% 47%;
  transform: rotate(180deg);
  cursor: context-menu;
}
</style>