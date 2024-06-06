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
  const viewData = ref<any>()

  function setViewState(state: ViewState, defaultData: any = null) {
    viewState.value = state
    if (defaultData) setViewData(defaultData)
  }

  function setViewData(data: any) {
    viewData.value = data
  }

  return {
    viewState,
    setViewState,
    setViewData,
    viewData
  }
})
