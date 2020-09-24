const path = require('path');

module.exports = {
    entry: {
        'challonge': './lib/bundle/challonge.js',
        'soku-events': './lib/bundle/soku-events.js',
    },
    output: {
        path: path.resolve(__dirname, 'dist'),
        filename: '[name].js'
    },
    module: {
        rules: [{
            test: /\.s[ac]ss$/i,
            use: ['style-loader', 'css-loader', 'sass-loader',],
        },{
            test: /\.html$/i,
            loader: 'html-loader',
            options: {attributes: false},
        }]
    }
};