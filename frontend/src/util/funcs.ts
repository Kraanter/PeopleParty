export const debounce = (func: Function, wait: number) => {
  let timeout: number | undefined
  return (...args: any[]) => {
    clearTimeout(timeout)
    timeout = setTimeout(() => {
      func(...args)
    }, wait)
  }
}

export function formatMilliseconds(miliseconds: number) {
  const minutes = Math.floor(miliseconds / 60000)
  const seconds = ((miliseconds % 60000) / 1000).toFixed(0)
  return `${minutes}:${Number(seconds) < 10 ? '0' : ''}${seconds}`
}

export function unLerp(start: number, end: number, t: number) {
  return (t - start) / (end - start)
}

export function lerp(start: number, end: number, t: number) {
  return start * (1 - t) + end * t
}
