// server.js
const express   = require('express');
const multer    = require('multer');
const path      = require('path');
const fs        = require('fs');
const { spawn } = require('child_process');

const upload = multer({ dest: 'tmp/' });
const app    = express();
const PORT   = 3000;

// 1) Path to Qt CLI executable
const CLI = path.resolve(__dirname,
    'cmake-build-debug',
    'SecureFileEncryptionQt.exe'
);
if (!fs.existsSync(CLI)) {
    console.error('ERROR: Cannot find', CLI);
    process.exit(1);
}

// 2) Keys folder next to the EXE
const exeDir  = path.dirname(CLI);
const pubKey  = path.join(exeDir, 'keys', 'public.pem');
const privKey = path.join(exeDir, 'keys', 'private.pem');

// 3) Ensure keys exist (or were generated previously)
if (!fs.existsSync(pubKey) || !fs.existsSync(privKey)) {
    console.error('ERROR: Missing keys in', path.join(exeDir,'keys'));
    process.exit(1);
}

// Helper: run the CLI mode and return stdout as float ms
function runCli(mode, inF, outF, arg) {
    return new Promise((resolve, reject) => {
        const args = [mode, inF, outF];
        if (arg) args.push(arg);

        const p = spawn(CLI, args, { stdio: ['ignore','pipe','pipe'] });
        let out = '', err = '';
        p.stdout.on('data', d => out += d);
        p.stderr.on('data', d => err += d);

        p.on('error', e => reject(e));
        p.on('close', code => {
            if (code !== 0) return reject(new Error(err.trim() || `Exit ${code}`));
            resolve(parseFloat(out) || 0);
        });
    });
}

// Serve static frontend
app.use(express.static('frontend'));
app.get('/', (req, res) =>
    res.sendFile(path.join(__dirname, 'frontend', 'index.html'))
);

// ── XOR Encrypt ───────────────────────────────────────
app.post('/encrypt', upload.single('file'), async (req, res) => {
    const inF  = req.file.path;
    const outF = inF + '.enc';
    const key  = req.body.key || '0';
    try {
        await runCli('encrypt', inF, outF, key);
        res.download(outF, req.file.originalname + '.enc', () => {
            fs.unlinkSync(inF);
            fs.unlinkSync(outF);
        });
    } catch (e) {
        console.error('Encrypt error:', e);
        res.status(500).send(e.message);
    }
});

// ── XOR Decrypt ───────────────────────────────────────
app.post('/decrypt', upload.single('file'), async (req, res) => {
    const inF   = req.file.path;
    const base  = path.basename(req.file.originalname, '.enc');
    const outF  = inF + '.' + base;
    try {
        await runCli('decrypt', inF, outF);
        res.header('Content-Disposition', `attachment; filename="${base}"`)
            .download(outF, base, () => {
                fs.unlinkSync(inF);
                fs.unlinkSync(outF);
            });
    } catch (e) {
        console.error('Decrypt error:', e);
        res.status(500).send(e.message);
    }
});

// ── Compare XOR vs AES+RSA (with key‐gen) ───────────────────────────
app.post('/compare', upload.single('file'), async (req, res) => {
    const tmp     = req.file.path;
    const xorEncF = tmp + '.xor.enc';
    const xorDecF = tmp + '.xor.dec';
    const arEncF  = tmp + '.ar.enc';
    const arDecF  = tmp + '.ar.dec';

    try {
        // 1) XOR timings
        const xorKey    = req.body.key || '0';
        const xorEncMs  = await runCli('encrypt',        tmp,    xorEncF, xorKey);
        const xorDecMs  = await runCli('decrypt',        xorEncF, xorDecF);

        // 2) RSA key‐generation timing
        const aesrsaKeygenMs = await runCli('aesrsa-keygen', pubKey, privKey);

        // 3) AES+RSA wrap/unwrap timings
        const aesrsaEncMs = await runCli('aesrsa-encrypt', tmp, arEncF, pubKey);
        const aesrsaDecMs = await runCli('aesrsa-decrypt', arEncF, arDecF, privKey);

        // Cleanup all temp files
        [tmp, xorEncF, xorDecF, arEncF, arDecF].forEach(f => {
            if (fs.existsSync(f)) fs.unlinkSync(f);
        });

        // Return all timings
        res.json({
            xorEncMs,
            xorDecMs,
            aesrsaKeygenMs,
            aesrsaEncMs,
            aesrsaDecMs
        });
    } catch (e) {
        console.error('Compare error:', e);
        res.status(500).json({ error: e.message });
    }
});

app.listen(PORT, () =>
    console.log(`Server running at http://localhost:${PORT}`)
);
