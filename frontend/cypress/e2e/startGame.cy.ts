/* eslint-disable cypress/no-unnecessary-waiting */
describe('Start game test', () => {
    it('Should be able to host and start a game', () => {
      cy.visit('/host')
  
      // check if a button with the text 'Host' exists
      cy.get('#partyButton').should('contain', 'Host')
      cy.wait(1000)
      cy.get('#partyButton').click()
  
      // check if qr code is made and a room code is displayed
      cy.get('#partyPrep').should('exist')

      // click on start game and see if game is running
      cy.get('#startGame').click()

      cy.get('#gameManager').should('exist')
    })
  })
  