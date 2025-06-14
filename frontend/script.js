// Decision-tree structure (mirror your C++ Tree)
const decisionTree = {
    question: "Is file size > 100 KB?",
    check: f => f.size > 100*1024,
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

// Tab switching
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.onclick = () => {
        document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.tab-content').forEach(s=>s.classList.add('hidden'));
        document.getElementById(btn.dataset.tab).classList.remove('hidden');
    };
});

// Encrypt flow
let encFile, encNode, encKey;
const fileE = document.getElementById('fileEncrypt'),
    questions = document.getElementById('questions'),
    goE = document.getElementById('goEncrypt'),
    statusE = document.getElementById('statusEncrypt');

fileE.onchange = () => {
    encFile = fileE.files[0];
    encNode = decisionTree;
    encKey = '';
    renderQ();
    goE.disabled = false;
    statusE.textContent = '';
};

function renderQ(){
    questions.innerHTML = '';
    if (!encNode) return;
    const q = document.createElement('div'); q.className='question';
    q.textContent = encNode.question;
    const y = document.createElement('button'), n = y.cloneNode();
    y.textContent='Yes'; n.textContent='No';
    y.onclick = ()=>answer(true); n.onclick = ()=>answer(false);
    q.append(y,n); questions.append(q);
}

function answer(ans){
    const ok = encNode.check(encFile);
    if ((ans && !ok) || (!ans && ok)) return alert('Wrong, restarting'),encNode=decisionTree,encKey='',renderQ();
    encKey += ans?'1':'0';
    encNode = ans? encNode.yes : encNode.no;
    renderQ();
}

goE.onclick = async () => {
    statusE.textContent = 'Uploading…';
    const fd = new FormData();
    fd.append('file', encFile);
    fd.append('key', encKey);
    const res = await fetch('/encrypt', { method:'POST', body:fd });
    if(!res.ok) return statusE.textContent='Error';
    const blob = await res.blob(), url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href=url; a.download = encFile.name + '.enc'; a.click();
    URL.revokeObjectURL(url);
    statusE.textContent='Done!';
};

// Decrypt flow
let decFile;
const fileD = document.getElementById('fileDecrypt'),
    goD = document.getElementById('goDecrypt'),
    statusD = document.getElementById('statusDecrypt');

fileD.onchange = () => { decFile = fileD.files[0]; goD.disabled=false; statusD.textContent=''; };
goD.onclick = async () => {
    statusD.textContent='Uploading…';
    const fd = new FormData(); fd.append('file', decFile);
    const res = await fetch('/decrypt', { method:'POST', body:fd });
    if(!res.ok) return statusD.textContent='Error';
    const blob = await res.blob();
    let fname = decFile.name.replace(/\.enc$/i,'');
    const cd = res.headers.get('Content-Disposition');
    if(cd){ const m = cd.match(/filename="([^"]+)"/); if(m) fname=m[1]; }
    const url = URL.createObjectURL(blob), a=document.createElement('a');
    a.href=url; a.download=fname; a.click(); URL.revokeObjectURL(url);
    statusD.textContent='Done!';
};
