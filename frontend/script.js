// theme toggle
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

// decision tree
const decisionTree = {
    question: "Is file size > 100 KB?",
    check: f => f.size > 100 * 1024,
    yes: {
        question: "Does filename start with a vowel?",
        check: f => /^[AEIOUaeiou]/.test(f.name),
        yes: null, no: null
    },
    no: {
        question: "Is file extension .txt?",
        check: f => f.name.toLowerCase().endsWith('.txt'),
        yes: null, no: null
    }
};

// tab switching
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.panel').forEach(p=>p.classList.remove('active'));
        document.getElementById(btn.dataset.tab).classList.add('active');
    });
});

// drop-zone helper
function setupDropZone(zId, iId, cb) {
    const zone = document.getElementById(zId),
        inp  = document.getElementById(iId);
    zone.addEventListener('dragover', e => { e.preventDefault(); zone.classList.add('dragover'); });
    zone.addEventListener('dragleave', ()    => zone.classList.remove('dragover'));
    zone.addEventListener('drop', e => {
        e.preventDefault(); zone.classList.remove('dragover');
        if (e.dataTransfer.files.length) {
            inp.files = e.dataTransfer.files;
            inp.dispatchEvent(new Event('change'));
        }
    });
    inp.addEventListener('change', () => cb(inp.files[0]));
}

// ENCRYPT flow
let encFile, encNode, encKey;
const questions      = document.getElementById('questions'),
    qHeader        = document.getElementById('questionHeader'),
    goE            = document.getElementById('goEncrypt'),
    loadE          = document.getElementById('loadingBarEncrypt'),
    statusE        = document.getElementById('statusTextEncrypt'),
    infoE          = document.getElementById('fileInfoEncrypt');

setupDropZone('dropZoneEncrypt','fileEncrypt', f => {
    encFile = f; encNode = decisionTree; encKey = '';
    infoE.textContent = `${f.name} (${(f.size/1024).toFixed(1)} KB)`;
    renderQuestion(); statusE.textContent = '';
});

function renderQuestion() {
    questions.innerHTML = '';
    if (!encNode || !encNode.question) {
        qHeader.classList.add('hidden'); goE.disabled = false;
        return;
    }
    goE.disabled = true;
    const depth = encKey.length + 1, total = 2;
    qHeader.textContent = `Question ${depth} of ${total}`;
    qHeader.classList.remove('hidden');

    const qDiv = document.createElement('div'); qDiv.className='question';
    const p    = document.createElement('p'); p.textContent=encNode.question;
    qDiv.append(p);

    const btns = document.createElement('div'); btns.className='answer-buttons';
    ['Yes','No'].forEach(t => {
        const b = document.createElement('button'); b.textContent=t;
        b.onclick = ()=>answer(t==='Yes');
        btns.append(b);
    });
    qDiv.append(btns);
    questions.append(qDiv);
}

function answer(isYes) {
    if ((isYes && !encNode.check(encFile)) ||
        (!isYes &&  encNode.check(encFile))) {
        alert('Incorrect—restarting.');
        encNode = decisionTree; encKey = '';
        renderQuestion(); return;
    }
    encKey += isYes ? '1' : '0';
    encNode = isYes ? encNode.yes : encNode.no;
    renderQuestion();
}

goE.addEventListener('click', async () => {
    statusE.textContent = ''; loadE.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', encFile);
        fd.append('key',  encKey);
        const res = await fetch('/encrypt',{method:'POST',body:fd});
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob(),
            url  = URL.createObjectURL(blob),
            a    = document.createElement('a');
        a.href = url; a.download = encFile.name + '.enc'; a.click();
        URL.revokeObjectURL(url);
        statusE.textContent='Encrypted!';
    } catch(err) {
        console.error(err);
        statusE.textContent='Error: '+err.message;
    } finally {
        loadE.classList.add('hidden');
    }
});

// DECRYPT flow
let decFile;
const goD     = document.getElementById('goDecrypt'),
    loadD   = document.getElementById('loadingBarDecrypt'),
    statusD = document.getElementById('statusTextDecrypt'),
    infoD   = document.getElementById('fileInfoDecrypt');

setupDropZone('dropZoneDecrypt','fileDecrypt', f => {
    decFile = f; statusD.textContent=''; goD.disabled=false;
    infoD.textContent = `${f.name} (${(f.size/1024).toFixed(1)} KB)`;
});

goD.addEventListener('click', async ()=>{
    statusD.textContent=''; loadD.classList.remove('hidden');
    try {
        const fd  = new FormData();
        fd.append('file', decFile);
        const res = await fetch('/decrypt',{method:'POST',body:fd});
        if (!res.ok) throw new Error(`Server ${res.status}`);
        const blob = await res.blob();
        let name = decFile.name.replace(/\.enc$/i,'');
        const cd  = res.headers.get('Content-Disposition');
        if(cd){ const m=cd.match(/filename="([^"]+)"/); if(m) name=m[1]; }
        const url = URL.createObjectURL(blob),
            a   = document.createElement('a');
        a.href    = url; a.download=name; a.click();
        URL.revokeObjectURL(url);
        statusD.textContent='Decrypted!';
    } catch(err) {
        console.error(err);
        statusD.textContent='Error: '+err.message;
    } finally {
        loadD.classList.add('hidden');
    }
});
