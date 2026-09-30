export interface CrazyCountingEntity {
  x_pos: number // normalized [0,1]
  y_pos: number // normalized [0,1]
}

export interface CrazyCountingHostData {
  entities: CrazyCountingEntity[]
  time_left: number
  submitted: string[]
}

export interface CrazyCountingResultPair {
  name: string
  guess: number
}

export interface CrazyCountingResult {
  correct_answer: number
  results: CrazyCountingResultPair[]
}
