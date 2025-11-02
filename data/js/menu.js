// Title and Menubar

function LoadTitle(containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;

  container.innerHTML = `
    <h1>1-Wire Interface</h1>
  `;
}

function LoadMenu(containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;

  container.innerHTML = `
    <div class="menubar">
      <a href="/">Dashboard</a>
      <a href="/config">Configuration</a>
      <a href="/ota">OTA Update</a>
      <a href="/console">Console</a>
      <a href="/logout">Logout</a>
    </div>
  `;
}
