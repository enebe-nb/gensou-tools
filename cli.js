process.on('unhandledRejection', e => console.error(e));
const options = require('./lib/options').fromProcess();
const tts = require('./lib/tts')(options.lang);
const util = require('util');

const twitchClient = require('twitch').withCredentials(options.twitchClientId, options.twitchToken);
const pubsub = new (require('twitch-pubsub-client'))();

async function startTwitchService() {
    await pubsub.registerUserListener(twitchClient);
    const subscribeListener = twitch.onSubscription(options.twitchUserId, async (message) => {
        await tts(util.format(options.textUsername, message.userDisplayName));
        if (message.message) await tts(util.format(options.textMessage, message.message.message));
    });
    return twitch;
}

startTwitchService();
