import { defineStore } from 'pinia'
import { ref } from 'vue'

export enum ViewState {
  None,
  PartyPrep,
  MiniGame,
  Leaderboard
}

export const useViewStore = defineStore('view', () => {
  const viewState = ref<ViewState>(ViewState.None)

  function setViewState(state: ViewState) {
    viewState.value = state
  }

  return {
    viewState,
    setViewState
  }
})
