/* eslint-disable cypress/no-unnecessary-waiting */
describe('join test', () => {

  it('Should be able to host a party', () => {
    cy.visit('/host')

    // check if a button with the text 'Host' exists
    cy.get('#partyButton').should('contain', 'Host')

    cy.get('#partyButton').click()

    // check if qr code is made and a room code is displayed
    cy.get('#partyPrep').should('exist')

    cy.get('#partyCode').then(($el) => {
      const roomCode = $el.text()
      cy.visit('/join')
      cy.get('#partyCode').first().type(roomCode)
      cy.wait(100)
      cy.get('#inputName').first().type("name")
      cy.get('#joinButton').click()
      cy.wait(2000)

      cy.get('.max-w-md').children().first().should('contain', 'Joined', { timeout: 10000 })
    })

    
  })
})
