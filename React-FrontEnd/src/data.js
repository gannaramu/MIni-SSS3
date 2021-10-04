export const Card = (rank, suit) => { return { rank: rank, suit: suit } }
export const PWMD = (duty, freq, sw) => { return { duty: duty, freq: freq,sw: sw } }
export const PotD = (wiper,sw) => { return { wiper: wiper,sw: sw } }

export const Duty = (value, error, helperText) => { return { value: value, error: error,helperText: helperText } }
export const Freq = (value, error, helperText) => { return { value: value, error: error,helperText: helperText } }
export const SW = (value, meta) => { return { value: value, meta: meta } }

export const Wiper = (value, error, helperText) => { return { value: value, error: error,helperText: helperText } }
