import './style/soku-events.scss'
const query = new URLSearchParams(window.location.hash.slice(1));

let notifierNextId = 0;
Vue.component('notifier', {
    template: require('./template/notifier.html'),
    data: () => ({notifications: []}),
    methods: {
        send(message) {
            this.notifications.push({id: notifierNextId++, message: message});
            setTimeout(() => this.notifications.shift(), 3000);
        }
    }
});

Vue.component('portrait', {
    props: ['character', 'right'],
    template: require('./template/portrait.html'),
});

Vue.component('card', {
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
        ratio: query.get('ratio'),
    },
    mounted() {
        socketWatch(
            () => this.$refs['notifier'].send('Connected!'),
            () => this.$refs['notifier'].send('Disconnected!'),
            (evt) => {
                const data = evt.data.split(' ');
                switch(data[0]) {
                    case 'characters':
                        this.p1 = characterList[parseInt(data[1])];
                        this.p2 = characterList[parseInt(data[2])];
                        break;
                    case 'deck1':
                        this.deck1 = data.slice(1); break;
                    case 'deck2':
                        this.deck2 = data.slice(1); break;
                }
            }
        );
    }
}

document.addEventListener('DOMContentLoaded', () => new Vue(options));