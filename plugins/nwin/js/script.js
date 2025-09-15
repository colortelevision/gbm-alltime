document.addEventListener('DOMContentLoaded', function() {
  // Select DOM elements
  const desktop = document.getElementById('desktop');
  const icons = document.querySelectorAll('.icon');
  const startMenu = document.getElementById('start-menu');
  const startButton = document.querySelector('.start-button');
  const windows = document.querySelectorAll('.window');
  const startMenuItems = document.querySelectorAll('#start-menu li');

  // Add event listeners
  desktop.addEventListener('click', hideStartMenu);
  icons.forEach(icon => {
    icon.addEventListener('click', openWindow);
  });
  startButton.addEventListener('click', showStartMenu);
  windows.forEach(window => {
    window.querySelector('.window-header').addEventListener('mousedown', 
dragWindow);
  });
  startMenuItems.forEach(item => {
    item.addEventListener('click', clickStartMenu);
  });

  // Define functions
  function hideStartMenu() {
    startMenu.style.display = 'none';
  }

  function showStartMenu() {
    startMenu.style.display = 'block';
  }

  function openWindow() {
    const windowId = this.getAttribute('data-window-id');
    const window = document.getElementById(windowId);
    window.style.display = 'block';
  }

  function dragWindow(event) {
    const window = this.parentNode;
    let offsetX = event.clientX - window.offsetLeft;
    let offsetY = event.clientY - window.offsetTop;

    function moveWindow(event) {
      let x = event.clientX - offsetX;
      let y = event.clientY - offsetY;
      window.style.left = x + 'px';
      window.style.top = y + 'px';
    }

    function stopMoving() {
      window.removeEventListener('mousemove', moveWindow);
      window.removeEventListener('mouseup', stopMoving);
    }

    window.addEventListener('mousemove', moveWindow);
    window.addEventListener('mouseup', stopMoving);
  }

  function clickStartMenu(event) {
    console.log(event.target.innerText);
  }
});

