import { defineStore } from 'pinia'
import { ref } from 'vue'
import axios from 'axios'

export enum ViewState {
  None,
  PartyPrep,
  MiniGame,
  Leaderboard
}

export const useViewStore = defineStore('view', () => {
  const viewState = ref<ViewState>(ViewState.None)
  const viewData = ref<any>({})

  const versionNumber = ref<string>('')

  async function getVersion() {
    try {
      const data = await axios.get(
        'https://api.github.com/repos/Kraanter/PeopleParty/releases/latest'
      )
      let version: string = data.data?.tag_name || ''
      if (version !== '') {
        if (window.location.host.includes('beta.') || window.location.host.includes('localhost')) {
          const newint = Number(version.split('.')[version.split('.').length - 1]) + 1
          version = version.split('.').slice(0, -1).join('.') + `.${newint}`
          version += '-beta'
        }
      }
      versionNumber.value = version
    } catch (error) {
      console.log(error)
    }
  }
  getVersion()

  function setViewState(state: ViewState, defaultData: any = null) {
    viewState.value = state
    if (defaultData) setViewData(defaultData)
  }

  function setViewData(data: any) {
    viewData.value = data ?? {}
  }

  return {
    versionNumber,
    viewState,
    setViewState,
    setViewData,
    viewData
  }
})
