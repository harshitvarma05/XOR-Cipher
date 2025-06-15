// ── Theme Toggle ──────────────────────────────────────
const themeToggle = document.getElementById('themeToggle');
const savedTheme  = localStorage.getItem('theme') || 'light';
document.documentElement.setAttribute('data-theme', savedTheme);
themeToggle.textContent = savedTheme === 'dark' ? '☀️' : '🌙';
themeToggle.addEventListener('click', () => {
    const t = document.documentElement.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', t);
    localStorage.setItem('theme', t);
    themeToggle.textContent = t === 'dark' ? '☀️' : '🌙';
});

// ── Decision Tree (unchanged) ─────────────────────────
const decisionTree = {
    question: "Is file size > 100 KB?",
    check: f => f.size > 100 * 1024,
    yes: {
        question: "Does filename start with a vowel?",
        check: f => /^[AEIOUaeiou]/.test(f.name),
        yes: null,
        no: null
    },
    no: {
        question: "Is file extension .txt?",
        check: f => f.name.toLowerCase().endsWith('.txt'),
        yes: null,
        no: null
    }
};

// ── Tab Navigation ────────────────────────────────────
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
        document.getElementById(btn.dataset.tab).classList.add('active');
    });
});

// ── Drag‐&‐Drop Helper ─────────────────────────────────
function setupDropZone(zoneId, inputId, onFile) {
    const zone = document.getElementById(zoneId),
        inp  = document.getElementById(inputId);

    zone.addEventListener('dragover', e => {
        e.preventDefault();
        zone.classList.add('dragover');
    });

    zone.addEventListener('dragleave', () => {
        zone.classList.remove('dragover');
    });

    zone.addEventListener('drop', e => {
        e.preventDefault();
        zone.classList.remove('dragover');
        if (e.dataTransfer.files.length) {
            inp.files = e.dataTransfer.files;
            inp.dispatchEvent(new Event('change'));
        }
    });

    inp.addEventListener('change', () => {
        if (inp.files.length) onFile(inp.files[0]);
    });
}

// ── ENCRYPT Flow ──────────────────────────────────────
let encFile = null, encNode = null, encKey = '';
const questions      = document.getElementById('questions'),
    questionHeader = document.getElementById('questionHeader'),
    goE            = document.getElementById('goEncrypt'),
    loadE          = document.getElementById('loadingBarEncrypt'),
    statusE        = document.getElementById('statusTextEncrypt'),
    infoE          = document.getElementById('fileInfoEncrypt');

setupDropZone('dropZoneEncrypt','fileEncrypt', f => {
    encFile = f;
    encNode = decisionTree;
    encKey  = '';
    infoE.textContent = `${f.name} (${(f.size/1024).toFixed(1)} KB)`;
    questions.innerHTML = '';
    questionHeader.classList.remove('hidden');
    goE.disabled = true;
    statusE.textContent = '';
    renderQuestion();
});

function renderQuestion() {
    questions.innerHTML = '';
    if (!encNode || !encNode.question) {
        questionHeader.classList.add('hidden');
        goE.disabled = false;
        return;
    }
    questionHeader.textContent = `Question ${encKey.length+1} of 2`;

    const div = document.createElement('div');
    div.className = 'question';

    const p = document.createElement('p');
    p.textContent = encNode.question;
    div.appendChild(p);

    const btns = document.createElement('div');
    btns.className = 'answer-buttons';
    ['Yes','No'].forEach(txt => {
        const b = document.createElement('button');
        b.textContent = txt;
        b.addEventListener('click', () => answer(txt === 'Yes'));
        btns.appendChild(b);
    });
    div.appendChild(btns);
    questions.appendChild(div);
}

function answer(isYes) {
    if ((isYes && !encNode.check(encFile)) || (!isYes && encNode.check(encFile))) {
        alert('Incorrect—restarting.');
        encNode = decisionTree;
        encKey  = '';
        renderQuestion();
        return;
    }
    encKey += isYes ? '1' : '0';
    encNode = isYes ? encNode.yes : encNode.no;
    renderQuestion();
}

