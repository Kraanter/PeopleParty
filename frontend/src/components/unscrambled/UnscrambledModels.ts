export interface UnscrambledHostData {
  time: number;
  round: number;
  scrambled_word: string;
}

export interface UnscrambledPlayerData {
  time: number;
  round: number;
  words: string[];
  submitted: boolean;
}


// round result
export interface UnscrambledRoundResultPair {
  name: string;
  submitted: boolean;
  guess: number;
}

export interface UnscrambledRoundResults {
  round: number;
  correct: number;
  results: UnscrambledRoundResultPair[]
}

// result
export interface UnscrambledResultPair {
  name: string;
  correct: number;
  placement: number;
}

export interface UnscrambledResults {
  results: UnscrambledResultPair[]
}