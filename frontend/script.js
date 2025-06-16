// -------------------------------------------------
// script.js — full application logic
// -------------------------------------------------

// Theme Toggle
const themeToggle = document.getElementById('themeToggle');
const savedTheme  = localStorage.getItem('theme') || 'light';
document.documentElement.setAttribute('data-theme', savedTheme);
themeToggle.textContent = savedTheme === 'dark' ? '☀️' : '🌙';
themeToggle.addEventListener('click', () => {
    const next = document.documentElement.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', next);
    localStorage.setItem('theme', next);
    themeToggle.textContent = next === 'dark' ? '☀️' : '🌙';
});

// Generic Tab Switching (now handles Encrypt/Decrypt/Compare/About)
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        // deactivate all tabs
        document.querySelectorAll('.tab-btn').forEach(b => {
            b.classList.remove('active');
            b.setAttribute('aria-selected','false');
            document.getElementById(b.dataset.tab).classList.remove('active');
            document.getElementById(b.dataset.tab).hidden = true;
        });
        // activate this tab
        btn.classList.add('active');
        btn.setAttribute('aria-selected','true');
        const panel = document.getElementById(btn.dataset.tab);
        panel.classList.add('active');
        panel.hidden = false;
    });
});

// Decision‐Tree Definition (same as before)
const decisionTree = {
    question: "Is file size > 100 KB?",
    check: f => f.size > 100 * 1024,
    yes: {
        question: "Does filename start with a vowel?",
        check: f => /^[AEIOUaeiou]/.test(f.name),
        yes: null, no: null
    },
    no: {
        question: "Is extension .txt?",
        check: f => f.name.toLowerCase().endsWith('.txt'),
        yes: null, no: null
    }
};

// Drag & Drop File Selector Helper
function setupDropZone(zoneId, inputId, callback) {
    const zone = document.getElementById(zoneId),
        input = document.getElementById(inputId);

    ['dragover'].forEach(evt =>
        zone.addEventListener(evt, e => {
            e.preventDefault();
            zone.classList.add('dragover');
        })
    );
    zone.addEventListener('dragleave', () => zone.classList.remove('dragover'));
    zone.addEventListener('drop', e => {
        e.preventDefault();
        zone.classList.remove('dragover');
        if (e.dataTransfer.files.length) {
            input.files = e.dataTransfer.files;
            input.dispatchEvent(new Event('change'));
        }
    });
    input.addEventListener('change', () => {
        if (input.files.length) callback(input.files[0]);
    });
}

// ── ENCRYPTION FLOW ─────────────────────────────────────────────
let encFile, encNode, encKey = '';
const qsE    = document.getElementById('questions'),
    qhE    = document.getElementById('questionHeader'),
    btnE   = document.getElementById('goEncrypt'),
    loadE  = document.getElementById('loadingBarEncrypt'),
    statE  = document.getElementById('statusTextEncrypt'),
    infoE  = document.getElementById('fileInfoEncrypt');

setupDropZone('dropZoneEncrypt','fileEncrypt', file => {
    encFile = file;
    encNode = decisionTree;
    encKey  = '';
    infoE.textContent = `${file.name} — ${(file.size/1024).toFixed(1)} KB`;
    qsE.innerHTML     = '';
    qhE.classList.remove('hidden');
    btnE.disabled     = true;
    statE.textContent = '';
    renderEncryptQ();
});

function renderEncryptQ() {
    qsE.innerHTML = '';
    if (!encNode || !encNode.question) {
        qhE.classList.add('hidden');
        btnE.disabled = false;
        return;
    }
    qhE.textContent = `Question ${encKey.length + 1} of 2`;
    const div = document.createElement('div');
    div.className = 'question';
    const p   = document.createElement('p');
    p.textContent = encNode.question;
    const btns = document.createElement('div');
    btns.className = 'answer-buttons';
    ['Yes','No'].forEach(txt => {
        const b = document.createElement('button');
        b.textContent = txt;
        b.addEventListener('click', () => answerEncrypt(txt === 'Yes'));
        btns.appendChild(b);
    });
    div.appendChild(p);
    div.appendChild(btns);
    qsE.appendChild(div);
}

function answerEncrypt(isYes) {
    if ((isYes && !encNode.check(encFile)) ||
        (!isYes &&  encNode.check(encFile))) {
        alert('Wrong answer—restarting questions.');
        encNode = decisionTree;
        encKey  = '';
        renderEncryptQ();
        return;
    }
    encKey += isYes ? '1' : '0';
    encNode  = isYes ? encNode.yes : encNode.no;
    renderEncryptQ();
}

