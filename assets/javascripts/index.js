/* global $ */

const getPosition = event => {
  const e = event || window.event

  const position = {
    x: 0,
    y: 0
  }

  if (e.pageX || e.pageY) {
    position.x = e.pageX
    position.y = e.pageY
  } else if (e.clientX || e.clientY) {
    position.x = e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft
    position.y = e.clientY + document.body.scrollTop + document.documentElement.scrollTop
  }

  return position
}

document.addEventListener('DOMContentLoaded', () => {
  const links = document.querySelectorAll('.ux-Details-link')
  const contextMenu = document.querySelector('.js-ContextMenu')
  const contextMenuDownload = document.querySelector('.js-ContextMenu-download')

  let currentLink = null

  links.forEach(link => {
    link.addEventListener('contextmenu', function (e) {
      e.preventDefault()

      if (currentLink) {
        currentLink.classList.remove('ux-Details-link--hover')
      }

      currentLink = this
      const position = getPosition(e)
      contextMenuDownload.setAttribute('href', `/download/${this.getAttribute('href')}`)
      this.classList.add('ux-Details-link--hover')
      contextMenu.classList.add('ux-Details-contextMenu--visible')
      contextMenu.style.top = `${position.y}px`
      contextMenu.style.left = `${position.x}px`
    })
  })

  document.addEventListener('click', (e) => {
    const isFromContext = $(e.target).closest('.js-ContextMenu').length

    if (!isFromContext && currentLink) {
      contextMenu.classList.remove('ux-Details-contextMenu--visible')
      contextMenu.style = ''
      currentLink.classList.remove('ux-Details-link--hover')
      currentLink = null
    }
  })
})
