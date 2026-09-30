/* eslint-env node */
require('@rushstack/eslint-patch/modern-module-resolution')

module.exports = {
  root: true,
  extends: [
    'plugin:vue/vue3-essential',
    'eslint:recommended',
    '@vue/eslint-config-typescript',
    '@vue/eslint-config-prettier/skip-formatting'
  ],
  parserOptions: {
    ecmaVersion: 'latest'
  },
  rules: {
    'no-restricted-imports': [
      'error',
      {
        paths: [
          {
            name: 'vue3-pixi',
            message: 'vue3-pixi has been removed — use the composables in @/composables/pixi.'
          },
          {
            name: 'pixi.js',
            importNames: ['Application'],
            message:
              'Application must only be constructed by useGameCanvas (@/composables/pixi) — do not import it directly.'
          }
        ]
      }
    ]
  },
  overrides: [
    {
      // useGameCanvas.ts is the one place Application is meant to be constructed.
      files: ['src/composables/pixi/**/*.ts'],
      rules: {
        'no-restricted-imports': 'off'
      }
    }
  ]
}
