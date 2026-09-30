document.getElementById('form').addEventListener('submit', function (e) {
  e.preventDefault();

  const form = document.getElementById('form');
  const file = document.getElementById('file').files[0];
  const button = document.getElementById('submit');
  const status = document.getElementById('status');

  if (!file) return;

  status.textContent = 'Uploading firmware… Do not disconnect power.';
  button.disabled = true;

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/update');

  xhr.onload = function () {
    status.textContent = xhr.responseText;
    button.disabled = false;
  };

  xhr.onerror = function () {
    status.textContent = 'Upload failed. Check the device connection.';
    button.disabled = false;
  };

  xhr.send(new FormData(form));
});