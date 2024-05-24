/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{vue,js,ts,jsx,tsx}'],
  theme: {
    extend: {
      animation: {
        'wiggle-slow': 'wiggle 5s none infinite',
        'wiggle-fast': 'wiggle 1s none infinite'
      },
      colors: {
        primary: 'var(--color-primary)',
        'primary-dark': 'var(--color-primary-dark)',
        'primary-light': 'var(--color-primary-light)',
        secondary: 'var(--color-secondary)',
        'secondary-dark': 'var(--color-secondary-dark)',
        'secondary-light': 'var(--color-secondary-light)',
        text: 'var(--color-text)'
      },
      fontFamily: {
        kanit: ['Kanit', 'sans-serif']
      },
      backgroundImage: {
        logo: "url('/assets/logo.svg')"
      }
    }
  },
  plugins: []
}
