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

// Tab Switching
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b => {
            b.classList.remove('active');
            b.setAttribute('aria-selected','false');
            const p = document.getElementById(b.dataset.tab);
            p.classList.remove('active');
            p.hidden = true;
        });
        btn.classList.add('active');
        btn.setAttribute('aria-selected','true');
        const panel = document.getElementById(btn.dataset.tab);
        panel.classList.add('active');
        panel.hidden = false;
    });
});

// Decision Tree
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

// Drag & Drop Helper
function setupDropZone(zoneId, inputId, callback) {
    const zone  = document.getElementById(zoneId),
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
        if (input.files.length) callback(input.files);
    });
}

// ── ENCRYPT FLOW ──────────────────────────────────────────────
let encFiles = [], encNode, encKey = '';
const qsE    = document.getElementById('questions'),
    qhE    = document.getElementById('questionHeader'),
    btnE   = document.getElementById('goEncrypt'),
    loadE  = document.getElementById('loadingBarEncrypt'),
    statE  = document.getElementById('statusTextEncrypt'),
    infoE  = document.getElementById('fileInfoEncrypt');

setupDropZone('dropZoneEncrypt','fileEncrypt', files => {
    encFiles = Array.from(files);
    encNode = decisionTree;
    encKey  = '';
    infoE.textContent = `${encFiles.length} file(s) selected`;
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
    qhE.textContent = `Question ${encKey.length+1} of 2`;
    const div = document.createElement('div'); div.className = 'question';
    const p   = document.createElement('p');     p.textContent = encNode.question;
    const btns= document.createElement('div');   btns.className = 'answer-buttons';
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
    if (encFiles.some(file => (isYes && !encNode.check(file)) || (!isYes && encNode.check(file)))) {
        alert('Wrong answer—starting over.');
        encNode = decisionTree;
        encKey  = '';
        renderEncryptQ();
        return;
    }
    encKey  += isYes ? '1' : '0';
    encNode  = isYes ? encNode.yes : encNode.no;
    renderEncryptQ();
}

btnE.addEventListener('click', async () => {
    statE.textContent = '';
    loadE.classList.remove('hidden');
    loadE.style.display = 'block';

    try {
        const fd  = new FormData();
        encFiles.forEach(file => fd.append('files', file));
        fd.append('key',  encKey);

        const res = await fetch('/encrypt-multi', { method:'POST', body:fd });
        if (!res.ok) throw new Error(res.statusText);
        const blob = await res.blob(),
            url  = URL.createObjectURL(blob),
            a    = document.createElement('a');
        a.href     = url;
        a.download = 'EncryptedBundle.enc';
        a.click();
        URL.revokeObjectURL(url);
        statE.textContent = '✅ Files encrypted and bundled.';
    } catch (err) {
        statE.textContent = 'Error: ' + err.message;
        console.error(err);
    } finally {
        loadE.style.display = 'none';
    }
});
