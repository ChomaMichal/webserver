/**
 * POST Request Examples using Fetch API
 * Run from browser console or Node.js with appropriate fetch polyfill
 */

const SERVER_URL = 'http://127.0.0.1:2222';

/**
 * Simple POST request
 */
async function simplePost() {
    try {
        const response = await fetch(`${SERVER_URL}/simple_file.txt`, {
            method: 'POST',
            body: 'This is a simple POST request'
        });
        console.log(`Status: ${response.status}`);
        console.log(`Headers:`, response.headers);
        console.log(await response.text());
    } catch (error) {
        console.error('Error:', error);
    }
}

/**
 * POST with JSON content type
 */
async function jsonPost() {
    try {
        const data = { message: 'Hello', timestamp: new Date().toISOString() };
        const response = await fetch(`${SERVER_URL}/data.json`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        console.log(`Status: ${response.status}`);
        console.log(await response.text());
    } catch (error) {
        console.error('Error:', error);
    }
}

/**
 * POST with plain text
 */
async function textPost() {
    try {
        const content = `
Line 1: Sample text from JavaScript
Line 2: Current time: ${new Date().toISOString()}
Line 3: POST request via fetch API
        `.trim();
        
        const response = await fetch(`${SERVER_URL}/sample.txt`, {
            method: 'POST',
            headers: { 'Content-Type': 'text/plain' },
            body: content
        });
        console.log(`Status: ${response.status}`);
        console.log(await response.text());
    } catch (error) {
        console.error('Error:', error);
    }
}

/**
 * POST with HTML form data
 */
async function formPost() {
    try {
        const formData = new FormData();
        formData.append('field1', 'value1');
        formData.append('field2', 'value2');
        
        const response = await fetch(`${SERVER_URL}/form_data.txt`, {
            method: 'POST',
            body: formData
        });
        console.log(`Status: ${response.status}`);
        console.log(await response.text());
    } catch (error) {
        console.error('Error:', error);
    }
}

/**
 * Multiple POST requests in sequence
 */
async function multiplePosts() {
    const files = [
        { path: '/file1.txt', content: 'First file' },
        { path: '/file2.txt', content: 'Second file' },
        { path: '/file3.txt', content: 'Third file' }
    ];
    
    for (const file of files) {
        try {
            const response = await fetch(`${SERVER_URL}${file.path}`, {
                method: 'POST',
                body: file.content
            });
            console.log(`${file.path}: ${response.status}`);
        } catch (error) {
            console.error(`${file.path}: Error -`, error);
        }
    }
}

// Usage in browser console:
// simplePost();
// jsonPost();
// textPost();
// formPost();
// multiplePosts();
