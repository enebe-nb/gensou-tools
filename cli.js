process.on('unhandledRejection', e => console.error(e));
const options = require('./lib/options').fromProcess();
const twitch = new (require('twitch-pubsub-client'))();
const synth = new (require('node-speech').tts)(options.lang);

if (options['list-lang']) {console.log(synth.installed_cultures()); return;}
if (options['list-voices']) {console.log(synth.get_voices()); return;}
synth.set_voice(options.voice);

async function startTwitchService() {
    await twitch.registerUserListener(options.twitchClientId);
    const subscribeListener = twitch.onSubscription(options.twitchUserId, (message) => {
        synth.speak(message.userDisplayName + ' just subscribed!');
        if (message.message) synth.speak('They said: ' + message.message.message);
    });
    return twitch;
}

startTwitchService();
