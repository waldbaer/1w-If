// Common utility functions for the web interface

function setupConfigForm() {
  // Handle config form
  document.getElementById('configForm').addEventListener('submit', function (event) {
    event.preventDefault(); // no classic send
    const formData = new FormData(this);

    fetch('/save', {
      method: 'POST',
      body: formData
    })
      .then(response => response.text())
      .then(data => {
        document.getElementById('configSaveStatus').innerText = data;
      })
      .catch(error => {
        document.getElementById('configSaveStatus').innerText = "Error: " + error;
      });
  });
}

// Hardware control - restart
function restartHardware() {
  fetch('/restart')
    .then(response => response.text())
    .then(data => {
      document.getElementById('restartHardwareStatus').innerText = data;
      setTimeout(() => {
        location.reload();
      }, 5000);
    })
    .catch(error => {
      document.getElementById('restartHardwareStatus').innerText = "Error: " + error;
    });
}
