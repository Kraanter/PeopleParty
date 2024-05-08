FROM cypress/base:14

WORKDIR /app

COPY package.json ./

RUN npm install cypress
RUN npm install typescript
RUN npm install @vue/tsconfig

COPY cypress/ ./cypress/
#COPY src/ .
COPY *.ts ./
COPY *.js ./
COPY *.json ./

RUN npm config rm proxy
RUN npm config rm https-proxy
RUN unset https_proxy http_proxy
RUN unset HTTPS_PROXY HTTP_PROXY

#RUN npx tsc cypress.config.ts

# Run Cypress tests
CMD ["./node_modules/.bin/cypress", "run", "--e2e"]