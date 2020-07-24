function deepExtend(target, ...sources) {
    for (let source of sources) {
        for (let key in source) {
            const value = source[key];
            if (value && typeof value === 'object' && !Array.isArray(value)) {
                target[key] = deepExtend(target[key] || {}, value);
            } else {
                target[key] = value;
            }
        }
    }
    return target;
}

function parseArgs(argv) {
    const parsed = {};
    let match;
    for (let arg of argv) {
        if (match = arg.match(/^--(no-)?([^=]+)(?:\=(.*))?/)) {
            parsed[match[2]] = match[3] ? match[3] : !match[1];
        } else {
            (parsed[''] = parsed[''] || []).push(arg);
        }
    }
    return parsed;
}

function expand(obj) {
    const stack = [obj];
    for (let i=0; i<stack.length; ++i) {
        for (let j in stack[i]) {
            if (j.indexOf('.') >= 0) {
                const k = j.split('.', 2);
                (stack[i][k[0]] = stack[i][k[0]] || {})[k[1]] = stack[i][j];
                delete stack[i][j];
                stack.push(stack[i][k[0]][k[1]])
            }
        }
    }
    return obj;
}

const builtOptions = {};
function buildOptions() {return deepExtend(builtOptions, ...arguments);};

module.exports = buildOptions;
module.exports.fromProcess = function (extra) {
    const args = parseArgs(process.argv.slice(2));
    for (let i in args['']) args[''][i] = JSON.parse(require('fs').readFileSync(args[''][i]));
    return buildOptions(...(args[''] || []), expand(args), extra || {});
}

