function LoadMenu(containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;

  container.innerHTML = `
    <div class="menubar">
      <img class="logo" src="img/logo-emblem.svg" height="50">
      <a href="/">Dashboard</a>
      <a href="/config">Configuration</a>
      <a href="/console">Console</a>
      <a href="/ota">OTA</a>
      <a href="/logout">Logout</a>
    </div>
  `;

  // mark current menu element active
  const links = document.querySelectorAll('.menubar a');
  const current = window.location.pathname;

  links.forEach(link => {
    if (link.getAttribute('href') === current) {
      link.classList.add('active');
    }
  });
}
