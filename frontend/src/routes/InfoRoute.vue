<script lang="ts" setup>
import { isMobile } from '@/util/detectmobilebrowser'
import PartyButton from '@/components/PartyButton.vue'
import { onMounted, ref } from 'vue'
import { NCarousel, NText, NH1 } from 'naive-ui'
import { useReleasesStore } from '@/stores/releasesStore'

const releasesStore = useReleasesStore()

const info = ref<HTMLElement | null>(null)
const scrollToInfo = () => {
  if (info.value) {
    info.value.scrollIntoView({
      behavior: 'smooth'
    })
  }
}

onMounted(() => {
  info.value = document.getElementById('info')
})
</script>

<template>
  <!--<div v-if="isMobile()">
        <div class="h-full w-full grid grid-rows-8 grid-cols-1 justify-center overflow-y-auto">
            <div class="row-start-1 row-span-7 max-h contain-content">
                <img class="h-full w-full" src="./../../public/assets/info-phone.svg" />
            </div>
            <div class="m-auto row-start-8 justify-center text-center">
                <router-link to="/join" class="underline mr-4 text-2xl">Join a party!</router-link>
                <p class="mt-8">{{ versionNumber }}</p>
            </div>
        </div>
    </div>-->
  <div class="container flex flex-col items-center overflow-y-auto">
    <div id="hero" class="min-h-screen">
      <div
        style="
          height: 50%;
          background: url('assets/logo.svg') no-repeat center;
          background-size: 100%;
        "
      ></div>

      <div id="subtitle" class="text-4xl text-center font-bold p-8">
        A fun party game for everyone!
      </div>

      <div id="ctaButtons" class="flex flex-col justify-center items-center p-4">
        <div id="partyButton" class="mb-4">
          <router-link to="/host">
            <PartyButton class="py-8 px-12">Play Now!</PartyButton>
          </router-link>
        </div>
        <div class="text-2xl text-black font-semibold underline mt-4 mb-2">
          <a href="#info" @click="scrollToInfo">Learn more</a>
        </div>
        <svg
          xmlns="http://www.w3.org/2000/svg"
          fill="none"
          viewBox="0 0 24 24"
          stroke-width="2.5"
          stroke="currentColor"
          class="size-7 w-10 mt-1 animate-bounce"
        >
          <path stroke-linecap="round" stroke-linejoin="round" d="m19.5 8.25-7.5 7.5-7.5-7.5" />
        </svg>
      </div>
    </div>
    <div
      id="info"
      class="flex flex-col w-full bg-secondary text-justify font-regular py-16 mt-4 rounded-xl gap-12"
      :class="isMobile() ? 'px-10 text-xl' : 'px-20 text-2xl'"
    >
      <div id="How to Play" class="flex flex-col gap-4">
        <n-h1 prefix="bar">
          <n-text type="black" :class="isMobile() ? '!heading-mobile' : 'heading'">
            🤔 How to Play
          </n-text>
        </n-h1>
        <div class="flex flex-col items-center mb-4">
          <img :class="isMobile() ? 'w-full' : 'w-1/2'" src="assets/devices.png" />
        </div>
        <div>
          People Party is easy to play and requires no special skills or equipment. Simply create a
          party or join an existing one, and you'll be presented with a series of fun and
          interactive challenges. Compete against your friends to earn points and climb the
          leaderboard!
        </div>
        <div>Tip: People Party works best when hosted on a big screen.</div>
      </div>

      <div id="Minigames" class="flex flex-col gap-4">
        <n-h1 prefix="bar">
          <n-text type="black" :class="isMobile() ? '!heading-mobile' : 'heading'">
            👾 Minigames
          </n-text>
        </n-h1>
        <!--TOFIX: prev and next cards not showing, carousel width should be 50%-->
        <div class="flex flex-col items-center">
          <n-carousel
            autoplay
            trigger="hover"
            effect="slide"
            :style="{ width: isMobile() ? '100%' : '50%' }"
            class="mb-4 rounded-xl"
          >
            <img class="carousel-img" src="assets/games/crazyCounting/crazyCountingLogo.svg" />
            <img class="carousel-img" src="assets/games/memoryMixer/memoryMixerLogo.svg" />
            <img class="carousel-img" src="assets/games/businessBailout/businessBailoutLogo.svg" />
            <img class="carousel-img" src="assets/games/launchParty/launchPartyLogo.svg" />
            <img class="carousel-img" src="assets/games/rpsBracket/rpsBracketLogo.svg" />
          </n-carousel>
        </div>
        <div>
          Minigames are the heart of People Party. Each minigame is designed to be quick, easy to
          learn, and fun to play. Whether you're a seasoned gamer or a first-time player, you'll
          find something to enjoy in our ever-expanding collection of minigames. There's something
          for everyone!
        </div>
      </div>

      <div id="Contributors" class="flex flex-col gap-4 mb-8">
        <n-h1 prefix="bar">
          <n-text type="black" :class="isMobile() ? '!heading-mobile' : 'heading'">
            🦸 Contributors
          </n-text>
        </n-h1>
        <div>
          Expandability is also a core part of People Party - our framework is designed for
          minigames to be easily developed and added without changing any core game files.
        </div>
        <div>
          Do you want to help us out and add your own minigame? Check out
          <a
            href="https://github.com/Kraanter/PeopleParty/wiki/How-to-add-a-game"
            target="_blank"
            class="text-primary underline"
            >How to add a game</a
          >
          in our Wiki for more details.
        </div>
        <div class="flex flex-col items-start">
          <a href="https://github.com/Kraanter/PeopleParty" target="_blank">
            <PartyButton class="py-4 px-8 bg-white !text-black !text-2xl !text-semibold"
              >Visit our GitHub</PartyButton
            >
          </a>
        </div>
      </div>

      <div id="About Us" class="flex flex-col gap-4">
        <n-h1 prefix="bar">
          <n-text type="black" :class="isMobile() ? '!heading-mobile' : 'heading'">
            🥳 About Us (the Party People)
          </n-text>
        </n-h1>
        <div>
          People Party was developed by a team of 4 passionate students from the Games Programming
          semester at Windesheim. We believe that games have the power to bring people together and
          create unforgettable experiences. That's why we've dedicated ourselves to creating a
          platform that's easy to use, fun to play, and accessible to everyone. We're constantly
          adding new features and minigames to keep People Party fresh and exciting, so be sure to
          check back often!
        </div>
      </div>

      <p class="flex text-xl">{{ releasesStore.versionNumber }}</p>
    </div>
  </div>
</template>

<style scoped>
.carousel-img {
  margin: 0 auto;
  width: 100%;
  height: 100%;
  object-fit: cover;
}
</style>

<style scoped>
.heading {
  font-size: 60px;
  font-weight: bold;
}
</style>

<style scoped>
.heading-mobile {
  text-align: left;
  font-size: 40px;
  font-weight: bold;
}
</style>
