// ─── Theme toggle ───────────────────────────────────────────
const themeToggle = document.getElementById('themeToggle');
const savedTheme  = localStorage.getItem('theme') || 'light';
document.documentElement.setAttribute('data-theme', savedTheme);
themeToggle.textContent = savedTheme === 'dark' ? '☀️' : '🌙';
themeToggle.addEventListener('click', () => {
    const newTheme = document.documentElement.getAttribute('data-theme') === 'dark'
        ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', newTheme);
    localStorage.setItem('theme', newTheme);
    themeToggle.textContent = newTheme === 'dark' ? '☀️' : '🌙';
});

// ─── Decision-tree definition ─────────────────────────────────
const decisionTree = {
    question: "Is file size > 100 KB?",
    check: file => file.size > 100 * 1024,
    yes: {
        question: "Does filename start with a vowel?",
        check: file => /^[AEIOUaeiou]/.test(file.name),
        yes: null, no: null
    },
    no: {
        question: "Is file extension .txt?",
        check: file => file.name.toLowerCase().endsWith('.txt'),
        yes: null, no: null
    }
};

// ─── Tab switching ────────────────────────────────────────────
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.panel').forEach(p=>p.classList.remove('active'));
        document.getElementById(btn.dataset.tab).classList.add('active');
    });
});

// ─── Drop-zone helper ─────────────────────────────────────────
function setupDropZone(zoneId, inputId, onFile) {
    const zone = document.getElementById(zoneId),
        inp  = document.getElementById(inputId);
    zone.addEventListener('dragover', e => { e.preventDefault(); zone.classList.add('dragover'); });
    zone.addEventListener('dragleave', ()    => zone.classList.remove('dragover'));
    zone.addEventListener('drop', e => {
        e.preventDefault();
        zone.classList.remove('dragover');
        if (e.dataTransfer.files.length) {
            inp.files = e.dataTransfer.files;
            inp.dispatchEvent(new Event('change'));
        }
    });
    inp.addEventListener('change', () => onFile(inp.files[0]));
}

// ─── ENCRYPT FLOW ────────────────────────────────────────────
let encFile, encNode, encKey;
const questions      = document.getElementById('questions'),
    questionHeader = document.getElementById('questionHeader'),
    goE            = document.getElementById('goEncrypt'),
    loadingE       = document.getElementById('loadingBarEncrypt'),
    statusE        = document.getElementById('statusTextEncrypt'),
    fileInfoEnc    = document.getElementById('fileInfoEncrypt');

setupDropZone('dropZoneEncrypt','fileEncrypt', f => {
    encFile = f;
    encNode = decisionTree;
    encKey  = '';
    renderQuestion();
    statusE.textContent = '';
    // **update file info**
    fileInfoEnc.textContent = `${f.name} (${(f.size/1024).toFixed(1)} KB)`;
});

function renderQuestion() {
    questions.innerHTML = '';
    if (!encNode || !encNode.question) {
        questionHeader.classList.add('hidden');
        goE.disabled = false;
        return;
    }
    goE.disabled = true;
    const depth = encKey.length + 1, total = 2;
    questionHeader.textContent = `Question ${depth} of ${total}`;
    questionHeader.classList.remove('hidden');

    const qDiv = document.createElement('div'); qDiv.className = 'question';
    const p    = document.createElement('p'); p.textContent = encNode.question;
    qDiv.append(p);

    const btns = document.createElement('div'); btns.className = 'answer-buttons';
    ['Yes','No'].forEach(text => {
        const b = document.createElement('button');
        b.textContent = text;
        b.onclick = () => answer(text === 'Yes');
        btns.append(b);
    });
    qDiv.append(btns);
    questions.append(qDiv);
}

function answer(isYes) {
    if ((isYes && !encNode.check(encFile)) ||
        (!isYes &&  encNode.check(encFile))) {
        alert('Incorrect—restarting questions.');
        encNode = decisionTree;
        encKey  = '';
        renderQuestion();
        return;
    }
    encKey  += isYes ? '1' : '0';
    encNode  = isYes ? encNode.yes : encNode.no;
    renderQuestion();
}

goE.addEventListener('click', async () => {
    statusE.textContent = '';
    loadingE.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', encFile);
        fd.append('key',  encKey);
        const res = await fetch('/encrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob(),
            url  = URL.createObjectURL(blob),
            a    = document.createElement('a');
        a.href    = url;
        a.download= encFile.name + '.enc';
        a.click();
        URL.revokeObjectURL(url);
        statusE.textContent = 'Encrypted!';
    } catch (err) {
        console.error(err);
        statusE.textContent = 'Error: ' + err.message;
    } finally {
        loadingE.classList.add('hidden');
    }
});

// ─── DECRYPT FLOW ────────────────────────────────────────────
let decFile;
const goD        = document.getElementById('goDecrypt'),
    loadingD   = document.getElementById('loadingBarDecrypt'),
    statusD    = document.getElementById('statusTextDecrypt'),
    fileInfoDec= document.getElementById('fileInfoDecrypt');

setupDropZone('dropZoneDecrypt','fileDecrypt', f => {
    decFile = f;
    statusD.textContent = '';
    goD.disabled = false;
    // **update file info**
    fileInfoDec.textContent = `${f.name} (${(f.size/1024).toFixed(1)} KB)`;
});

goD.addEventListener('click', async () => {
    statusD.textContent = '';
    loadingD.classList.remove('hidden');
    try {
        const fd = new FormData();
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
        const url = URL.createObjectURL(blob),
            a   = document.createElement('a');
        a.href     = url;
        a.download = name;
        a.click();
        URL.revokeObjectURL(url);
        statusD.textContent = 'Decrypted!';
    } catch (err) {
        console.error(err);
        statusD.textContent = 'Error: ' + err.message;
    } finally {
        loadingD.classList.add('hidden');
    }
});
