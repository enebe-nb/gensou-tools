const ffbinaries = require('ffbinaries');
const binary = (async () => {
    let result = ffbinaries.locateBinariesSync(['ffplay'], {paths: ['./']});
    if (result.ffplay.found) return result.ffplay.path;
    return new Promise((resolve, reject) => {
        ffbinaries.downloadBinaries(['ffplay'], (err, data) => {
            if (err) return reject(err);
            resolve(data[0].filename);
        })
    });
})();

module.exports = (lang) => {
    return (text) => new Promise(async (resolve) => {
        spawn(await binary, [await require('google-tts-api')(text, lang), '-nodisp', '-autoexit']).on('close', resolve);
    });
}