btnE.addEventListener('click', async () => {
    statE.textContent = '';
    loadE.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', encFile);
        fd.append('key',  encKey);
        const res = await fetch('/encrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(res.statusText);
        const blob = await res.blob(),
            url  = URL.createObjectURL(blob),
            a    = document.createElement('a');
        a.href     = url;
        a.download = `${encFile.name}.enc`;
        a.click();
        URL.revokeObjectURL(url);
        statE.textContent = '✅ Encryption complete';
    } catch (err) {
        statE.textContent = 'Error: ' + err.message;
        console.error(err);
    } finally {
        loadE.classList.add('hidden');
    }
});

// ── DECRYPTION FLOW ─────────────────────────────────────────────
let decFile;
const btnD   = document.getElementById('goDecrypt'),
    loadD  = document.getElementById('loadingBarDecrypt'),
    statD  = document.getElementById('statusTextDecrypt'),
    infoD  = document.getElementById('fileInfoDecrypt');

setupDropZone('dropZoneDecrypt','fileDecrypt', file => {
    decFile = file;
    infoD.textContent = `${file.name} — ${(file.size/1024).toFixed(1)} KB`;
    btnD.disabled     = false;
    statD.textContent = '';
});

btnD.addEventListener('click', async () => {
    statD.textContent = '';
    loadD.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', decFile);
        const res = await fetch('/decrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(res.statusText);
        const blob = await res.blob(),
            cd   = res.headers.get('Content-Disposition');
        let filename = decFile.name.replace(/\.enc$/i, '');
        if (cd) {
            const m = cd.match(/filename="([^"]+)"/);
            if (m) filename = m[1];
        }
        const url = URL.createObjectURL(blob),
            a   = document.createElement('a');
        a.href     = url;
        a.download = filename;
        a.click();
        URL.revokeObjectURL(url);
        statD.textContent = '✅ Decryption complete';
    } catch (err) {
        statD.textContent = 'Error: ' + err.message;
        console.error(err);
    } finally {
        loadD.classList.add('hidden');
    }
});

// ── COMPARISON FLOW ─────────────────────────────────────────────
let cmpFile;
const btnC   = document.getElementById('goCompare'),
    loadC  = document.getElementById('loadingBarCompare'),
    statC  = document.getElementById('statusTextCompare'),
    infoC  = document.getElementById('fileInfoCompare'),
    tbl    = document.getElementById('compareTable'),
    xorEnc = document.getElementById('xorEnc'),
    xorDec = document.getElementById('xorDec'),
    xorTot = document.getElementById('xorTotal'),
    rsaKey = document.getElementById('rsaKeygen'),
    rsaEnc = document.getElementById('rsaEnc'),
    rsaDec = document.getElementById('rsaDec'),
    rsaTot = document.getElementById('rsaTotal');

setupDropZone('dropZoneCompare','fileCompare', file => {
    cmpFile = file;
    infoC.textContent = `${file.name} — ${(file.size/1024).toFixed(1)} KB`;
    btnC.disabled     = false;
    statC.textContent = '';
    tbl.classList.add('hidden');
});

btnC.addEventListener('click', async () => {
    btnC.disabled     = true;
    statC.textContent = '';
    loadC.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', cmpFile);
        const res = await fetch('/compare', { method:'POST', body:fd });
        if (!res.ok) throw new Error(res.statusText);
        const {
            xorEncMs, xorDecMs,
            aesrsaKeygenMs, aesrsaEncMs, aesrsaDecMs
        } = await res.json();

        const xorTotal = xorEncMs + xorDecMs;
        const rsaTotal = aesrsaKeygenMs + aesrsaEncMs + aesrsaDecMs;

        xorEnc.textContent = xorEncMs.toFixed(1);
        xorDec.textContent = xorDecMs.toFixed(1);
        xorTot.textContent = xorTotal.toFixed(1);

        rsaKey.textContent = aesrsaKeygenMs.toFixed(1);
        rsaEnc.textContent = aesrsaEncMs.toFixed(1);
        rsaDec.textContent = aesrsaDecMs.toFixed(1);
        rsaTot.textContent = rsaTotal.toFixed(1);

        tbl.classList.remove('hidden');
    } catch (err) {
        statC.textContent = 'Error: ' + err.message;
        console.error(err);
    } finally {
        loadC.classList.add('hidden');
        btnC.disabled = false;
    }
});
