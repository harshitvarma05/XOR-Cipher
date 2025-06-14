// ─── decision-tree mirror ─────────────────────────────────────
const decisionTree = {
    question: "Is file size > 100 KB?",
    check: file => file.size > 100 * 1024,
    yes: {
        question: "Does filename start with a vowel?",
        check: file => /^[AEIOUaeiou]/.test(file.name),
        yes: null,
        no:  null
    },
    no: {
        question: "Is file extension .txt?",
        check: file => file.name.toLowerCase().endsWith('.txt'),
        yes: null,
        no:  null
    }
};

// ─── tab logic ────────────────────────────────────────────────
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
        document.getElementById(btn.dataset.tab).classList.add('active');
    });
});

// ─── helper to wire up drag/drop zones ───────────────────────
function setupDropZone(zoneId, inputId, infoId) {
    const zone = document.getElementById(zoneId);
    const inp  = document.getElementById(inputId);
    const info = document.getElementById(infoId);

    zone.addEventListener('dragover', e => {
        e.preventDefault();
        zone.classList.add('dragover');
    });
    zone.addEventListener('dragleave', () => zone.classList.remove('dragover'));
    zone.addEventListener('drop', e => {
        e.preventDefault();
        zone.classList.remove('dragover');
        if (e.dataTransfer.files.length) {
            inp.files = e.dataTransfer.files;
            inp.dispatchEvent(new Event('change'));
        }
    });
    inp.addEventListener('change', () => {
        const f = inp.files[0];
        info.textContent = f
            ? `${f.name} (${(f.size/1024).toFixed(1)} KB)`
            : 'No file selected';
    });
}

// Initialize both zones
setupDropZone('dropZoneEncrypt','fileEncrypt','fileInfoEncrypt');
setupDropZone('dropZoneDecrypt','fileDecrypt','fileInfoDecrypt');

// ─── ENCRYPT FLOW ────────────────────────────────────────────
let encFile, encNode, encKey;
const fileE     = document.getElementById('fileEncrypt');
const questions = document.getElementById('questions');
const goE       = document.getElementById('goEncrypt');
const statusE   = document.getElementById('statusTextEncrypt');
const spinnerE  = document.getElementById('spinnerEncrypt');

fileE.addEventListener('change', () => {
    encFile = fileE.files[0];
    encNode = decisionTree;
    encKey  = '';
    renderQuestion();
    goE.disabled = true;      // disable until we finish all questions
    statusE.textContent = '';
});

function renderQuestion() {
    questions.innerHTML = '';
    // If no more questions, allow encrypt
    if (!encNode || !encNode.question) {
        goE.disabled = false;
        return;
    }
    // Still have questions to answer
    goE.disabled = true;

    const qDiv = document.createElement('div');
    qDiv.className = 'question';

    const p = document.createElement('p');
    p.textContent = encNode.question;
    qDiv.appendChild(p);

    const btns = document.createElement('div');
    btns.className = 'answer-buttons';
    ['Yes','No'].forEach(text => {
        const b = document.createElement('button');
        b.textContent = text;
        b.onclick = () => answer(text === 'Yes');
        btns.appendChild(b);
    });
    qDiv.appendChild(btns);

    questions.appendChild(qDiv);
}

function answer(isYes) {
    const correct = encNode.check(encFile);
    if ((isYes && !correct) || (!isYes && correct)) {
        alert('Incorrect answer. Restarting.');
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
    spinnerE.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', encFile);
        fd.append('key',  encKey);
        const res = await fetch('/encrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob();
        const url  = URL.createObjectURL(blob);
        const a    = document.createElement('a');
        a.href     = url;
        a.download = encFile.name + '.enc';
        a.click();
        URL.revokeObjectURL(url);
        statusE.textContent = 'Encrypted!';
    } catch (err) {
        console.error(err);
        statusE.textContent = 'Error: ' + err.message;
    } finally {
        spinnerE.classList.add('hidden');
    }
});

// ─── DECRYPT FLOW ────────────────────────────────────────────
let decFile;
const fileD   = document.getElementById('fileDecrypt');
const goD     = document.getElementById('goDecrypt');
const statusD = document.getElementById('statusTextDecrypt');
const spinnerD= document.getElementById('spinnerDecrypt');

fileD.addEventListener('change', () => {
    decFile = fileD.files[0];
    goD.disabled = false;
    statusD.textContent = '';
});

goD.addEventListener('click', async () => {
    statusD.textContent = '';
    spinnerD.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', decFile);
        const res = await fetch('/decrypt', { method:'POST', body:fd });
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob();
        let name = decFile.name.replace(/\.enc$/i,'');
        const cd  = res.headers.get('Content-Disposition');
        if (cd) {
            const m = cd.match(/filename="([^"]+)"/);
            if (m) name = m[1];
        }
        const url = URL.createObjectURL(blob);
        const a   = document.createElement('a');
        a.href    = url;
        a.download= name;
        a.click();
        URL.revokeObjectURL(url);
        statusD.textContent = 'Decrypted!';
    } catch (err) {
        console.error(err);
        statusD.textContent = 'Error: ' + err.message;
    } finally {
        spinnerD.classList.add('hidden');
    }
});
