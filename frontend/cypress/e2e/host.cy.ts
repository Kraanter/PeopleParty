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
    cy.get('button').should('contain', 'Host')

    cy.get('button').click()

    // check if qr code is made and a room code is displayed
    cy.get('.text-title').should('contain', 'Party code:')
    cy.get('.text-title').should('not.contain', 'false')
  })
})
