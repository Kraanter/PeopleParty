/* eslint-disable cypress/no-unnecessary-waiting */
describe('join test', () => {

  it('Should be able to host a party', () => {
    cy.visit('/host')

    // check if a button with the text 'Host' exists
    cy.get('button').should('contain', 'Host')

    cy.get('button').click()

    // check if qr code is made and a room code is displayed
    cy.get('.text-title').should('contain', 'Party code:')
    cy.get('.text-title').should('not.contain', 'false')

    // open a new thread to join the room
    cy.get('.text-title').then(($el) => {
      const roomCode = $el.text().split(' ')[2]
      cy.visit('/join')
      cy.get('input').first().type(roomCode)
      cy.wait(100)
      cy.get('input').first().type("name")
      cy.get('button').click()
      cy.wait(2000)

      cy.get('.max-w-md').children().first().should('contain', 'Joined')
    })
  })
})
