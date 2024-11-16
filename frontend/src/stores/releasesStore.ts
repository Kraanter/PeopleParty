import { defineStore } from 'pinia'
import { ref } from 'vue'
import axios from 'axios'

export type Release = {
    tag_name: string
    date: Date
    body: string
}

export const useReleasesStore = defineStore('view', () => {
  const versionNumber = ref<string>('')
  const releases = ref<Release[]>([])

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

  async function getReleases() {
    try {
      const data = await axios.get(
        'https://api.github.com/repos/Kraanter/PeopleParty/releases'
      )
      releases.value = data.data.map((release: any) => {
        return {
          tag_name: release.tag_name,
          date: new Date(release.published_at),
          body: release.body
        }
      })
      console.log(releases.value)
    } catch (error) {
      console.log(error)
    }
  }

  getVersion()
  getReleases()

  return {
    versionNumber,
    releases
  }
})