goE.addEventListener('click', async () => {
    statusE.textContent = '';
    loadE.classList.remove('hidden');
    try {
        const fd = new FormData();
        fd.append('file', encFile);
        fd.append('key', encKey);
        const res = await fetch('/encrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob();
        const url  = URL.createObjectURL(blob);
        const a    = document.createElement('a');
        a.href     = url;
        a.download = `${encFile.name}.enc`;
        a.click();
        URL.revokeObjectURL(url);
        statusE.textContent = 'Encrypted!';
    } catch(err) {
        console.error(err);
        statusE.textContent = 'Error: ' + err.message;
    } finally {
        loadE.classList.add('hidden');
    }
});

// ── DECRYPT Flow ──────────────────────────────────────
let decFile = null;
const goD     = document.getElementById('goDecrypt'),
    loadD   = document.getElementById('loadingBarDecrypt'),
    statusD = document.getElementById('statusTextDecrypt'),
    infoD   = document.getElementById('fileInfoDecrypt');

setupDropZone('dropZoneDecrypt','fileDecrypt', f => {
    decFile = f;
    infoD.textContent = `${f.name} (${(f.size/1024).toFixed(1)} KB)`;
    goD.disabled = false;
    statusD.textContent = '';
});

goD.addEventListener('click', async () => {
    statusD.textContent = '';
    loadD.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', decFile);
        const res = await fetch('/decrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob();
        let name = decFile.name.replace(/\.enc$/i,'');
        const cd = res.headers.get('Content-Disposition');
        if (cd) {
            const m = cd.match(/filename="([^"]+)"/);
            if (m) name = m[1];
        }
        const url = URL.createObjectURL(blob);
        const a   = document.createElement('a');
        a.href     = url;
        a.download = name;
        a.click();
        URL.revokeObjectURL(url);
        statusD.textContent = 'Decrypted!';
    } catch(err) {
        console.error(err);
        statusD.textContent = 'Error: ' + err.message;
    } finally {
        loadD.classList.add('hidden');
    }
});

// ── COMPARE Flow (with keygen + total) ─────────────────────────────
let cmpFile = null;
const dropZoneCompare  = 'dropZoneCompare';
const fileInputCompare = 'fileCompare';
const infoC            = document.getElementById('fileInfoCompare');
const goC              = document.getElementById('goCompare');
const loadC            = document.getElementById('loadingBarCompare');
const statusC          = document.getElementById('statusTextCompare');
const tblCompare       = document.getElementById('compareTable');
const xorEncCell       = document.getElementById('xorEnc');
const xorDecCell       = document.getElementById('xorDec');
const xorTotalCell     = document.getElementById('xorTotal');
const rsaKeygenCell    = document.getElementById('rsaKeygen');
const rsaEncCell       = document.getElementById('rsaEnc');
const rsaDecCell       = document.getElementById('rsaDec');
const rsaTotalCell     = document.getElementById('rsaTotal');

setupDropZone(dropZoneCompare, fileInputCompare, file => {
    cmpFile = file;
    infoC.textContent = `${file.name} (${(file.size/1024).toFixed(1)} KB)`;
    goC.disabled = false;
    statusC.textContent = '';
    tblCompare.classList.add('hidden');
});

goC.addEventListener('click', async () => {
    if (!cmpFile) return;
    goC.disabled = true;
    loadC.classList.remove('hidden');
    statusC.textContent = '';

    try {
        const fd = new FormData();
        fd.append('file', cmpFile);

        const res = await fetch('/compare', {
            method: 'POST',
            body: fd
        });
        if (!res.ok) throw new Error(`Server ${res.status}`);

        const {
            xorEncMs,
            xorDecMs,
            aesrsaKeygenMs,
            aesrsaEncMs,
            aesrsaDecMs
        } = await res.json();

        // compute totals
        const xorTotal = xorEncMs + xorDecMs;
        const rsaTotal = aesrsaKeygenMs + aesrsaEncMs + aesrsaDecMs;

        // populate table
        xorEncCell.textContent    = xorEncMs.toFixed(1);
        xorDecCell.textContent    = xorDecMs.toFixed(1);
        xorTotalCell.textContent  = xorTotal.toFixed(1);

        rsaKeygenCell.textContent = aesrsaKeygenMs.toFixed(1);
        rsaEncCell.textContent    = aesrsaEncMs.toFixed(1);
        rsaDecCell.textContent    = aesrsaDecMs.toFixed(1);
        rsaTotalCell.textContent  = rsaTotal.toFixed(1);

        tblCompare.classList.remove('hidden');
    } catch (err) {
        console.error(err);
        statusC.textContent = 'Error: ' + err.message;
    } finally {
        loadC.classList.add('hidden');
        goC.disabled = false;
    }
});