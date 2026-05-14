const statusBox = document.getElementById('status');
const requestPathInput = document.getElementById('requestPath');
const requestBodyInput = document.getElementById('requestBody');

function setStatus(message, kind = '') {
    statusBox.className = `status${kind ? ` ${kind}` : ''}`;
    statusBox.textContent = message;
}

function normalizePath(path) {
    if (!path) {
        return '/uploads/demo.txt';
    }
    return path.startsWith('/') ? path : `/${path}`;
}

async function sendPost() {
    const path = normalizePath(requestPathInput.value.trim());
    const body = requestBodyInput.value;

    setStatus(`Sending POST to ${path}...`);

    try {
        const response = await fetch(path, {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body
        });
        const text = await response.text();
        setStatus(
            [
                `POST ${path}`,
                `Status: ${response.status} ${response.statusText}`,
                '',
                'Response body:',
                text || '(empty body)'
            ].join('\n'),
            response.ok ? 'ok' : 'error'
        );
    } catch (error) {
        setStatus(`POST failed: ${error.message}`, 'error');
    }
}

async function sendDelete() {
    const path = normalizePath(requestPathInput.value.trim());
    setStatus(`Sending DELETE to ${path}...`);

    try {
        const response = await fetch(path, {
            method: 'DELETE'
        });
        const text = await response.text();
        setStatus(
            [
                `DELETE ${path}`,
                `Status: ${response.status} ${response.statusText}`,
                '',
                'Response body:',
                text || '(empty body)'
            ].join('\n'),
            response.ok ? 'ok' : 'error'
        );
    } catch (error) {
        setStatus(`DELETE failed: ${error.message}`, 'error');
    }
}

function openRedirect(path) {
    setStatus(`Navigating to ${path} to trigger redirect...`);
    window.location.assign(path);
}

document.getElementById('sendPost').addEventListener('click', sendPost);
document.getElementById('sendDelete').addEventListener('click', sendDelete);
document.getElementById('openRedirect1').addEventListener('click', () => openRedirect('/api/hihi'));
document.getElementById('openRedirect2').addEventListener('click', () => openRedirect('/api2'));
document.getElementById('openHehe').addEventListener('click', () => openRedirect('/hehe'));
document.getElementById('openTests').addEventListener('click', () => openRedirect('/tests/'));

setStatus('Ready. Use POST, DELETE, or redirect buttons to exercise the server.');
