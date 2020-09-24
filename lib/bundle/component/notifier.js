import '../style/notifier.scss'

let notifierNextId = 0;
Vue.component('notifier', {
    template: require('../template/notifier.html'),
    data: () => ({notifications: []}),
    methods: {
        send(message) {
            this.notifications.push({id: notifierNextId++, message: message});
            setTimeout(() => this.notifications.shift(), 3000);
        }
    }
});
