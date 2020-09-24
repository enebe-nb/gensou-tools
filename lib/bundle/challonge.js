import './style/challonge.scss'
import './component/notifier'
import EventEmitter from 'events'

class Tournament extends EventEmitter {
    constructor(apiKey, id) {
        super();
        this.apiKey = apiKey;
        this.id = id;
        setTimeout(() => this.updateLoop(), 1);
    }

    async updateLoop() {
        const query = new URLSearchParams({
            api_key: this.apiKey,
            include_participants: 1,
            include_matches: 1,
        });
        const result = await fetch('https://api.challonge.com/v1/tournaments/'+this.id+'.json?'+query)
        switch (result.status) {
            case 401: return this.emit('error', 'Invalid authentication, check your Api Key.');
            case 404: return this.emit('error', 'Tournament not found, check its id.');
            case 200: this.update((await result.json()).tournament); break;
            case 500: console.warn(result.status, result.statusText); break;
            default: return console.error(result.status, result.statusText);
        }
        setTimeout(() => this.updateLoop(), 5000);
    }

    update(data) {
        this.participants = data.participants;
        if (this.lastState == 'pending' && data.state == 'underway') {
            this.emit('started', {
                id: data.url,
                name: data.name,
                gameName: data.game_name,
            });
        }

        if (this.lastMatches) for (let match of data.matches) {
            const lastMatch = this.lastMatches.find((m) => m.match.id == match.match.id);
            if (lastMatch) this.updateMatch(match.match, lastMatch.match);
        }

        this.lastState = data.state;
        this.lastMatches = data.matches;
    }

    updateMatch(data, last) {
        if (data.underway_at && !last.underway_at) {
            this.emit('matchStarted', {
                id: data.id,
                round: data.round,
                player1: this.participants.find((p) => p.participant.id == data.player1_id).participant.name,
                player2: this.participants.find((p) => p.participant.id == data.player2_id).participant.name,
            })
        }

        if (data.state == 'complete' && last.state != 'complete') {
            this.emit('matchComplete', {
                id: data.id,
                round: data.round,
                player1: this.participants.find((p) => p.participant.id == data.player1_id).participant.name,
                player2: this.participants.find((p) => p.participant.id == data.player2_id).participant.name,
                forfeited: data.forfeited,
                scores: data.scores_csv,
                isPlayer1Winner: data.winner_id == data.player1_id
            });
        }
    }
}

Vue.component('tournament-started', {
    props: ['event'],
    template: require('./template/tournament-started.html'),
});

Vue.component('match-started', {
    props: ['event'],
    template: require('./template/match-started.html'),
});

Vue.component('match-complete', {
    props: ['event'],
    template: require('./template/match-complete.html'),
});

let eventNextId = 0;
const query = new URLSearchParams(window.location.hash.slice(1));
const options = {
    el: '#main',
    template: require('./template/challonge.html'),
    data: {
        eventSfx: null,
        eventQueue: [],
    },

    mounted() {
        const tournament = new Tournament(query.get('api_key'), query.get('tournament'));
        tournament.on('error', (err) => this.$refs['notifier'].send(err));
        tournament.on('started', (data) => this.dispatchEvent('started', data));
        tournament.on('matchStarted', (data) => this.dispatchEvent('matchStarted', data));
        tournament.on('matchComplete', (data) => this.dispatchEvent('matchComplete', data));
        this.eventSfx = new Audio('./sfx/challonge.wav');
        this.eventSfx.addEventListener("error", console.error);
    },

    methods: {
        dispatchEvent(type, data) {
            console.log(type, data);
            this.eventQueue.push({id: eventNextId++, type: type, data: data});
            if (this.eventQueue.length == 1) {
                this.eventSfx.play();
                setTimeout(() => this.shiftEvent(), 5000);
            }
        },

        shiftEvent() {
            this.eventQueue.shift();
            if (this.eventQueue.length > 0) {
                this.eventSfx.play();
                setTimeout(() => this.shiftEvent(), 5000);
            }
        }
    }
}

document.addEventListener('DOMContentLoaded', () => new Vue(options));