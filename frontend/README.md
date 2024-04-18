# frontend

This template should help get you started developing with Vue 3 in Vite.

## Dependencies

Make sure you have the following dependencies installed on your machine:

- [Bun](https://bun.sh/)

## Project Setup

```sh
bun install
```

### Compile and Hot-Reload for Development

```sh
bun dev
```

### Type-Check, Compile and Minify for Production

```sh
bun build
```

### Run Headed Component Tests with [Cypress Component Testing](https://on.cypress.io/component)

```sh
bun test:unit:dev # or `bun test:unit` for headless testing
```

### Run End-to-End Tests with [Cypress](https://www.cypress.io/)

```sh
bun test:e2e:dev
```

This runs the end-to-end tests against the Vite development server.
It is much faster than the production build.

But it's still recommended to test the production build with `test:e2e` before deploying (e.g. in CI environments):

```sh
bun build
bun test:e2e
```

### Lint with [ESLint](https://eslint.org/)

```sh
bun lint
```
