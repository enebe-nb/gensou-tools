import './style/soku-events.scss'
import './component/notifier'
const query = new URLSearchParams(window.location.hash.slice(1));
const barWidth = query.get('bar_width') || config.sokuEvents.bar_width;
const cardsPerLine = query.get('cards_per_line') || config.sokuEvents.cards_per_line;

Vue.component('portrait', {
    data: function() { return {width: barWidth}; },
    props: ['character', 'right'],
    template: require('./template/portrait.html'),
});

Vue.component('card', {
    data: function() {
        return { width: (100.0 / cardsPerLine) +"%" };
    },
    props: ['character', 'card'],
    template: require('./template/card.html'),
    methods: {
        ldInt(value) {
            let s = String(value);
            while (s.length < 3) s = "0" + s;
            return s;
        }
    }
});

async function socketConnect() {
    const socket = new WebSocket('ws://127.0.0.1:6723');
    await new Promise((resolve, reject) => {
        socket.addEventListener('open', resolve, {once: true});
        socket.addEventListener('close', reject, {once: true});
    });
    return socket;
}

async function socketWatch(openCb, closeCb, messageCb) {
    try {
        const socket = await socketConnect();
        if (openCb) openCb();
        if (messageCb) socket.addEventListener('message', messageCb);
        socket.addEventListener('close', () => {
            if (closeCb) closeCb();
            setTimeout(() => socketWatch(openCb, closeCb, messageCb), 1000);            
        });
    } catch(err) {
        setTimeout(() => socketWatch(openCb, closeCb, messageCb), 1000);
    }
}

const characterList = [
    'reimu', 'marisa', 'sakuya', 'alice',
    'patchouli', 'youmu', 'remilia', 'yuyuko',
    'yukari', 'suika', 'reisen', 'aya',
    'komachi', 'iku', 'tenshi', 'sanae',
    'cirno', 'meiling', 'utsuho', 'suwako',
    'namazu', 'none',
]

const options = {
    el: '#main',
    template: require('./template/soku-events.html'),
    data: {
        p1: 'none',
        p2: 'none',
        deck1: [],
        deck2: [],
        width: barWidth,
    },
    mounted() {
        socketWatch(
            () => this.$refs['notifier'].send('Connected!'),
            () => this.$refs['notifier'].send('Disconnected!'),
            (evt) => {
                const data = JSON.parse(evt.data);
                switch(data.type) {
                    case 'battleBegin':
                        this.p1 = characterList[data.left.character];
                        this.p2 = characterList[data.right.character];
                        this.deck1 = data.left.deck;
                        this.deck2 = data.right.deck;
                        break;
                }
            }
        );
    }
}

document.addEventListener('DOMContentLoaded', () => new Vue(options));