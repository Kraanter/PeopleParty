export interface RightOnTimeData {
  round: number;
  target: number;
  time: number;
  fade_out: boolean;
  submitted: string[];
}

// round result
export interface RightOnTimeRoundResultPair {
  player: string;
  diff: number;
}

export interface RightOnTimeRoundResults {
  round: number;
  target: number;
  results: RightOnTimeRoundResultPair[]
}

// result
export interface RightOnTimeResultPair {
  name: string;
  round_one_diff: number;
  round_two_diff: number;
  round_three_diff: number;
  average_diff_time: number;
}

export interface RightOnTimeResults {
  results: RightOnTimeResultPair[]
}