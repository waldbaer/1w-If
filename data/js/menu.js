function LoadMenu(containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;

  container.innerHTML = `
    <div class="menubar">
      <img class="logo" src="img/logo.png">
      <a href="/">Dashboard</a>
      <a href="/config">Configuration</a>
      <a href="/console">Console</a>
      <a href="/ota">OTA</a>
      <a href="/logout">Logout</a>
    </div>
  `;
}
