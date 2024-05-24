/* eslint-disable cypress/no-unnecessary-waiting */
describe('Host test', () => {
  it('Should redirect from root', () => {
    cy.visit('/')

    // check if the url is redirected to /host
    cy.url().should('include', '/host')

    // check if a button with the text 'Host' exists
    cy.get('button').should('contain', 'Host')
  })

  it('Should be able to host a party', () => {
    cy.visit('/host')

    // check if a button with the text 'Host' exists
    cy.get('#partyButton').should('contain', 'Host')
    cy.wait(1000)
    cy.get('#partyButton').click()

    // check if qr code is made and a room code is displayed
    cy.get('#partyPrep').should('exist')
  })
})
