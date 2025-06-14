// api.js
const express = require('express');
const multer  = require('multer');
const { spawn } = require('child_process');
const path = require('path');
const fs   = require('fs');

const upload = multer({ dest: 'tmp/' });
const app    = express();
const PORT   = 3000;

// 1) Serve static front-end
app.use(express.static(path.join(__dirname, 'frontend')));
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'frontend', 'index.html'));
});

// Full path to your rebuilt .exe
const exePath = 'C:\\Users\\Harshit\\CLionProjects\\XOR-Cipher-Qt\\cmake-build-debug\\SecureFileEncryptionQt.exe';

// 2) Encrypt endpoint
app.post('/encrypt', upload.single('file'), (req, res) => {
    const key     = req.body.key;
    const inFile  = req.file.path;
    const outFile = inFile + '.enc';

    const proc = spawn(exePath, ['encrypt', inFile, outFile, key], { windowsHide: true });
    proc.on('exit', code => {
        if (code !== 0) return res.sendStatus(500);
        res.download(outFile, req.file.originalname + '.enc', err => {
            fs.unlinkSync(inFile);
            fs.unlinkSync(outFile);
        });
    });
});

// 3) Decrypt endpoint
app.post('/decrypt', upload.single('file'), (req, res) => {
    const inFile  = req.file.path;
    const orig    = req.file.originalname.replace(/\.enc$/i, '');
    const outFile = inFile + '.dec';

    const proc = spawn(exePath, ['decrypt', inFile, outFile], { windowsHide: true });
    proc.on('exit', code => {
        if (code !== 0) return res.sendStatus(500);
        res.download(outFile, orig, err => {
            fs.unlinkSync(inFile);
            fs.unlinkSync(outFile);
        });
    });
});

app.listen(PORT, () => {
    console.log(`Server listening on http://localhost:${PORT}/`);
});
