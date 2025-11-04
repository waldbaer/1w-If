// Common utility functions for the web interface

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
